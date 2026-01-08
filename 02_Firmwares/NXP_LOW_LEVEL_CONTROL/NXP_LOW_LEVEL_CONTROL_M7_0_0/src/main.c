/*
 * main.c
 *
 *  Created on: Jan 6, 2026
 *      Author: phamnamhien
 */

#ifdef __cplusplus
extern "C" {
#endif
/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Clock_Ip.h"
#include "Lpuart_Uart_Ip.h"
#include "IntCtrl_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "string.h"

#include "systick.h"
#include "log_debug.h"
#include "test_lan9646.h"


/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
#define clockConfig &Clock_Ip_aClockConfig[0]
/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/
static const char* TAG = "MAIN";
/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/
/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/
/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/
extern uint32_t SystemCoreClock;

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/


/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/
//int main(void)
//{
//    /* Initialize Clock */
//    Clock_Ip_Init(clockConfig);
//    /* Initialize SysTick */
//    SysTick_Init();
//    /* Initialize Pin */
//    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals,
//                       g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);
//
//    /* set ISR */
//    IntCtrl_Ip_Init(&IntCtrlConfig_0);
//
//    log_init();
//    log_set_level(LOG_LEVEL_DEBUG);
//
//    LOG_I(TAG, "Start");
//    while(1) {
//    	LOG_I(TAG, "Hi");
//    	SysTick_DelayMs(100);
//    }
//}
int main(void)
{
	Clock_Ip_Init(clockConfig);
    SysTick_Init();
    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals,
                           g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);
    IntCtrl_Ip_Init(&IntCtrlConfig_0);

    log_init();

    LOG_I(TAG, "System Start");

    Test_LAN9646_Init();

    while(1) {
        Test_LAN9646_PeriodicRead();
    }
}

#ifdef __cplusplus
}
#endif

/** @} */

