/*
 * log_debug.c
 *
 *  Created on: Jan 6, 2026
 *      Author: phamnamhien
 */
#include "log_debug.h"
#include "string.h"

static log_level_t current_level = LOG_LEVEL_INFO;
static uint32 log_counter = 0;

void log_init(void)
{
    log_counter = 0;
}

void log_set_level(log_level_t level)
{
    current_level = level;
}

void log_write(log_level_t level, const char* tag, const char* format, ...)
{
    if (level > current_level) return;

    char buffer[256];
    char* level_str;
    uint32 remainingBytes = 0;
    uint32 timeout = 0xFFFFFF;
    Lpuart_Uart_Ip_StatusType status;

    switch(level) {
        case LOG_LEVEL_ERROR: level_str = "E"; break;
        case LOG_LEVEL_WARN: level_str = "W"; break;
        case LOG_LEVEL_INFO: level_str = "I"; break;
        case LOG_LEVEL_DEBUG: level_str = "D"; break;
        case LOG_LEVEL_VERBOSE: level_str = "V"; break;
        default: return;
    }

    int len = snprintf(buffer, sizeof(buffer), "%s (%s): ", level_str, tag);

    va_list args;
    va_start(args, format);
    len += vsnprintf(buffer + len, sizeof(buffer) - len, format, args);
    va_end(args);

    len += snprintf(buffer + len, sizeof(buffer) - len, "\r\n");

    status = Lpuart_Uart_Ip_AsyncSend(LOG_UART_CHANNEL, (const uint8*)buffer, len);

    if (status == LPUART_UART_IP_STATUS_SUCCESS) {
        do {
            status = Lpuart_Uart_Ip_GetTransmitStatus(LOG_UART_CHANNEL, &remainingBytes);
        } while (status == LPUART_UART_IP_STATUS_BUSY && timeout-- > 0);
    }

    log_counter++;
}
