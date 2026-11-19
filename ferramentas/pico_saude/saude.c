/*
 * Pico — diagnóstico de saúde (RP2040)
 *
 * Testes: chip/ROM, ID único, cristal e clocks, flash (JEDEC + escrita/leitura
 * nos últimos 64 KB), RAM (160 KB), os dois núcleos, sensor de temperatura,
 * tensão VSYS e todos os GPIOs livres (pull-up/pull-down).
 *
 * IMPORTANTE: o teste de GPIO só é válido com NADA ligado nos pinos.
 * Abra o monitor serial; os testes rodam quando a porta é aberta.
 * Digite 'r' para repetir.
 */
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "pico/multicore.h"
#include "pico/flash.h"
#include "hardware/flash.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"

static int falhas = 0;

#define OK(fmt, ...)   printf("  [ OK ] " fmt "\n", ##__VA_ARGS__)
#define FAIL(fmt, ...) do { printf("  [FALHA] " fmt "\n", ##__VA_ARGS__); falhas++; } while (0)
#define INFO(fmt, ...) printf("  [info] " fmt "\n", ##__VA_ARGS__)

/* ------------------------------------------------------------ chip */
static void teste_chip(void) {
    printf("\n1) Chip\n");
    char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
    pico_get_unique_board_id_string(id, sizeof(id));
    INFO("RP2040 revisao B%u, ROM v%u", rp2040_chip_version(), rp2040_rom_version());
    INFO("ID unico da placa: %s", id);
    OK("chip responde");
}

/* ------------------------------------------------------------ clocks */
static void teste_clocks(void) {
    printf("\n2) Cristal e clocks\n");
    uint32_t xosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_XOSC_CLKSRC);
    uint32_t sys  = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint32_t usb  = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    INFO("cristal %lu kHz | sistema %lu kHz | USB %lu kHz", xosc, sys, usb);
    if (xosc > 11900 && xosc < 12100) OK("cristal de 12 MHz dentro da tolerancia");
    else FAIL("cristal fora do esperado (12000 kHz)");
    if (usb > 47900 && usb < 48100) OK("clock USB 48 MHz");
    else FAIL("clock USB fora do esperado");
}

/* ------------------------------------------------------------ flash */
#define FLASH_TEST_SIZE (64 * 1024)
static uint8_t flash_buf[FLASH_SECTOR_SIZE];
static uint32_t flash_off;
static uint8_t jedec[4];

/* O núcleo 1 está parado durante o teste de flash, então basta desligar as
 * interrupções em volta de cada operação (as funções de flash do SDK rodam
 * da RAM). */
static void cb_jedec(void) {
    uint8_t tx[4] = {0x9F, 0, 0, 0};
    uint32_t s = save_and_disable_interrupts();
    flash_do_cmd(tx, jedec, 4);
    restore_interrupts(s);
}
static void cb_erase_sector(uint32_t off) {
    uint32_t s = save_and_disable_interrupts();
    flash_range_erase(off, FLASH_SECTOR_SIZE);
    restore_interrupts(s);
}

/* Registro de etapa em scratch do watchdog (sobrevive a reset do watchdog). */
static void etapa(uint32_t n) { watchdog_hw->scratch[0] = 0x5A5A0000u | n; }
static void cb_prog(uint32_t off) {
    uint32_t s = save_and_disable_interrupts();
    flash_range_program(off, flash_buf, FLASH_SECTOR_SIZE);
    restore_interrupts(s);
}

static uint8_t padrao(uint32_t i) { return (uint8_t)((i * 2654435761u) >> 24); }

