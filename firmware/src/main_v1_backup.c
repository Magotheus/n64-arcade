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
    joybus_target_register(bus, JOYBUS_TARGET(&n64_controller));

    uint16_t ultimo_botoes = 0xFFFF;
    int8_t ultimo_x = 127, ultimo_y = 127;
    absolute_time_t proximo_aviso = get_absolute_time();

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
            printf("[N64 Mechanical V1] joybus_init=%d, DATA=GP%d\n", err, JOYBUS_DATA_GPIO);
            proximo_aviso = make_timeout_time_ms(5000);
        }

        sleep_ms(1);
    }
}
