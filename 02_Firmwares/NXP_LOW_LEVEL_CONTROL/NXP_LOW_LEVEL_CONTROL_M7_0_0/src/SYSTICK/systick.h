#ifndef SYSTICK_H
#define SYSTICK_H

#include "Std_Types.h"
#include "Gpt.h"

#define GPT_CHANNEL     GptConf_GptChannelConfiguration_GptChannelConfiguration_0
#define GPT_PERIOD      40000UL

void SysTick_Init(void);
uint32 SysTick_GetTick(void);
void SysTick_DelayMs(uint32 ms);

#endif