static void teste_flash(void) {
    printf("\n3) Memoria flash\n");
    cb_jedec();
    uint32_t tamanho = (jedec[3] >= 16 && jedec[3] <= 26) ? (1u << jedec[3]) : 0;
    INFO("JEDEC: fabricante 0x%02X, tipo 0x%02X, capacidade 0x%02X", jedec[1], jedec[2], jedec[3]);
    if (tamanho == 0 || jedec[1] == 0x00 || jedec[1] == 0xFF) {
        FAIL("chip de flash nao identificado");
        return;
    }
    INFO("tamanho detectado: %lu KB", tamanho / 1024);
    if (tamanho < PICO_FLASH_SIZE_BYTES) INFO("(menor que os %u KB do Pico padrao)", PICO_FLASH_SIZE_BYTES / 1024);

    /* Área segura: 1 MB a partir do início (programa ocupa < 100 KB). */
    flash_off = 1024 * 1024;
    const uint8_t *xip = (const uint8_t *)(XIP_BASE + flash_off);
    uint32_t erros = 0, nao_apagou = 0;

    for (uint32_t s = 0; s < FLASH_TEST_SIZE; s += FLASH_SECTOR_SIZE) {
        etapa(10 + s / FLASH_SECTOR_SIZE);         /* registra onde está */
        stdio_flush(); sleep_ms(5);
        cb_erase_sector(flash_off + s);
        for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i++) if (xip[s + i] != 0xFF) nao_apagou++;
        for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i++) flash_buf[i] = padrao(s + i);
        stdio_flush(); sleep_ms(5);
        cb_prog(flash_off + s);
        for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i++) if (xip[s + i] != padrao(s + i)) erros++;
        cb_erase_sector(flash_off + s);            /* deixa limpo */
        watchdog_update();
    }
    etapa(0);
    if (nao_apagou) FAIL("%lu bytes nao apagaram", nao_apagou);
    if (erros) FAIL("%lu bytes errados na leitura da flash", erros);
    if (!nao_apagou && !erros) OK("apagar / gravar / ler 64 KB (16 setores) sem erros");
}

/* ------------------------------------------------------------ RAM */
#define RAM_TEST_WORDS (160 * 1024 / 4)
static uint32_t ram_buf[RAM_TEST_WORDS];

static void teste_ram(void) {
    printf("\n4) Memoria RAM\n");
    const uint32_t pads[] = {0x00000000, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA};
    uint32_t erros = 0;
    for (int p = 0; p < 4; p++) {
        for (uint32_t i = 0; i < RAM_TEST_WORDS; i++) ram_buf[i] = pads[p];
        for (uint32_t i = 0; i < RAM_TEST_WORDS; i++) if (ram_buf[i] != pads[p]) erros++;
    }
    for (uint32_t i = 0; i < RAM_TEST_WORDS; i++) ram_buf[i] = (uint32_t)(uintptr_t)&ram_buf[i] ^ 0xA5A5A5A5;
    for (uint32_t i = 0; i < RAM_TEST_WORDS; i++) if (ram_buf[i] != ((uint32_t)(uintptr_t)&ram_buf[i] ^ 0xA5A5A5A5)) erros++;
    if (erros) FAIL("%lu palavras erradas em 160 KB de RAM", erros);
    else OK("160 KB testados com 5 padroes, sem erros");
}

/* ------------------------------------------------------------ núcleos */
static void core1_main(void) {
    uint32_t n = multicore_fifo_pop_blocking();
    uint32_t soma = 0;
    for (uint32_t i = 1; i <= n; i++) soma += i;
    multicore_fifo_push_blocking(soma);
}

static void teste_nucleos(void) {
    printf("\n5) Nucleos\n");
    multicore_reset_core1();
    multicore_launch_core1(core1_main);
    multicore_fifo_push_blocking(1000);
    uint32_t r;
    if (multicore_fifo_pop_timeout_us(500000, &r) && r == 500500) OK("nucleo 0 e nucleo 1 funcionando");
    else FAIL("nucleo 1 nao respondeu corretamente");
    multicore_reset_core1();
}

/* ------------------------------------------------------------ analógico */
static void teste_analogico(void) {
    printf("\n6) Temperatura e alimentacao\n");
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
    float v = adc_read() * 3.3f / 4096.0f;
    float t = 27.0f - (v - 0.706f) / 0.001721f;
    INFO("temperatura do chip: %.1f C", t);
    if (t > 5.0f && t < 70.0f) OK("sensor de temperatura plausivel");
    else FAIL("temperatura fora do normal (sensor ou ADC com problema)");

    /* VSYS/3 no GP29: só existe no Pico oficial. No Pico W o GP25 precisa
     * estar em nível alto; em clones o GP29 costuma ser um pino livre. */
    gpio_init(24); gpio_set_dir(24, GPIO_IN); gpio_disable_pulls(24);
    bool vbus = gpio_get(24);
    gpio_init(25); gpio_set_dir(25, GPIO_OUT); gpio_put(25, 1);
    adc_gpio_init(29);
    adc_select_input(3);
    sleep_ms(2);
    float vsys = adc_read() * 3.3f / 4096.0f * 3.0f;
    gpio_put(25, 0);
    INFO("VBUS (GP24): %s | VSYS medido: %.2f V", vbus ? "presente" : "nao detectado", vsys);
    if (vsys > 4.3f && vsys < 5.5f) OK("alimentacao normal (divisor VSYS do Pico oficial)");
    else if (vsys < 1.0f) INFO("sem divisor de VSYS no GP29 -> provavel CLONE do Pico (nao e defeito)");
    else FAIL("VSYS fora do esperado (cabo/porta fraca ou regulador)");
}

