#include "systick.h"



volatile uint32 g_sysTick = 0;

void SysTick_Custom_Handler(void) {
    g_sysTick++;
}

void SysTick_Init(void) {
    /* Config SysTick: 1ms tick */
    /* Initialize PIT instance 0 - Channel 0 */
    Pit_Ip_Init(PIT_INST_0, &PIT_0_InitConfig_PB);
    /* Initialize channel 0 */
    Pit_Ip_InitChannel(PIT_INST_0, PIT_0_CH_0);
    /* Enable channel interrupt PIT_0 - CH_0 */
    Pit_Ip_EnableChannelInterrupt(PIT_INST_0, CH_0);
    /* Start channel CH_0 */
    Pit_Ip_StartChannel(PIT_INST_0, CH_0, PIT_PERIOD);
}

uint32 SysTick_GetTick(void) {
    return g_sysTick;
}

void SysTick_DelayMs(uint32 ms) {
    uint32 start = SysTick_GetTick();
    uint32 target = start + ms;

    /* Xử lý overflow */
    if (target < start) {
        /* Wait for overflow */
        while(SysTick_GetTick() > start);
    }

    while(SysTick_GetTick() < target);
}

