/*
 * log_debug.c
 * Non-blocking UART logging with ring buffer and auto-flush timer
 */
#include "log_debug.h"
#include "OsIf.h"
#include "string.h"

/* FreeRTOS for timestamps, synchronization and timer */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*===========================================================================*/
/*                          CONFIGURATION                                     */
/*===========================================================================*/

/* Ring buffer size - must be power of 2 for efficient wrap */
#define LOG_RING_BUFFER_SIZE    4096U
#define LOG_RING_BUFFER_MASK    (LOG_RING_BUFFER_SIZE - 1U)

/* TX buffer for DMA - size of one log message */
#define LOG_TX_BUFFER_SIZE      256U

/* Timer period for auto-flush (ms) */
#define LOG_FLUSH_PERIOD_MS     10U

/*===========================================================================*/
/*                          RING BUFFER                                       */
/*===========================================================================*/

static uint8_t g_ring_buffer[LOG_RING_BUFFER_SIZE];
static volatile uint32_t g_ring_head = 0;  /* Write position */
static volatile uint32_t g_ring_tail = 0;  /* Read position */

/* TX buffer for current DMA transfer */
static uint8_t g_tx_buffer[LOG_TX_BUFFER_SIZE];
static volatile uint8_t g_tx_busy = 0;

/*===========================================================================*/
/*                          STATE                                             */
/*===========================================================================*/

static log_level_t current_level = LOG_LEVEL_INFO;
static uint8_t is_initialized = 0;
static uint32_t pre_scheduler_ms = 0;

/* Auto-flush timer */
static TimerHandle_t g_flush_timer = NULL;

/*===========================================================================*/
/*                          RING BUFFER HELPERS                                */
/*===========================================================================*/

static inline uint32_t ring_available(void) {
    return (g_ring_head - g_ring_tail) & LOG_RING_BUFFER_MASK;
}

static inline uint32_t ring_free(void) {
    return (LOG_RING_BUFFER_SIZE - 1U) - ring_available();
}

static void ring_write(const uint8_t* data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        g_ring_buffer[g_ring_head] = data[i];
        g_ring_head = (g_ring_head + 1U) & LOG_RING_BUFFER_MASK;
    }
}

static uint32_t ring_read(uint8_t* data, uint32_t max_len) {
    uint32_t available = ring_available();
    uint32_t len = (available < max_len) ? available : max_len;

    for (uint32_t i = 0; i < len; i++) {
        data[i] = g_ring_buffer[g_ring_tail];
        g_ring_tail = (g_ring_tail + 1U) & LOG_RING_BUFFER_MASK;
    }
    return len;
}

/*===========================================================================*/
/*                          UART TX PUMP                                       */
/*===========================================================================*/

/* Start next TX transfer if data available and not busy (call from critical section) */
static void log_pump_tx_unsafe(void) {
    if (g_tx_busy) {
        /* Check if previous transfer completed */
        uint32 bytesTransferred = 0;
        Uart_StatusType status = Uart_GetStatus(LOG_UART_CHANNEL, &bytesTransferred, UART_SEND);
        if (status == UART_STATUS_OPERATION_ONGOING) {
            return;  /* Still busy */
        }
        g_tx_busy = 0;
    }

    /* Get data from ring buffer */
    uint32_t len = ring_read(g_tx_buffer, LOG_TX_BUFFER_SIZE);
    if (len == 0) {
        return;  /* No data */
    }

    /* Start async transfer */
    g_tx_busy = 1;
    Std_ReturnType ret = Uart_AsyncSend(LOG_UART_CHANNEL, g_tx_buffer, len);
    if (ret != E_OK) {
        g_tx_busy = 0;  /* Failed to start */
    }
}

/*===========================================================================*/
/*                          TIMER CALLBACK                                     */
/*===========================================================================*/

static void log_flush_timer_callback(TimerHandle_t xTimer) {
    (void)xTimer;

    /* Pump TX from timer context (runs in timer task) */
    taskENTER_CRITICAL();
    log_pump_tx_unsafe();
    taskEXIT_CRITICAL();
}

/*===========================================================================*/
/*                          PUBLIC API                                         */
/*===========================================================================*/

void log_init(void) {
    /* Reset ring buffer */
    g_ring_head = 0;
    g_ring_tail = 0;
    g_tx_busy = 0;
    pre_scheduler_ms = 0;
    is_initialized = 1;
}

void log_start_flush_timer(void) {
    /* Create and start auto-flush timer (call after scheduler started) */
    if (g_flush_timer == NULL) {
        g_flush_timer = xTimerCreate(
            "LogFlush",
            pdMS_TO_TICKS(LOG_FLUSH_PERIOD_MS),
            pdTRUE,  /* Auto-reload */
            NULL,
            log_flush_timer_callback
        );

        if (g_flush_timer != NULL) {
            xTimerStart(g_flush_timer, 0);
        }
    }
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

    /*
     * Before scheduler starts: use blocking mode to ensure output
     * After scheduler starts: use ring buffer for non-blocking
     */
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {
        /* Blocking mode - wait for UART to complete */
        uint32 timeout = 0xFFFFFF;
        uint32 bytesTransferred = 0;
        Std_ReturnType ret = Uart_AsyncSend(LOG_UART_CHANNEL, (const uint8*)buffer, len);
        if (ret == E_OK) {
            while (Uart_GetStatus(LOG_UART_CHANNEL, &bytesTransferred, UART_SEND) == UART_STATUS_OPERATION_ONGOING && timeout-- > 0);
        }
    } else {
        /* Non-blocking mode - add to ring buffer */
        taskENTER_CRITICAL();

        if ((uint32_t)len <= ring_free()) {
            ring_write((const uint8_t*)buffer, len);
        }
        /* else: buffer full, drop message */

        /* Try to pump TX immediately */
        log_pump_tx_unsafe();

        taskEXIT_CRITICAL();
    }
}

/* Call this periodically to drain the ring buffer */
void log_flush(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        taskENTER_CRITICAL();
        log_pump_tx_unsafe();
        taskEXIT_CRITICAL();
    }
}

/* Blocking flush - wait for all data to be sent */
void log_flush_blocking(void) {
    uint32_t timeout = 100000;

    while (ring_available() > 0 && timeout-- > 0) {
        log_flush();

        /* Small delay */
        volatile uint32_t delay = 1000;
        while (delay-- > 0);
    }

    /* Wait for last transfer to complete */
    if (g_tx_busy) {
        timeout = 100000;
        uint32 bytesTransferred = 0;
        while (Uart_GetStatus(LOG_UART_CHANNEL, &bytesTransferred, UART_SEND) == UART_STATUS_OPERATION_ONGOING && timeout-- > 0);
        g_tx_busy = 0;
    }
}
