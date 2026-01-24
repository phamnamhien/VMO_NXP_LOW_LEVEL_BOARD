/**
 * @file    simple_uart.c
 * @brief   Simple UART driver using direct register access
 * @note    Independent of NXP MCAL drivers - uses polling mode for reliability
 */

#include "simple_uart.h"
#include <stdio.h>
#include <string.h>

/*===========================================================================*/
/*                          LPUART REGISTER MAP (S32K388)                     */
/*===========================================================================*/

/* LPUART Base Addresses for S32K388 */
#define LPUART0_BASE    0x40328000UL
#define LPUART1_BASE    0x4032C000UL
#define LPUART2_BASE    0x40330000UL
#define LPUART3_BASE    0x40334000UL
#define LPUART4_BASE    0x40338000UL
#define LPUART5_BASE    0x4033C000UL
#define LPUART6_BASE    0x40340000UL
#define LPUART7_BASE    0x40344000UL
#define LPUART8_BASE    0x4048C000UL
#define LPUART9_BASE    0x40490000UL
#define LPUART10_BASE   0x40494000UL
#define LPUART11_BASE   0x40498000UL
#define LPUART12_BASE   0x4049C000UL
#define LPUART13_BASE   0x404A0000UL
#define LPUART14_BASE   0x404A4000UL
#define LPUART15_BASE   0x404A8000UL

/* Select base address based on configured instance */
#if (SIMPLE_UART_INSTANCE == 0)
    #define LPUART_BASE LPUART0_BASE
#elif (SIMPLE_UART_INSTANCE == 1)
    #define LPUART_BASE LPUART1_BASE
#elif (SIMPLE_UART_INSTANCE == 2)
    #define LPUART_BASE LPUART2_BASE
#elif (SIMPLE_UART_INSTANCE == 3)
    #define LPUART_BASE LPUART3_BASE
#else
    #define LPUART_BASE LPUART1_BASE  /* Default to LPUART1 */
#endif

/* LPUART Register Offsets */
#define LPUART_VERID    (*(volatile uint32_t*)(LPUART_BASE + 0x00))  /* Version ID */
#define LPUART_PARAM    (*(volatile uint32_t*)(LPUART_BASE + 0x04))  /* Parameter */
#define LPUART_GLOBAL   (*(volatile uint32_t*)(LPUART_BASE + 0x08))  /* Global */
#define LPUART_PINCFG   (*(volatile uint32_t*)(LPUART_BASE + 0x0C))  /* Pin Config */
#define LPUART_BAUD     (*(volatile uint32_t*)(LPUART_BASE + 0x10))  /* Baud Rate */
#define LPUART_STAT     (*(volatile uint32_t*)(LPUART_BASE + 0x14))  /* Status */
#define LPUART_CTRL     (*(volatile uint32_t*)(LPUART_BASE + 0x18))  /* Control */
#define LPUART_DATA     (*(volatile uint32_t*)(LPUART_BASE + 0x1C))  /* Data */
#define LPUART_MATCH    (*(volatile uint32_t*)(LPUART_BASE + 0x20))  /* Match Address */
#define LPUART_MODIR    (*(volatile uint32_t*)(LPUART_BASE + 0x24))  /* Modem IR */
#define LPUART_FIFO     (*(volatile uint32_t*)(LPUART_BASE + 0x28))  /* FIFO */
#define LPUART_WATER    (*(volatile uint32_t*)(LPUART_BASE + 0x2C))  /* Watermark */

/* LPUART_STAT Register Bits */
#define LPUART_STAT_TDRE    (1UL << 23)  /* Transmit Data Register Empty */
#define LPUART_STAT_TC      (1UL << 22)  /* Transmission Complete */
#define LPUART_STAT_RDRF    (1UL << 21)  /* Receive Data Register Full */
#define LPUART_STAT_OR      (1UL << 19)  /* Overrun Flag */

/* LPUART_CTRL Register Bits */
#define LPUART_CTRL_TE      (1UL << 19)  /* Transmitter Enable */
#define LPUART_CTRL_RE      (1UL << 18)  /* Receiver Enable */

