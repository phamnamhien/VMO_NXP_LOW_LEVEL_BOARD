/**
 * @file    log_debug.h
 * @brief   Debug logging using NXP MCAL UART driver
 * @note    Uses Uart_SyncSend for reliable blocking transmission
 */

#ifndef LOG_DEBUG_H_
#define LOG_DEBUG_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

/* Debug levels */
typedef enum {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VERBOSE
} log_level_t;

/* Log macros */
#define LOG_E(tag, ...) log_write(LOG_LEVEL_ERROR, tag, __VA_ARGS__)
#define LOG_W(tag, ...) log_write(LOG_LEVEL_WARN, tag, __VA_ARGS__)
#define LOG_I(tag, ...) log_write(LOG_LEVEL_INFO, tag, __VA_ARGS__)
#define LOG_D(tag, ...) log_write(LOG_LEVEL_DEBUG, tag, __VA_ARGS__)
#define LOG_V(tag, ...) log_write(LOG_LEVEL_VERBOSE, tag, __VA_ARGS__)

/* Function prototypes */
void log_init(void);
void log_set_level(log_level_t level);
void log_write(log_level_t level, const char* tag, const char* format, ...);

/* Compatibility functions (no-op when using Uart_SyncSend) */
void log_start_flush_timer(void);
void log_flush(void);
void log_flush_blocking(void);

#endif /* LOG_DEBUG_H_ */
