/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7 
*   Platform             : CORTEXM
*   Peripheral           : SIUL2
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 6.0.0
*   Build Version        : S32K3_S32M27x_Real-Time_Drivers_AUTOSAR_R21-11_Version_6_0_0_D2506_ASR_REL_4_7_REV_0000_20250610
*
*   Copyright 2020 - 2025 NXP
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

/**
*   @file      Tspc_Port_Ip_Cfg.h
*
*   @addtogroup Port_CFG
*   @{
*/

#ifdef __cplusplus
extern "C"{
#endif


/*==================================================================================================
                                         INCLUDE FILES
 1) system and project includes
 2) needed interfaces from external units
 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Tspc_Port_Ip_Cfg.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define TSPC_PORT_IP_VENDOR_ID_CFG_C                       43
#define TSPC_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_C        4
#define TSPC_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_C        7
#define TSPC_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_C     0
#define TSPC_PORT_IP_SW_MAJOR_VERSION_CFG_C                6
#define TSPC_PORT_IP_SW_MINOR_VERSION_CFG_C                0
#define TSPC_PORT_IP_SW_PATCH_VERSION_CFG_C                0

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/
/* Check if Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h are of the same vendor */
#if (TSPC_PORT_IP_VENDOR_ID_CFG_C != TSPC_PORT_IP_VENDOR_ID_CFG_H)
    #error "Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h have different vendor ids"
#endif
/* Check if Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h are of the same Autosar version */
#if ((TSPC_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_C    != TSPC_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_H) || \
    (TSPC_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_C    != TSPC_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_H) || \
    (TSPC_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_C != TSPC_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_H) \
    )
    #error "AutoSar Version Numbers of Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h are different"
#endif
/* Check if Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h are of the same Software version */
#if ((TSPC_PORT_IP_SW_MAJOR_VERSION_CFG_C != TSPC_PORT_IP_SW_MAJOR_VERSION_CFG_H) || \
    (TSPC_PORT_IP_SW_MINOR_VERSION_CFG_C != TSPC_PORT_IP_SW_MINOR_VERSION_CFG_H) || \
    (TSPC_PORT_IP_SW_PATCH_VERSION_CFG_C != TSPC_PORT_IP_SW_PATCH_VERSION_CFG_H)    \
    )
    #error "Software Version Numbers of Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h are different"
#endif

/*==================================================================================================
                             LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                             LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                                            LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                           GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           GLOBAL VARIABLES
==================================================================================================*/