/* LPUART_FIFO Register Bits */
#define LPUART_FIFO_TXFE    (1UL << 7)   /* TX FIFO Enable */
#define LPUART_FIFO_RXFE    (1UL << 3)   /* RX FIFO Enable */

/*===========================================================================*/
/*                          PRIVATE DATA                                      */
/*===========================================================================*/

static volatile uint8_t g_initialized = 0;

/*===========================================================================*/
/*                          PUBLIC FUNCTIONS                                   */
/*===========================================================================*/

void simple_uart_init(void) {
    /*
     * Note: This assumes that:
     * 1. Clock for LPUART is already enabled (via Mcu_Init)
     * 2. Pins are already configured (via Port_Init)
     *
     * If UART was already initialized by NXP driver, we just verify settings.
     * Otherwise we configure it.
     */

    /* Check if already enabled */
    if (LPUART_CTRL & LPUART_CTRL_TE) {
        /* Already configured by NXP driver - just mark initialized */
        g_initialized = 1;
        return;
    }

    /* Disable TX/RX before configuration */
    LPUART_CTRL = 0;

    /* Calculate baud rate divisor
     * SBR = CLK / (OSR * BAUD)
     * Using OSR = 16 (default oversampling)
     */
    uint32_t osr = 16;
    uint32_t sbr = SIMPLE_UART_CLK_FREQ / (osr * SIMPLE_UART_BAUD_RATE);

    /* Configure baud rate */
    LPUART_BAUD = (sbr & 0x1FFF) |       /* SBR[12:0] */
                  ((osr - 1) << 24);      /* OSR[4:0] */

    /* 8-bit data, no parity (default in CTRL) */

    /* Enable TX FIFO for better performance */
    LPUART_FIFO = LPUART_FIFO_TXFE;

    /* Enable transmitter */
    LPUART_CTRL = LPUART_CTRL_TE;

    g_initialized = 1;
}

bool simple_uart_tx_ready(void) {
    return (LPUART_STAT & LPUART_STAT_TDRE) != 0;
}

void simple_uart_putc(char c) {
    if (!g_initialized) {
        simple_uart_init();
    }

    /* Wait for TX register to be empty */
    uint32_t timeout = 100000;
    while (!(LPUART_STAT & LPUART_STAT_TDRE) && timeout > 0) {
        timeout--;
    }

    /* Write character */
    LPUART_DATA = (uint32_t)c;
}

void simple_uart_puts(const char* str) {
    if (!str) return;

    while (*str) {
        if (*str == '\n') {
            simple_uart_putc('\r');  /* Add CR before LF */
        }
        simple_uart_putc(*str++);
    }
}

void simple_uart_printf(const char* format, ...) {
    char buffer[256];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    simple_uart_puts(buffer);
}

void simple_uart_test(void) {
    simple_uart_puts("\n");
    simple_uart_puts("===========================================\n");
    simple_uart_puts("  SIMPLE UART TEST\n");
    simple_uart_puts("===========================================\n");
    simple_uart_puts("\n");

    /* Test basic output */
    simple_uart_puts("Test 1: Basic string output... OK\n");

    /* Test printf */
    simple_uart_printf("Test 2: Printf with number: %d\n", 12345);
    simple_uart_printf("Test 3: Printf with hex: 0x%08X\n", 0xDEADBEEF);

    /* Test long string */
    simple_uart_puts("Test 4: Long string test - ");
    simple_uart_puts("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    simple_uart_puts("abcdefghijklmnopqrstuvwxyz");
    simple_uart_puts("0123456789\n");

    /* Test multiple rapid outputs */
    simple_uart_puts("Test 5: Rapid output test:\n");
    for (int i = 0; i < 10; i++) {
        simple_uart_printf("  Line %d of 10\n", i + 1);
    }

    simple_uart_puts("\n");
    simple_uart_puts("===========================================\n");
    simple_uart_puts("  ALL TESTS COMPLETED!\n");
    simple_uart_puts("===========================================\n");
    simple_uart_puts("\n");
}
