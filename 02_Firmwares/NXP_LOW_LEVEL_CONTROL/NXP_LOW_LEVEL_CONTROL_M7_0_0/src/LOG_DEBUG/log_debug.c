/**
 * @file    log_debug.c
 * @brief   Debug logging using simple direct UART access
 */
#include "log_debug.h"
#include "simple_uart.h"
#include <string.h>
#include <stdio.h>

/* FreeRTOS for timestamps */
#include "FreeRTOS.h"
#include "task.h"

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
    simple_uart_init();
    pre_scheduler_ms = 0;
    is_initialized = 1;
}

void log_start_flush_timer(void) {
    /* No-op - using blocking mode */
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

    /* Format header */
    int len = snprintf(buffer, sizeof(buffer), "[%lu.%03lu] %s (%s): ",
                      (unsigned long)sec, (unsigned long)ms, level_str, tag);

    /* Format message */
    va_list args;
    va_start(args, format);
    len += vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
    va_end(args);

    /* Add newline */
    if (len < (int)sizeof(buffer) - 2) {
        buffer[len++] = '\r';
        buffer[len++] = '\n';
        buffer[len] = '\0';
    }

    /* Send via simple UART (blocking) */
    simple_uart_puts(buffer);
}

void log_flush(void) {
    /* No-op - using blocking mode */
}

void log_flush_blocking(void) {
    /* No-op - already blocking */
}

/* Test function for simple UART */
void log_test_uart(void) {
    simple_uart_test();
}
