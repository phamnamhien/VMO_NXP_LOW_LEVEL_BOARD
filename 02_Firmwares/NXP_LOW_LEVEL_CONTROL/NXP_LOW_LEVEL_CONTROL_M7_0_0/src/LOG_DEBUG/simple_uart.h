/**
 * @file    simple_uart.h
 * @brief   Simple UART driver using direct register access
 * @note    Independent of NXP MCAL drivers - uses polling mode for reliability
 */

#ifndef SIMPLE_UART_H_
#define SIMPLE_UART_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

/*===========================================================================*/
/*                          CONFIGURATION                                     */
/*===========================================================================*/

/* LPUART instance to use (0-15 for S32K388) */
#define SIMPLE_UART_INSTANCE    1   /* LPUART1 - PTA18(TX)/PTA19(RX) */

/* Baud rate - must match clock configuration */
#define SIMPLE_UART_BAUD_RATE   115200

/* LPUART clock frequency (from ClockYaml.txt: 40 MHz) */
#define SIMPLE_UART_CLK_FREQ    40000000UL

/*===========================================================================*/
/*                          PUBLIC API                                         */
/*===========================================================================*/

/**
 * @brief Initialize UART with direct register access
 * @note  Call this AFTER Port_Init() and clock configuration
 */
void simple_uart_init(void);

/**
 * @brief Send a single character (blocking)
 */
void simple_uart_putc(char c);

/**
 * @brief Send a string (blocking)
 */
void simple_uart_puts(const char* str);

/**
 * @brief Printf-style output (blocking)
 */
void simple_uart_printf(const char* format, ...);

/**
 * @brief Check if UART TX is ready
 */
bool simple_uart_tx_ready(void);

/**
 * @brief Test function - sends test pattern
 */
void simple_uart_test(void);

#endif /* SIMPLE_UART_H_ */
