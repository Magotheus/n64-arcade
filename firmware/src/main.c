/*
 * N64 Mechanical Controller — firmware V1 (Joybus target)
 *
 * Placa: Raspberry Pi Pico W (RP2040)
 * Biblioteca: loopj/libjoybus v0.9.0 (MIT), backend rp2xxx
 *
 * - 14 switches MX ligados entre GPIO e GND (ativo em LOW, pull-up interno).
 * - Os 4 direcionais (CIMA/BAIXO/ESQUERDA/DIREITA) acionam ao mesmo tempo
 *   o STICK analógico (valores fixos) e o D-pad, para funcionar tanto em
 *   ports que leem o stick quanto nos que leem só o D-pad.
 * - Linha DATA Joybus em GP18 (pino físico 24).
 * - Pela USB (sem console) o firmware imprime o estado dos botões no
 *   monitor serial, para validar o build em bancada.
 */

#include <stdio.h>
#include <pico/stdlib.h>
#include <joybus/joybus.h>
#include <joybus/backend/rp2xxx.h>

/* ---------------------------------------------------------------- Pinagem */

#define JOYBUS_DATA_GPIO 18 /* pino físico 24 */

#define GPIO_CIMA       2
#define GPIO_BAIXO      3
#define GPIO_ESQUERDA   4
#define GPIO_DIREITA    5
#define GPIO_A          6
#define GPIO_B          7
#define GPIO_Z          8
#define GPIO_L          9
#define GPIO_R          10
#define GPIO_C_CIMA     11
#define GPIO_C_BAIXO    12
#define GPIO_C_ESQUERDA 13
#define GPIO_C_DIREITA  14
#define GPIO_START      15

/* ------------------------------------------------- Valores do stick digital
 * Controle original: faixa nominal -80..80. Nas diagonais o valor por eixo
 * é menor, imitando o "octógono" do controle original. Ajuste se um jogo
 * andar devagar ou rápido demais.
 */
#define STICK_CARDINAL 80
#define STICK_DIAGONAL 70

/* ------------------------------------------------------- Mapa de botões */

typedef struct {
    uint gpio;
    uint16_t mask;
    const char *nome;
} botao_t;

static const botao_t botoes[] = {
    {GPIO_A,          JOYBUS_N64_BUTTON_A,       "A"},
    {GPIO_B,          JOYBUS_N64_BUTTON_B,       "B"},
    {GPIO_Z,          JOYBUS_N64_BUTTON_Z,       "Z"},
    {GPIO_L,          JOYBUS_N64_BUTTON_L,       "L"},
    {GPIO_R,          JOYBUS_N64_BUTTON_R,       "R"},
    {GPIO_C_CIMA,     JOYBUS_N64_BUTTON_C_UP,    "C_CIMA"},
    {GPIO_C_BAIXO,    JOYBUS_N64_BUTTON_C_DOWN,  "C_BAIXO"},
    {GPIO_C_ESQUERDA, JOYBUS_N64_BUTTON_C_LEFT,  "C_ESQUERDA"},
    {GPIO_C_DIREITA,  JOYBUS_N64_BUTTON_C_RIGHT, "C_DIREITA"},
    {GPIO_START,      JOYBUS_N64_BUTTON_START,   "START"},
};
#define NUM_BOTOES (sizeof(botoes) / sizeof(botoes[0]))

static const uint direcionais[] = {GPIO_CIMA, GPIO_BAIXO, GPIO_ESQUERDA, GPIO_DIREITA};

/* -------------------------------------------------------------- Joybus */

static struct joybus_rp2xxx rp2xxx_bus;
static struct joybus *bus = JOYBUS(&rp2xxx_bus);
static struct joybus_n64_controller n64_controller;

/* ------------------------------------------ Acessório: "nenhum pak inserido"
 * A libjoybus v0.9.0 só responde aos comandos 0xFF/0x00/0x01; os comandos
 * de acessório 0x02 (ler) e 0x03 (escrever) ficavam sem resposta.
 * Aqui respondemos como um controle original SEM acessório: dados zerados e
 * CRC dos dados INVERTIDO (é assim que o console detecta "no pak").
 * Os contadores pak_read/pak_write saem no aviso periódico da serial.
 * OBS (19/11/2026): no teste com Pokémon Stadium os contadores ficaram 0/0,
 * ou seja, o jogo NÃO mandou 0x02/0x03 (o status 0x00 já diz "sem pak").
 * Mantido como robustez; não foi a causa comprovada da falha anterior.
 */
