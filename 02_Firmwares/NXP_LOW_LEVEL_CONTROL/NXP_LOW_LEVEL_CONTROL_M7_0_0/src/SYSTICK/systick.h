#ifndef SYSTICK_H
#define SYSTICK_H

#include "Std_Types.h"
#include "Gpt.h"

#define GPT_CHANNEL     GptConf_GptChannelConfiguration_GptChannelConfiguration_1
#define GPT_PERIOD      40000UL  /* 40000 ticks @ 40MHz = 1ms */

void SysTick_Init(void);
void SysTick_SetSchedulerStarted(void);  /* Call after vTaskStartScheduler() */
uint32 SysTick_GetTick(void);
void SysTick_DelayMs(uint32 ms);

/* GPT notification callback - called every 1ms */
void SysTick_Custom_Handler(void);

#endif

