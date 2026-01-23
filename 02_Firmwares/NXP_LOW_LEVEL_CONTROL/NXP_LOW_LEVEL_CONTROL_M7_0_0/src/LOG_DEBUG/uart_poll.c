/**
 * @file    uart_poll.c
 * @brief   Simple polling-based UART output for debug (no interrupts)
 *
 * This bypasses all NXP UART drivers and writes directly to LPUART4 registers.
 * Used for debugging when interrupt-based UART doesn't work.
 */

#include "uart_poll.h"
#include <stdarg.h>
#include <stdio.h>

/* LPUART Register offsets */
#define LPUART_VERID_OFFSET     0x00U
#define LPUART_PARAM_OFFSET     0x04U
#define LPUART_GLOBAL_OFFSET    0x08U
#define LPUART_PINCFG_OFFSET    0x0CU
#define LPUART_BAUD_OFFSET      0x10U
#define LPUART_STAT_OFFSET      0x14U
#define LPUART_CTRL_OFFSET      0x18U
#define LPUART_DATA_OFFSET      0x1CU
#define LPUART_MATCH_OFFSET     0x20U
#define LPUART_MODIR_OFFSET     0x24U
#define LPUART_FIFO_OFFSET      0x28U
#define LPUART_WATER_OFFSET     0x2CU

/* LPUART STAT register bits */
#define LPUART_STAT_TDRE_MASK   (1UL << 23)  /* Transmit Data Register Empty */
#define LPUART_STAT_TC_MASK     (1UL << 22)  /* Transmission Complete */
#define LPUART_STAT_RDRF_MASK   (1UL << 21)  /* Receive Data Register Full */

/* LPUART CTRL register bits */
#define LPUART_CTRL_TE_MASK     (1UL << 19)  /* Transmitter Enable */
#define LPUART_CTRL_RE_MASK     (1UL << 18)  /* Receiver Enable */

/* LPUART4 base address for S32K388 */
#define LPUART4_BASE            0x40330000UL

/* Register access macros */
#define LPUART4_REG(offset)     (*(volatile uint32_t*)(LPUART4_BASE + (offset)))
#define LPUART4_STAT            LPUART4_REG(LPUART_STAT_OFFSET)
#define LPUART4_CTRL            LPUART4_REG(LPUART_CTRL_OFFSET)
#define LPUART4_DATA            LPUART4_REG(LPUART_DATA_OFFSET)
#define LPUART4_BAUD            LPUART4_REG(LPUART_BAUD_OFFSET)

void uart_poll_init(void) {
    /*
     * Assumes clock and pins are already configured by Mcu_Init and Port_Init.
     * Just ensure transmitter is enabled.
     */
    uint32_t ctrl = LPUART4_CTRL;
    ctrl |= LPUART_CTRL_TE_MASK;  /* Enable transmitter */
    LPUART4_CTRL = ctrl;
}

void uart_poll_putc(char c) {
    /* Wait for transmit data register to be empty */
    while ((LPUART4_STAT & LPUART_STAT_TDRE_MASK) == 0) {
        /* Busy wait */
    }
    /* Write character to data register */
    LPUART4_DATA = (uint32_t)c;
}

void uart_poll_puts(const char* str) {
    if (str == NULL) return;

    while (*str) {
        if (*str == '\n') {
            uart_poll_putc('\r');  /* Add CR before LF */
        }
        uart_poll_putc(*str++);
    }
}

void uart_poll_printf(const char* fmt, ...) {
    char buffer[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    uart_poll_puts(buffer);
}
