/**
 * @file    log_debug.c
 * @brief   Debug logging using NXP MCAL UART driver
 * @note    Uses Uart_SyncSend for reliable blocking transmission
 */

#include "log_debug.h"
#include "CDD_Uart.h"
#include <string.h>
#include <stdio.h>

/* FreeRTOS for timestamps */
#include "FreeRTOS.h"
#include "task.h"

/*===========================================================================*/
/*                          CONFIGURATION                                     */
/*===========================================================================*/

/* UART channel configured in EB Tresos */
#define LOG_UART_CHANNEL    0U

/* Timeout for UART transmission (microseconds) */
#define LOG_UART_TIMEOUT_US 10000U  /* 10ms timeout - sufficient for 256 bytes at 115200 baud */

/*===========================================================================*/
/*                          STATE                                             */
/*===========================================================================*/

static log_level_t current_level = LOG_LEVEL_INFO;
static uint8_t is_initialized = 0;
static uint32_t pre_scheduler_ms = 0;

/*===========================================================================*/
/*                          PUBLIC API                                         */
/*===========================================================================*/

void log_init(void) {
    /* Uart_Init() should be called before this in main() */
    pre_scheduler_ms = 0;
    is_initialized = 1;
}

void log_start_flush_timer(void) {
    /* No-op - using blocking Uart_SyncSend */
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

    /* Get elapsed time */
    uint32_t elapsed_ms;
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        TickType_t current_tick = xTaskGetTickCount();
        elapsed_ms = (uint32_t)(current_tick * 1000U / configTICK_RATE_HZ);
    } else {
        elapsed_ms = pre_scheduler_ms++;
    }

    uint32_t sec = elapsed_ms / 1000U;
    uint32_t ms = elapsed_ms % 1000U;

    /* Format the complete message */
    int len = snprintf(buffer, sizeof(buffer), "[%lu.%03lu] %s (%s): ",
                      (unsigned long)sec, (unsigned long)ms, level_str, tag);

    va_list args;
    va_start(args, format);
    len += vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
    va_end(args);

    /* Add CRLF */
    if (len < (int)sizeof(buffer) - 2) {
        buffer[len++] = '\r';
        buffer[len++] = '\n';
        buffer[len] = '\0';
    }

    /*
     * Use Uart_SyncSend - blocking send with timeout
     * This is the proper MCAL way to send data synchronously
     */
    (void)Uart_SyncSend(LOG_UART_CHANNEL, (const uint8*)buffer, (uint32)len, LOG_UART_TIMEOUT_US);
}

void log_flush(void) {
    /* No-op - Uart_SyncSend already blocks until complete */
}

void log_flush_blocking(void) {
    /* No-op - Uart_SyncSend already blocks until complete */
}
