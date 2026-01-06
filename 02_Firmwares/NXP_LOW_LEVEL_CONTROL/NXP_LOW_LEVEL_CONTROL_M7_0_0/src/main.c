/*
 * main.c
 *
 *  Created on: Jan 6, 2026
 *      Author: phamnamhien
 */


#include "Lpuart_Uart_Ip.h"
#include "Clock_Ip.h"
#include "IntCtrl_Ip.h"
#include "Siul2_Port_Ip.h"
#include "string.h"

#include "log_debug.h"

static const char* TAG = "MAIN";


int main(void)
{
    /* Init clock */
    Clock_Ip_Init(&Clock_Ip_aClockConfig[0]);

#if defined (FEATURE_CLOCK_IP_HAS_SPLL_CLK)
    while (CLOCK_IP_PLL_LOCKED != Clock_Ip_GetPllStatus())
    {
        /* Wait PLL locked */
    }
    Clock_Ip_DistributePll();
#endif

    /* Init pins */
    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals,
                       g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);

    /* Init IRQ */
    IntCtrl_Ip_Init(&IntCtrlConfig_0);

    /* Init LPUART4 for Debug*/
    Lpuart_Uart_Ip_Init(LOG_UART_CHANNEL, &Lpuart_Uart_Ip_xHwConfigPB_0);

    /* Init log */
    log_init();
    log_set_level(LOG_LEVEL_DEBUG);



    while(1) {
        LOG_I(TAG, "System started");
        LOG_D(TAG, "Clock: %d Hz", 80000000);
        LOG_W(TAG, "Warning message");
        LOG_E(TAG, "Error message");
        for(uint32_t i = 0; i < 10000000; i++) {

        }
    }

    return 0;
}
