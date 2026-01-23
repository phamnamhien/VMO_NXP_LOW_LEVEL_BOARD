/**
 * @file    uart_poll.h
 * @brief   Simple polling-based UART output for debug (no interrupts)
 */

#ifndef UART_POLL_H
#define UART_POLL_H

#include <stdint.h>

/**
 * @brief Initialize LPUART4 for polling mode output
 * @note  Call this AFTER Port_Init() and clock setup
 */
void uart_poll_init(void);

/**
 * @brief Send a single character (blocking/polling)
 * @param c Character to send
 */
void uart_poll_putc(char c);

/**
 * @brief Send a string (blocking/polling)
 * @param str Null-terminated string
 */
void uart_poll_puts(const char* str);

/**
 * @brief Send formatted string (like printf)
 * @param fmt Format string
 * @param ... Arguments
 */
void uart_poll_printf(const char* fmt, ...);

#endif /* UART_POLL_H */