#define CMD_PAK_READ        0x02 /* 1 cmd + 2 endereço        -> 32 dados + 1 CRC */
#define CMD_PAK_WRITE       0x03 /* 1 cmd + 2 endereço + 32   -> 1 CRC */
#define PAK_READ_TX         3
#define PAK_WRITE_TX        35
#define PAK_BLOCO           32

static const struct joybus_target_api *api_original;
static uint8_t resposta_pak[PAK_BLOCO + 1];
static volatile uint32_t cont_pak_read, cont_pak_write;

/* CRC de dados do pak (polinômio 0x85), mesmo algoritmo do libdragon. */
static uint8_t pak_crc(const uint8_t *dados) {
    uint8_t crc = 0;
    for (int i = 0; i <= PAK_BLOCO; i++) {
        for (int bit = 7; bit >= 0; bit--) {
            uint8_t xor_tap = (crc & 0x80) ? 0x85 : 0x00;
            crc <<= 1;
            if (i < PAK_BLOCO && (dados[i] & (1u << bit))) crc |= 1;
            crc ^= xor_tap;
        }
    }
    return crc;
}

/* ------------------------------------------------ Diagnóstico da linha DATA
 * Mostra na serial, a cada 2 s, uma linha "[DIAG] ...":
 *   - nível da linha DATA (com o console ligado e conectado ela fica ALTA,
 *     porque o pull-up é do console; BAIXA/flutuando = fio solto ou trocado);
 *   - pulsos/s: bordas de descida no pino (console consulta ~60x/s);
 *   - comandos que chegaram ao firmware: info (0x00), leitura (0x01),
 *     reset (0xFF) e outros.
 * Pulsos > 0 mas comandos = 0 -> sinal chega mas não é entendido.
 * Pulsos = 0 com console ligado -> o sinal não chega ao pino (fiação). */
/* "pulsos" agora = bytes recebidos do console (antes era uma interrupção de
 * borda no próprio pino DATA, removida em 19/11 tarde por suspeita de atrasar
 * a resposta Joybus). O painel só precisa que seja > 0 quando há conversa. */
static volatile uint32_t cont_pulsos;
static volatile uint32_t cont_cmd_info, cont_cmd_ler, cont_cmd_reset, cont_cmd_outros;

/* ------------------------------------------------------------- Telemetria
 * Cada comando concluído vira um evento [EVT] com hora (µs), intervalo desde
 * o comando anterior e a resposta enviada. Para não inundar a USB:
 *   - registra TUDO por 3 s quando o console volta a falar depois de >300 ms
 *     de silêncio (= console ligou/reiniciou: é aí que o jogo detecta o controle);
 *   - fora disso, registra só o anormal (comando incompleto, não suportado).
 * Produtor: interrupção do Joybus. Consumidor: laço principal (imprime). */
#define EVT_INICIO      0x01  /* primeiro comando após silêncio */
#define EVT_INCOMPLETO  0x02  /* o comando ANTERIOR não chegou inteiro */
#define EVT_NAO_SUPORT  0x04  /* comando que ninguém respondeu */
#define SILENCIO_US     300000u
#define JANELA_US       3000000u
#define EVT_N           512u

typedef struct {
    uint32_t t, dt;      /* hora do 1º byte e intervalo desde o comando anterior */
    uint8_t cmd, nbytes, flags, resp_len;
    uint8_t resp[4];
} evento_t;

static evento_t eventos[EVT_N];
static volatile uint32_t evt_head, evt_tail, evt_perdidos, cont_incompletos, cont_inicios;
static volatile bool cmd_pendente;
static uint32_t ultimo_cmd_us, cmd_t0, cmd_dt, janela_ate;
static uint8_t flags_atuais;
static volatile uint32_t cont_ressinc, cont_realinha_off;

/* Função da libjoybus v0.9.0 (backend rp2xxx, não é static): reinicia a
 * máquina PIO no modo "aguardando comando" com o buffer zerado. */
extern int64_t target_rx_timeout(alarm_id_t id, void *user_data);

