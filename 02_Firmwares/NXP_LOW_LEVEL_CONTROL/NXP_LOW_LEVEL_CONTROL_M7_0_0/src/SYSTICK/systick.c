#include "systick.h"
#include "Pit_Ip.h"

volatile uint32 g_sysTick = 0;

void SysTick_Init(void) {
    /* Start the Gpt timer */
    Gpt_StartTimer(GPT_CHANNEL, GPT_PERIOD);
    /* Enable the Gpt notification to get the event for toggling the LED periodically */
    Gpt_EnableNotification(GPT_CHANNEL);
}

uint32 SysTick_GetTick(void) {
    return g_sysTick;
}

void SysTick_DelayMs(uint32 ms) {
    uint32 start = g_sysTick;
    while ((g_sysTick - start) < ms);
}

void OsIf_Millisecond(void) {
    g_sysTick++;
}

void SysTick_Custom_Handler(void) {
    g_sysTick++;
}
