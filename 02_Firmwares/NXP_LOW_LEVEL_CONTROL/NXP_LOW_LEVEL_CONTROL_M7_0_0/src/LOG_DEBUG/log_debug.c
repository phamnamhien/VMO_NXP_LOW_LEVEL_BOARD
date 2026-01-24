/*
 * log_debug.c
 */
#include "log_debug.h"
#include "OsIf.h"
#include "string.h"

/* FreeRTOS for timestamps after scheduler starts */
#include "FreeRTOS.h"
#include "task.h"

/* ARM Cortex-M SysTick for pre-scheduler timestamps */
#define SYSTICK_LOAD  (*(volatile uint32_t*)0xE000E014)
#define SYSTICK_VAL   (*(volatile uint32_t*)0xE000E018)
#define SYSTICK_CTRL  (*(volatile uint32_t*)0xE000E010)

static log_level_t current_level = LOG_LEVEL_INFO;
static uint8_t is_initialized = 0;
static uint32_t log_start_systick = 0;
static uint32_t pre_scheduler_ms = 0;

void log_init(void) {
    /* Note: Uart_Init(NULL_PTR) must be called BEFORE log_init() */
    log_start_systick = SYSTICK_VAL;
    pre_scheduler_ms = 0;
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

    /* Get elapsed time */
    uint32 elapsed_ms;
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        /* Use FreeRTOS tick count after scheduler starts */
        TickType_t current_tick = xTaskGetTickCount();
        elapsed_ms = (uint32)(current_tick * 1000U / configTICK_RATE_HZ);
    } else {
        /* Simple incrementing counter before scheduler (rough estimate) */
        /* Each LOG call takes ~1ms at 115200 baud for ~100 chars */
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

    uint32 bytesTransferred = 0;
    Std_ReturnType ret = Uart_AsyncSend(LOG_UART_CHANNEL, (const uint8*)buffer, len);
    if (ret == E_OK) {
        while (Uart_GetStatus(LOG_UART_CHANNEL, &bytesTransferred, UART_SEND) == UART_STATUS_OPERATION_ONGOING && timeout-- > 0);
    }
}
