#ifndef SYSTICK_H
#define SYSTICK_H

#include "Std_Types.h"
#include "Pit_Ip.h"

#include "IntCtrl_Ip.h"


/* PIT instance used - 0 */
#define PIT_INST_0 	0U
/* PIT Channel used - 0 */
#define CH_0 0U
/* PIT time-out period - equivalent to 1ms */
#define PIT_PERIOD 	12000UL



void SysTick_Init(void);
uint32 SysTick_GetTick(void);
void SysTick_DelayMs(uint32 ms);


#endif /* SYSTICK_H */