/* Console desligado deixa a linha DATA em 0 V; o PIO guarda esse "bit" e, ao
 * religar, lê tudo deslocado de 1 bit. Chamado do laço principal quando a
 * linha voltou a ficar ALTA e parada: zera o receptor com segurança. */
static void realinhar_receptor(void) {
    irq_set_enabled(PIO0_IRQ_0, false);
    target_rx_timeout(0, bus);
    cmd_pendente = false;
    irq_set_enabled(PIO0_IRQ_0, true);
    cont_realinha_off++;
}

static void registrar(uint8_t cmd, uint8_t nbytes, uint8_t flags, const uint8_t *resp, uint8_t len) {
    bool janela = (int32_t)(janela_ate - cmd_t0) > 0;
    if (!janela && !(flags & (EVT_INCOMPLETO | EVT_NAO_SUPORT))) return;
    uint32_t h = evt_head;
    if (h - evt_tail >= EVT_N) { evt_perdidos++; return; }
    evento_t *e = &eventos[h % EVT_N];
    e->t = cmd_t0; e->dt = cmd_dt; e->cmd = cmd; e->nbytes = nbytes; e->flags = flags;
    e->resp_len = len > 4 ? 4 : len;
    for (uint8_t i = 0; i < e->resp_len; i++) e->resp[i] = resp[i];
    evt_head = h + 1;
}

/* byte_idx = quantos bytes do comando já chegaram (1 = só o código). */
static int controle_byte_received(struct joybus_target *target, const uint8_t *cmd,
                                  uint8_t byte_idx, joybus_target_response_cb_t enviar,
                                  void *user_data) {
    cont_pulsos++;
    if (byte_idx == 1) {
        uint32_t agora = time_us_32();
        flags_atuais = 0;
        if (cmd_pendente) { cont_incompletos++; flags_atuais |= EVT_INCOMPLETO; }
        if (agora - ultimo_cmd_us > SILENCIO_US) {
            flags_atuais |= EVT_INICIO; cont_inicios++;
            janela_ate = agora + JANELA_US;
        }
        cmd_dt = agora - ultimo_cmd_us;
        ultimo_cmd_us = agora;
        cmd_t0 = agora;
        switch (cmd[0]) {
        case 0x00: cont_cmd_info++; break;
        case 0x01: cont_cmd_ler++; break;
        case 0xFF: cont_cmd_reset++; break;
        default:   cont_cmd_outros++; break;
        }
    }
    int rc;
    switch (cmd[0]) {
    case CMD_PAK_READ:
    case CMD_PAK_WRITE:
        rc = 0; break; /* tratados abaixo */
    default:
        rc = api_original->byte_received(target, cmd, byte_idx, enviar, user_data);
        if (rc < 0 && byte_idx < 60) {
            /* Comando não reconhecido (normalmente um byte lido DESALINHADO,
             * ex. 0x01 lido como 0x80). A libjoybus reiniciaria o receptor no
             * meio do comando, deixando bits sobrando e realimentando o erro.
             * Pedimos "mais um byte": o resto do comando é engolido e, 100 µs
             * depois (linha parada), o timeout da lib reinicia o receptor alinhado. */
            if (byte_idx == 1) {
                cont_ressinc++;
                registrar(cmd[0], byte_idx, flags_atuais | EVT_NAO_SUPORT, NULL, 0);
            }
            cmd_pendente = false;
            return 1;
        }
        cmd_pendente = rc > 0;
        if (rc == 0) {
            const struct joybus_n64_controller *c = JOYBUS_N64_CONTROLLER(target);
            if (cmd[0] == 0x01) registrar(cmd[0], byte_idx, flags_atuais, (const uint8_t *)&c->input, 4);
            else                registrar(cmd[0], byte_idx, flags_atuais, c->id, 3);
        } else if (rc < 0) {
            cmd_pendente = false;
            registrar(cmd[0], byte_idx, flags_atuais | EVT_NAO_SUPORT, NULL, 0);
        }
        return rc;
    }
    (void)rc;
    switch (cmd[0]) {
    case CMD_PAK_READ:
        if (byte_idx < PAK_READ_TX) { cmd_pendente = true; return PAK_READ_TX - byte_idx; }
        cmd_pendente = false;
        for (int i = 0; i < PAK_BLOCO; i++) resposta_pak[i] = 0x00;
        resposta_pak[PAK_BLOCO] = (uint8_t)~pak_crc(resposta_pak);
        enviar(resposta_pak, PAK_BLOCO + 1, user_data);
        cont_pak_read++;
        registrar(cmd[0], byte_idx, flags_atuais, &resposta_pak[PAK_BLOCO], 1);
        return 0;
    case CMD_PAK_WRITE:
        if (byte_idx < PAK_WRITE_TX) { cmd_pendente = true; return PAK_WRITE_TX - byte_idx; }
        cmd_pendente = false;
        resposta_pak[0] = (uint8_t)~pak_crc(&cmd[3]);
        enviar(resposta_pak, 1, user_data);
        cont_pak_write++;
        registrar(cmd[0], byte_idx, flags_atuais, resposta_pak, 1);
        return 0;
    default:
        return api_original->byte_received(target, cmd, byte_idx, enviar, user_data);
    }
}

