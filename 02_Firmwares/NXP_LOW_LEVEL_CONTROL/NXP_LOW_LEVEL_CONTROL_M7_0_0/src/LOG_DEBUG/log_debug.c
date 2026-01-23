/*
 * log_debug.c
 */
#include "log_debug.h"
#include "OsIf.h"
#include "string.h"

static log_level_t current_level = LOG_LEVEL_INFO;
static uint8_t is_initialized = 0;
static uint32 log_start_time = 0;

void log_init(void) {
    /* Note: Uart_Init(NULL_PTR) must be called in main.c BEFORE calling log_init() */
    log_start_time = OsIf_GetCounter(OSIF_COUNTER_SYSTEM);
    is_initialized = 1;
}

void log_set_level(log_level_t level) {
    current_level = level;
}

void log_write(log_level_t level, const char* tag, const char* format, ...) {
    if (level > current_level) return;

    char buffer[256];
    char* level_str;
    uint32 timeout = 0xFFFFFF;

    switch(level) {
        case LOG_LEVEL_ERROR:   level_str = "E"; break;
        case LOG_LEVEL_WARN:    level_str = "W"; break;
        case LOG_LEVEL_INFO:    level_str = "I"; break;
        case LOG_LEVEL_DEBUG:   level_str = "D"; break;
        case LOG_LEVEL_VERBOSE: level_str = "V"; break;
        default: return;
    }

    /* Get elapsed time in microseconds since log_init */
    uint32 elapsed_us = OsIf_GetElapsed(&log_start_time, OSIF_COUNTER_SYSTEM);
    uint32 elapsed_ms = elapsed_us / 1000U;
    uint32 sec = elapsed_ms / 1000U;
    uint32 ms = elapsed_ms % 1000U;

    int len = snprintf(buffer, sizeof(buffer), "[%lu.%03lu] %s (%s): ",
                      sec, ms, level_str, tag);

    va_list args;
    va_start(args, format);
    len += vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
    va_end(args);

    len += snprintf(buffer + len, sizeof(buffer) - len, "\r\n");

    uint32 bytesTransferred = 0;
    Std_ReturnType ret = Uart_AsyncSend(LOG_UART_CHANNEL, (const uint8*)buffer, len);
    if (ret == E_OK) {
        while (Uart_GetStatus(LOG_UART_CHANNEL, &bytesTransferred, UART_SEND) == UART_STATUS_OPERATION_ONGOING && timeout-- > 0);
    }
}