/* ------------------------------------------------------------ GPIO */
static void teste_gpio(void) {
    printf("\n7) Pinos GPIO (com NADA ligado neles)\n");
    const uint pinos[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,26,27,28};
    char ruins[160] = "";
    int nruins = 0;
    for (size_t i = 0; i < sizeof(pinos) / sizeof(pinos[0]); i++) {
        uint g = pinos[i];
        gpio_init(g);
        gpio_set_dir(g, GPIO_IN);
        gpio_pull_up(g);   sleep_us(200); bool alto  = gpio_get(g);
        gpio_pull_down(g); sleep_us(200); bool baixo = gpio_get(g);
        gpio_disable_pulls(g);
        if (!alto || baixo) {
            char tmp[24];
            snprintf(tmp, sizeof(tmp), " GP%u(%s)", g, !alto ? "preso em 0" : "preso em 1");
            strncat(ruins, tmp, sizeof(ruins) - strlen(ruins) - 1);
            nruins++;
        }
    }
    if (nruins) FAIL("%d pino(s) nao obedecem:%s", nruins, ruins);
    else OK("26 GPIOs sobem e descem normalmente");
}

/* ------------------------------------------------------------ LED */
static void pisca_led(void) {
#ifdef PICO_DEFAULT_LED_PIN
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    for (int i = 0; i < 6; i++) { gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN); sleep_ms(150); }
#endif
}

static void rodar_tudo(void) {
    falhas = 0;
    printf("\n==============================================\n");
    printf(" DIAGNOSTICO DE SAUDE - Raspberry Pi Pico\n");
    printf("==============================================\n");
    teste_chip();
    teste_clocks();
    teste_flash();
    teste_ram();
    teste_nucleos();
    teste_analogico();
    teste_gpio();
    printf("\n----------------------------------------------\n");
    if (falhas == 0) printf(" RESULTADO: SAUDAVEL - nenhum problema encontrado\n");
    else printf(" RESULTADO: %d PROBLEMA(S) ENCONTRADO(S)\n", falhas);
    printf("----------------------------------------------\n");
    printf(" 'r' = repetir   'b' = voltar ao modo BOOTSEL\n");
    printf("FIM_DO_DIAGNOSTICO\n");
    pisca_led();
}

int main(void) {
    stdio_init_all();
    uint32_t marca = watchdog_hw->scratch[0];
    bool travou = watchdog_caused_reboot() && (marca & 0xFFFF0000u) == 0x5A5A0000u
                  && (marca & 0xFFFF) != 0;
    while (true) {
        while (!stdio_usb_connected()) sleep_ms(50);
        sleep_ms(500);
        if (travou) {
            uint32_t n = marca & 0xFFFF;
            printf("\n!!! A placa TRAVOU na execucao anterior e foi reiniciada pelo watchdog.\n");
            if (n >= 10) printf("!!! Travou no teste de FLASH, setor %lu de 16.\n", n - 10 + 1);
            printf("!!! Isso indica problema no chip de flash ou na comunicacao com ele.\n");
            printf("FIM_DO_DIAGNOSTICO\n");
            travou = false;
            etapa(0);
            /* aguarda 'r' para tentar de novo */
            while (stdio_usb_connected()) {
                int c = getchar_timeout_us(100000);
                if (c == 'r' || c == 'R') break;
                if (c == 'b' || c == 'B') reset_usb_boot(0, 0);
            }
        }
        watchdog_enable(8000, true);   /* reinicia se travar > 8 s */
        rodar_tudo();
        watchdog_disable();
        while (stdio_usb_connected()) {
            int c = getchar_timeout_us(100000);
            if (c == 'r' || c == 'R') { watchdog_enable(8000, true); rodar_tudo(); watchdog_disable(); }
            if (c == 'b' || c == 'B') { printf("Reiniciando em BOOTSEL...\n"); sleep_ms(200); reset_usb_boot(0, 0); }
        }
    }
}