static const struct joybus_target_api controle_api = {
    .byte_received = controle_byte_received,
};

static inline bool pressionado(uint gpio) {
    return gpio_get(gpio) == 0; /* pull-up: LOW = pressionado */
}

static void init_entrada(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

/* Converte os 4 direcionais em X/Y do stick E em bits do D-pad, ao mesmo
 * tempo: alguns ports leem só o D-pad (2D/fliperama), outros só o stick.
 * Opostos simultâneos se anulam (neutro) nos dois. */
static uint16_t ler_direcionais(int8_t *x, int8_t *y) {
    int dx = (pressionado(GPIO_DIREITA) ? 1 : 0) - (pressionado(GPIO_ESQUERDA) ? 1 : 0);
    int dy = (pressionado(GPIO_CIMA) ? 1 : 0) - (pressionado(GPIO_BAIXO) ? 1 : 0);
    int amp = (dx != 0 && dy != 0) ? STICK_DIAGONAL : STICK_CARDINAL;
    *x = (int8_t)(dx * amp);
    *y = (int8_t)(dy * amp); /* N64: Y positivo = para cima */

    uint16_t dpad = 0;
    if (dx > 0) dpad |= JOYBUS_N64_BUTTON_RIGHT;
    if (dx < 0) dpad |= JOYBUS_N64_BUTTON_LEFT;
    if (dy > 0) dpad |= JOYBUS_N64_BUTTON_UP;
    if (dy < 0) dpad |= JOYBUS_N64_BUTTON_DOWN;
    return dpad;
}

int main(void) {
    stdio_init_all();

    for (size_t i = 0; i < NUM_BOTOES; i++) init_entrada(botoes[i].gpio);
    for (size_t i = 0; i < 4; i++) init_entrada(direcionais[i]);

    /* API da libjoybus v0.9.0 (segue o exemplo gcn-controller-target da tag) */
    int err = joybus_rp2xxx_init(&rp2xxx_bus, JOYBUS_DATA_GPIO, pio0);
    joybus_enable(bus);

    /* ID 0x0500 = controle N64 padrão (NUS-005) */
    joybus_n64_controller_init(&n64_controller, (uint8_t)(JOYBUS_ID_N64_CONTROLLER >> 8));
    /* Intercepta 0x02/0x03 (pak) e repassa o resto para a libjoybus. */
    api_original = n64_controller.base.api;
    n64_controller.base.api = &controle_api;
    joybus_target_register(bus, JOYBUS_TARGET(&n64_controller));


    uint16_t ultimo_botoes = 0xFFFF;
    int8_t ultimo_x = 127, ultimo_y = 127;
    absolute_time_t proximo_aviso = get_absolute_time();
    absolute_time_t proximo_diag = make_timeout_time_ms(2000);
    uint32_t pulsos_ant = 0;

    while (true) {
        uint16_t b = 0;
        for (size_t i = 0; i < NUM_BOTOES; i++) {
            if (pressionado(botoes[i].gpio)) b |= botoes[i].mask;
        }
        int8_t x, y;
        uint16_t dpad = ler_direcionais(&x, &y);
        b |= dpad;

        /* Atualiza o estado enviado ao console (preserva bit de reset). */
        n64_controller.input.buttons =
            (n64_controller.input.buttons & ~JOYBUS_N64_BUTTON_MASK) | b;
        /* v0.9.0 declara os eixos como uint8_t; o byte é complemento de dois */
        n64_controller.input.stick_x = (uint8_t)x;
        n64_controller.input.stick_y = (uint8_t)y;

        /* Diagnóstico pela USB serial: imprime só quando algo muda. */
        if (b != ultimo_botoes || x != ultimo_x || y != ultimo_y) {
            printf("X=%4d Y=%4d DPAD=%s%s%s%s |", x, y,
                   (b & JOYBUS_N64_BUTTON_UP) ? "^" : "",
                   (b & JOYBUS_N64_BUTTON_DOWN) ? "v" : "",
                   (b & JOYBUS_N64_BUTTON_LEFT) ? "<" : "",
                   (b & JOYBUS_N64_BUTTON_RIGHT) ? ">" : "");
            for (size_t i = 0; i < NUM_BOTOES; i++) {
                if (b & botoes[i].mask) printf(" %s", botoes[i].nome);
            }
            printf("\n");
            ultimo_botoes = b;
            ultimo_x = x;
            ultimo_y = y;
        }

        /* Lembrete periódico de que o firmware está vivo. */
        if (time_reached(proximo_aviso)) {
            printf("[N64 Mechanical V1] joybus_init=%d, DATA=GP%d, pak_read=%lu, pak_write=%lu\n",
                   err, JOYBUS_DATA_GPIO, (unsigned long)cont_pak_read, (unsigned long)cont_pak_write);
            proximo_aviso = make_timeout_time_ms(5000);
        }

        /* Vigia da linha DATA: baixa por >20 ms = console desligado. Quando
         * volta ALTA e fica parada 2 ms, realinha o receptor. */
        {
            static uint32_t ms_baixa, ms_alta;
            static bool console_off;
            if (!gpio_get(JOYBUS_DATA_GPIO)) {
                ms_alta = 0;
                if (++ms_baixa >= 20 && !console_off) {
                    console_off = true;
                    printf("[EVT] linha DATA baixa ha 20 ms: console desligado\n");
                }
            } else {
                ms_baixa = 0;
                if (console_off && ++ms_alta >= 2) {
                    console_off = false;
                    realinhar_receptor();
                    printf("[EVT] linha DATA voltou: console ligado, receptor realinhado\n");
                }
            }
        }

        /* Telemetria: descarrega os eventos registrados pela interrupção. */
        for (int n = 0; n < 16 && evt_tail != evt_head; n++) {
            evento_t e = eventos[evt_tail % EVT_N];
            evt_tail = evt_tail + 1;
            if (e.flags & EVT_INICIO)
                printf("[EVT] ===== console comecou a falar (silencio de %lu ms) =====\n",
                       (unsigned long)(e.dt / 1000));
            printf("[EVT] t=%lu.%06lu dt=%luus cmd=%02X n=%u resp=", (unsigned long)(e.t / 1000000),
                   (unsigned long)(e.t % 1000000), (unsigned long)e.dt, e.cmd, e.nbytes);
            for (uint8_t i = 0; i < e.resp_len; i++) printf("%02X", e.resp[i]);
            if (!e.resp_len) printf("-");
            if (e.flags & EVT_INCOMPLETO) printf(" ANTERIOR_INCOMPLETO");
            if (e.flags & EVT_NAO_SUPORT) printf(" NAO_SUPORTADO");
            printf("\n");
        }

        /* Diagnóstico da linha DATA a cada 2 s. */
        if (time_reached(proximo_diag)) {
            uint32_t p = cont_pulsos;
            printf("[DIAG] DATA=%s pulsos/s=%lu cmd_info=%lu cmd_ler=%lu cmd_reset=%lu cmd_outros=%lu\n",
                   gpio_get(JOYBUS_DATA_GPIO) ? "ALTA" : "BAIXA",
                   (unsigned long)((p - pulsos_ant) / 2),
                   (unsigned long)cont_cmd_info, (unsigned long)cont_cmd_ler,
                   (unsigned long)cont_cmd_reset, (unsigned long)cont_cmd_outros);
            printf("[TEL] inicios=%lu incompletos=%lu eventos_perdidos=%lu ressinc=%lu realinha_off=%lu\n",
                   (unsigned long)cont_inicios, (unsigned long)cont_incompletos,
                   (unsigned long)evt_perdidos, (unsigned long)cont_ressinc,
                   (unsigned long)cont_realinha_off);
            pulsos_ant = p;
            proximo_diag = make_timeout_time_ms(2000);
        }

        sleep_ms(1);
    }
}
