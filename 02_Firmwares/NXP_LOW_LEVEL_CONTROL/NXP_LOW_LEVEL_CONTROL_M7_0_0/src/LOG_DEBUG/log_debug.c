/**
 * @file    log_debug.c
 * @brief   Debug logging using NXP MCAL UART driver (Baremetal)
 */

#include "log_debug.h"
#include "CDD_Uart.h"
#include <string.h>
#include <stdio.h>

/*===========================================================================*/
/*                          CONFIGURATION                                     */
/*===========================================================================*/

#define LOG_UART_CHANNEL    0U
#define LOG_UART_TIMEOUT_US 100000U  /* 100ms timeout for full message */

/*===========================================================================*/
/*                          STATE                                             */
/*===========================================================================*/

static log_level_t current_level = LOG_LEVEL_INFO;

/*===========================================================================*/
/*                          PUBLIC API                                        */
/*===========================================================================*/

void log_init(void) {
    /* Nothing to initialize */
}

void log_start_flush_timer(void) {
    /* No-op for baremetal */
}

void log_set_level(log_level_t level) {
    current_level = level;
}

void log_write(log_level_t level, const char* tag, const char* format, ...) {
    if (level > current_level) return;

    char buffer[256];
    const char* level_str;

    switch(level) {
        case LOG_LEVEL_ERROR:   level_str = "E"; break;
        case LOG_LEVEL_WARN:    level_str = "W"; break;
        case LOG_LEVEL_INFO:    level_str = "I"; break;
        case LOG_LEVEL_DEBUG:   level_str = "D"; break;
        case LOG_LEVEL_VERBOSE: level_str = "V"; break;
        default: return;
    }

    /* Format: [LEVEL] (TAG): message */
    int len = snprintf(buffer, sizeof(buffer), "[%s] (%s): ", level_str, tag);

    va_list args;
    va_start(args, format);
    len += vsnprintf(buffer + len, sizeof(buffer) - len - 3, format, args);
    va_end(args);

    /* Add CRLF */
    buffer[len++] = '\r';
    buffer[len++] = '\n';
    buffer[len] = '\0';

    /* Blocking UART send - wait until complete */
    (void)Uart_SyncSend(LOG_UART_CHANNEL, (const uint8*)buffer, (uint32)len, LOG_UART_TIMEOUT_US);
}

void log_flush(void) {
    /* No-op - Uart_SyncSend already blocks */
}

void log_flush_blocking(void) {
    /* No-op - Uart_SyncSend already blocks */
}
