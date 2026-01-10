/**
 * \file            main.c
 * \brief           NXP_LOW_LEVEL_CONTROL
 */

/*
 * Copyright (c) 2026 Pham Nam Hien
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of NXP_LOW_LEVEL_CONTROL
 *
 * Author:          Pham Nam Hien
 * Version:         v1.0.0
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
/* Standard includes */
#include <stdint.h>
#include <stdbool.h>
#include "string.h"
/* AUTOSAR RTD includes */
#include "Clock_Ip.h"
#include "Lpuart_Uart_Ip.h"
#include "IntCtrl_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
/* Application includes */
#include "systick.h"
#include "log_debug.h"
//#include "lan9646_soft_i2c_init_example.h"
// #include "lan9646_readonly_test.h"
#include "lan9646_monitor_fixed.h"
#include "lan9646_port6_test.h"
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
static void system_init(void);
static void delay_ms(uint32_t ms);
/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                       MAIN APPLICATION
==================================================================================================*/

int main(void)
{
    /* System initialization */
    system_init();

    /* Initialize log system */
    log_init();
    log_set_level(LOG_LEVEL_INFO);

    LOG_I(TAG, "");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "  S32K388 - NXP Low Level Control");
    LOG_I(TAG, "  LAN9646 Soft I2C Test");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "");

    /* ========== 1. INIT LAN9646 ========== */
    LOG_I(TAG, "Starting LAN9646 initialization...");
    lan9646_init_monitor();
//    lan9646_try_enable_port6();

//    /* ========== 2. CONFIG VLAN ========== */
//    lan9646_check_vlan_config();
//    lan9646_setup_vlan_table_port6();
//    lan9646_enable_unknown_unicast_to_port6();
//
//    /* ========== 3. CONFIG FORWARDING ========== */
//    lan9646_port6_enable_forwarding();
//
//    /* ========== 4. CONFIG PORT MIRRORING ========== */
//    lan9646_enable_port_mirroring_to_port6();
//
//    /* ========== 5. DUMP CONFIG ========== */
//    lan9646_port6_dump_mac();

    /* ========== 6. ĐỢI SWITCH ỔN ĐỊNH ========== */
    LOG_I(TAG, "");
    LOG_I(TAG, "Waiting 3 seconds for switch to stabilize...");
    SysTick_DelayMs(3000);

    /* ========== 7. TEST ========== */
    lan9646_port6_read_mib();
    lan9646_port6_test_rx_from_port1();

    /* ========== 8. MAIN LOOP ========== */
    LOG_I(TAG, "");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "  System Ready - Entering Main Loop");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "");

    while(1) {
        /* Monitor Port 6 traffic mỗi 2 giây */
        lan9646_port6_monitor_traffic();
        SysTick_DelayMs(2000);
    }
    return 0;
}



/**
 * \brief           Simple delay in milliseconds
 * \param[in]       ms: Delay time in milliseconds
 * \note            This is a blocking delay
 *                  Adjust the loop count based on CPU frequency
 */
static void
delay_ms(uint32_t ms) {
    volatile uint32_t i, j;

    /* Assuming 160MHz CPU clock */
    /* Adjust multiplier for your actual CPU frequency */
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 40000; j++) {
            __asm("NOP");
        }
    }
}

/**
 * \brief           System initialization
 * \note            Initialize clocks, ports, and other peripherals
 */
static void
system_init(void) {
  	Clock_Ip_Init(clockConfig);
    SysTick_Init();
    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals,
                           g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);
    IntCtrl_Ip_Init(&IntCtrlConfig_0);
}


#ifdef __cplusplus
}
#endif

/** @} */

