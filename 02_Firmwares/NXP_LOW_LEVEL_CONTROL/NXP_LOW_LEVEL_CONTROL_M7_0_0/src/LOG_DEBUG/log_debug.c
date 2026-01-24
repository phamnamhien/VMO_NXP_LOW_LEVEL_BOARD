/*
 * log_debug.c
 * Simple blocking UART logging (reliable)
 */
#include "log_debug.h"
#include "OsIf.h"
#include "string.h"

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
    pre_scheduler_ms = 0;
    is_initialized = 1;
}

void log_start_flush_timer(void) {
    /* No-op in blocking mode */
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
    uint32 elapsed_ms;
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        TickType_t current_tick = xTaskGetTickCount();
        elapsed_ms = (uint32)(current_tick * 1000U / configTICK_RATE_HZ);
    } else {
        elapsed_ms = pre_scheduler_ms++;
    }

    uint32 sec = elapsed_ms / 1000U;
    uint32 ms = elapsed_ms % 1000U;

    int len = snprintf(buffer, sizeof(buffer), "[%lu.%03lu] %s (%s): ",
                      sec, ms, level_str, tag);

    va_list args;
    va_start(args, format);
    len += vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
    va_end(args);

    len += snprintf(buffer + len, sizeof(buffer) - len, "\r\n");

    /* Blocking send - wait for UART to complete */
    uint32 timeout = 0xFFFFFF;
    uint32 bytesTransferred = 0;
    Std_ReturnType ret = Uart_AsyncSend(LOG_UART_CHANNEL, (const uint8*)buffer, len);
    if (ret == E_OK) {
        while (Uart_GetStatus(LOG_UART_CHANNEL, &bytesTransferred, UART_SEND) == UART_STATUS_OPERATION_ONGOING && timeout-- > 0);
    }
}

void log_flush(void) {
    /* No-op in blocking mode */
}

void log_flush_blocking(void) {
    /* No-op in blocking mode - already blocking */
}
