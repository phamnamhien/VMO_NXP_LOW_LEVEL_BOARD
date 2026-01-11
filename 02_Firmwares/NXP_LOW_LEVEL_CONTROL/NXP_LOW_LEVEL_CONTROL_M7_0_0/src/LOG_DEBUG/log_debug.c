/*
 * log_debug.c
 *
 *  Created on: Jan 6, 2026
 *      Author: phamnamhien
 */
#include "log_debug.h"
#include "systick.h"
#include "string.h"

static log_level_t current_level = LOG_LEVEL_INFO;
static uint8_t is_initialized = 0;
void log_init(void) {
	if(!is_initialized) {
	    Lpuart_Uart_Ip_Init(LOG_UART_CHANNEL, &Lpuart_Uart_Ip_xHwConfigPB_0);
	    is_initialized = 1;
	}
}

void log_set_level(log_level_t level) {
    current_level = level;
}

void log_write(log_level_t level, const char* tag, const char* format, ...) {
    if (level > current_level) return;

    char buffer[256];
    char* level_str;
    uint32 remainingBytes = 0;
    uint32 timeout = 0xFFFFFF;
    Lpuart_Uart_Ip_StatusType status;

    switch(level) {
        case LOG_LEVEL_ERROR: level_str = "E"; break;
        case LOG_LEVEL_WARN:  level_str = "W"; break;
        case LOG_LEVEL_INFO:  level_str = "I"; break;
        case LOG_LEVEL_DEBUG: level_str = "D"; break;
        case LOG_LEVEL_VERBOSE: level_str = "V"; break;
        default: return;
    }

    /* Lấy timestamp */
    uint32 tick = SysTick_GetTick();
    uint32 sec = tick / 1000;
    uint32 ms = tick % 1000;

    /* Format: [timestamp] LEVEL (TAG): message */
    int len = snprintf(buffer, sizeof(buffer), "[%lu.%03lu] %s (%s): ",
                      sec, ms, level_str, tag);

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
}
