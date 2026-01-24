#include "systick.h"
#include "Pit_Ip.h"

/* FreeRTOS tick handler from port.c */
extern void xPortSysTickHandler(void);

volatile uint32 g_sysTick = 0;
static volatile uint8 g_schedulerStarted = 0;

void SysTick_Init(void) {
    /* Start the Gpt timer */
    Gpt_StartTimer(GPT_CHANNEL, GPT_PERIOD);
    /* Enable the Gpt notification to get the event for toggling the LED periodically */
    Gpt_EnableNotification(GPT_CHANNEL);
}

void SysTick_SetSchedulerStarted(void) {
    g_schedulerStarted = 1;
}

uint32 SysTick_GetTick(void) {
    return g_sysTick;
}

void SysTick_DelayMs(uint32 ms) {
    uint32 start = g_sysTick;
    while ((g_sysTick - start) < ms);
}

void SysTick_Custom_Handler(void) {
    g_sysTick++;

    /* Call FreeRTOS tick handler if scheduler is running */
    if (g_schedulerStarted) {
        xPortSysTickHandler();
    }
}