/* clang-format off */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
PortContainer_0_BOARD_InitPeripherals:
- options: {callFromInitBoot: 'true', coreID: M7_0_0}
- pin_list:
  - {pin_num: E7, peripheral: CAN6, signal: can6_tx, pin_signal: PTH0, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: E8, peripheral: CAN6, signal: can6_rx, pin_signal: PTH1, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: E10, peripheral: CAN7, signal: can7_tx, pin_signal: PTH2, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: E11, peripheral: CAN7, signal: can7_rx, pin_signal: PTH3, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: K13, peripheral: LPUART2, signal: lpuart2_rx, pin_signal: PTH7, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: L13, peripheral: LPUART2, signal: lpuart2_tx, pin_signal: PTH8, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: F6, peripheral: JTAG, signal: jtag_tms_swd_dio, pin_signal: PTA4, direction: INPUT/OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: A3, peripheral: SYSTEM, signal: reset_b, pin_signal: PTA5, direction: INPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: M16, peripheral: CAN0, signal: can0_tx, pin_signal: PTA7, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: M15, peripheral: CAN0, signal: can0_rx, pin_signal: PTA6, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: A2, peripheral: WKPU, signal: 'wkpu, 23', pin_signal: PTA8, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: G8, peripheral: JTAG_TRACENOETM, signal: jtag_tdo_tracenoetm_swo, pin_signal: PTA10, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: B1, peripheral: LPUART1, signal: lpuart1_tx, pin_signal: PTA18, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: C1, peripheral: LPUART1, signal: lpuart1_rx, pin_signal: PTA19, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: G1, peripheral: OSC32K, signal: osc32k_xtal, pin_signal: PTA24, pullSelect: pullUp, pullEnable: enabled}
  - {pin_num: H1, peripheral: OSC32K, signal: osc32k_extal, pin_signal: PTA25, pullSelect: pullUp, pullEnable: enabled}
  - {pin_num: N2, peripheral: GMAC1, signal: gmac1_mii_rmii_rgmii_mdc, pin_signal: PTA28, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: M3, peripheral: GMAC1, signal: gmac1_mii_rmii_rgmii_mdio, pin_signal: PTA29, direction: INPUT/OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: P16, peripheral: LPUART0, signal: lpuart0_rx, pin_signal: PTB0, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: R17, peripheral: LPUART0, signal: lpuart0_tx, pin_signal: PTB1, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: T11, peripheral: SIUL2, signal: 'gpio, 34', pin_signal: PTB2, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: U10, peripheral: GMAC0, signal: gmac0_mii_rgmii_txd3, pin_signal: PTB3, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: U3, peripheral: TRGMUX, signal: trgmux_in1, pin_signal: PTB4, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: C15, peripheral: LPUART9, signal: lpuart9_rx, pin_signal: PTB9, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: D15, peripheral: LPUART9, signal: lpuart9_tx, pin_signal: PTB10, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: J17, peripheral: LPUART7, signal: lpuart7_rx, pin_signal: PTB14, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: J16, peripheral: LPUART7, signal: lpuart7_tx, pin_signal: PTB15, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: K17, peripheral: LPUART4, signal: lpuart4_tx, pin_signal: PTB16, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: K16, peripheral: LPUART4, signal: lpuart4_rx, pin_signal: PTB17, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: M6, peripheral: GMAC0, signal: gmac0_mii_rgmii_rx_clk, pin_signal: PTB22, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: M7, peripheral: GMAC0, signal: gmac0_mii_rgmii_rxd2, pin_signal: PTB23, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: P7, peripheral: GMAC0, signal: gmac0_mii_rgmii_rxd3, pin_signal: PTB24, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: P8, peripheral: GMAC0, signal: gmac0_rgmii_txctl, pin_signal: PTB25, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: P10, peripheral: GMAC0, signal: gmac0_mii_rgmii_txd2, pin_signal: PTB28, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: U12, peripheral: GMAC0, signal: 'gmac0_mii_rmii_rgmii_txd, 0', pin_signal: PTB29, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: U7, peripheral: GMAC1, signal: gmac1_mii_rgmii_txd2, pin_signal: PTC0, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: T8, peripheral: GMAC1, signal: gmac1_mii_rmii_rgmii_tx_clk, pin_signal: PTC1, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: T4, peripheral: GMAC1, signal: gmac1_mii_rgmii_rxd3, pin_signal: PTC2, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: F7, peripheral: JTAG, signal: jtag_tck_swd_clk, pin_signal: PTC4, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: F8, peripheral: JTAG, signal: jtag_tdi, pin_signal: PTC5, pullEnable: enabled, InitValue: state_1}
  - {pin_num: N15, peripheral: CAN1, signal: can1_tx, pin_signal: PTC8, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: N14, peripheral: CAN1, signal: can1_rx, pin_signal: PTC9, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: T17, peripheral: CAN5, signal: can5_tx, pin_signal: PTC10, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: T15, peripheral: CAN5, signal: can5_rx, pin_signal: PTC11, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: T9, peripheral: GMAC0, signal: 'gmac0_mii_rmii_rgmii_rxd, 0', pin_signal: PTC14, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: U9, peripheral: GMAC0, signal: 'gmac0_mii_rmii_rgmii_rxd, 1', pin_signal: PTC15, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: P6, peripheral: GMAC0, signal: gmac0_mii_rmii_rx_dv_rgmii_rxctl, pin_signal: PTC16, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: R6, peripheral: GMAC1, signal: 'gmac1_mii_rmii_rgmii_txd, 1', pin_signal: PTC17, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: U11, peripheral: GMAC0, signal: 'gmac0_mii_rmii_rgmii_txd, 1', pin_signal: PTC18, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: T12, peripheral: GMAC0, signal: gmac0_mii_rmii_rgmii_tx_clk, pin_signal: PTC19, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: P17, peripheral: CAN3, signal: can3_tx, pin_signal: PTC28, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: N16, peripheral: CAN3, signal: can3_rx, pin_signal: PTC29, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: F3, peripheral: LPSPI3, signal: lpspi3_sout, pin_signal: PTD0, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: E3, peripheral: LPSPI3, signal: lpspi3_sck, pin_signal: PTD1, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: F16, peripheral: LPUART3, signal: lpuart3_tx, pin_signal: PTD2, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: F17, peripheral: LPUART3, signal: lpuart3_rx, pin_signal: PTD3, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: U5, peripheral: GMAC1, signal: gmac1_mii_rmii_rx_dv_rgmii_rxctl, pin_signal: PTD5, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: T5, peripheral: GMAC1, signal: 'gmac1_mii_rmii_rgmii_rxd, 0', pin_signal: PTD6, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: U4, peripheral: GMAC1, signal: gmac1_mii_rgmii_rxd2, pin_signal: PTD7, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: R8, peripheral: GMAC1, signal: 'gmac1_mii_rmii_rgmii_txd, 0', pin_signal: PTD8, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: U8, peripheral: GMAC1, signal: gmac1_mii_rgmii_txd3, pin_signal: PTD9, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: T6, peripheral: GMAC1, signal: gmac1_mii_rgmii_rx_clk, pin_signal: PTD10, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: U6, peripheral: GMAC1, signal: gmac1_rgmii_txctl, pin_signal: PTD11, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: R5, peripheral: GMAC1, signal: 'gmac1_mii_rmii_rgmii_rxd, 1', pin_signal: PTD12, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: P5, peripheral: LPI2C0, signal: lpi2c0_sda, pin_signal: PTD13, direction: INPUT/OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: M4, peripheral: LPI2C0, signal: lpi2c0_scl, pin_signal: PTD14, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: E17, peripheral: SIUL2, signal: 'gpio, 119', pin_signal: PTD23, direction: INPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: D16, peripheral: SIUL2, signal: 'gpio, 120', pin_signal: PTD24, direction: INPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: C16, peripheral: SIUL2, signal: 'gpio, 121', pin_signal: PTD25, direction: INPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: B5, peripheral: LPSPI0, signal: lpspi0_sin, pin_signal: PTE0, direction: INPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: A5, peripheral: LPSPI0, signal: lpspi0_sck, pin_signal: PTE1, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: B10, peripheral: LPSPI0, signal: lpspi0_sout, pin_signal: PTE2, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: N1, peripheral: LPUART5, signal: lpuart5_rx, pin_signal: PTE3, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: E2, peripheral: LPSPI3, signal: lpspi3_sin, pin_signal: PTE10, direction: INPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: L3, peripheral: LPUART5, signal: lpuart5_tx, pin_signal: PTE14, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: D2, peripheral: SIUL2, signal: 'gpio, 143', pin_signal: PTE15, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: C2, peripheral: SIUL2, signal: 'gpio, 144', pin_signal: PTE16, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: A8, peripheral: CAN2, signal: can2_tx, pin_signal: PTE24, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: B8, peripheral: CAN2, signal: can2_rx, pin_signal: PTE25, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: A4, peripheral: CAN4, signal: can4_tx, pin_signal: PTE30, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: B4, peripheral: CAN4, signal: can4_rx, pin_signal: PTE31, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: C6, peripheral: LPUART6, signal: lpuart6_tx, pin_signal: PTF2, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: D6, peripheral: LPUART6, signal: lpuart6_rx, pin_signal: PTF3, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: J15, peripheral: LPUART8, signal: lpuart8_tx, pin_signal: PTF20, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: K14, peripheral: LPUART8, signal: lpuart8_rx, pin_signal: PTF21, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: R12, peripheral: SIUL2, signal: 'gpio, 186', pin_signal: PTF26, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: P13, peripheral: SIUL2, signal: 'gpio, 187', pin_signal: PTF27, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: L10, peripheral: LPSPI0, signal: lpspi0_pcs0, pin_signal: PTF28, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: K4, peripheral: LPSPI3, signal: lpspi3_pcs1, pin_signal: PTF29, direction: OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: P2, peripheral: SIUL2, signal: 'gpio, 112', pin_signal: PTD16, direction: INPUT/OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
  - {pin_num: N3, peripheral: SIUL2, signal: 'gpio, 113', pin_signal: PTD17, direction: INPUT/OUTPUT, pullSelect: pullUp, pullEnable: enabled, InitValue: state_1}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* No registers that support TSPC were configured*/

/*==================================================================================================
                                      LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                                           LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
                                           GLOBAL FUNCTIONS
==================================================================================================*/


#ifdef __cplusplus
}
#endif

/** @} */
