/*
 * log_debug.c
 */
#include "log_debug.h"
#include "string.h"
#include "OsIf.h"
#include "osif_rtd_port.h"

#ifdef USING_OS_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#else
#include "systick.h"
#endif

static log_level_t current_level = LOG_LEVEL_INFO;
static uint8_t is_initialized = 0;

#ifdef USING_OS_FREERTOS
static SemaphoreHandle_t log_mutex = NULL;
#endif

void log_init(void) {
    if(!is_initialized) {
#ifdef USING_OS_FREERTOS
        log_mutex = xSemaphoreCreateMutex();
#endif
        is_initialized = 1;
    }
}

void log_set_level(log_level_t level) {
    current_level = level;
}

static uint32_t log_get_tick(void) {
#ifdef USING_OS_FREERTOS
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    } else {
        return OsIf_GetMilliseconds();
    }
#else
    return SysTick_GetTick();
#endif
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

#ifdef USING_OS_FREERTOS
    uint8_t mutex_taken = 0;
    if (log_mutex != NULL && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (xSemaphoreTake(log_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            mutex_taken = 1;
        }
    }
#endif

    uint32 tick = log_get_tick();
    uint32 sec = tick / 1000;
    uint32 ms = tick % 1000;

    int len = snprintf(buffer, sizeof(buffer), "[%lu.%03lu] %s (%s): ",
                      (unsigned long)sec, (unsigned long)ms, level_str, tag);

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

#ifdef USING_OS_FREERTOS
    if (mutex_taken) {
        xSemaphoreGive(log_mutex);
    }
#endif
}

