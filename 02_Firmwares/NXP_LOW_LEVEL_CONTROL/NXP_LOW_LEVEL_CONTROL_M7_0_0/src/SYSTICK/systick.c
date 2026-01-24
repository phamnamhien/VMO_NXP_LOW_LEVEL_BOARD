#include "systick.h"
#include "Pit_Ip.h"

volatile uint32 g_sysTick = 0;

void SysTick_Init(void) {
    Gpt_StartTimer(GPT_CHANNEL, GPT_PERIOD);
    Gpt_EnableNotification(GPT_CHANNEL);
}

void SysTick_SetSchedulerStarted(void) {
    /* Not used - kept for compatibility */
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
}
