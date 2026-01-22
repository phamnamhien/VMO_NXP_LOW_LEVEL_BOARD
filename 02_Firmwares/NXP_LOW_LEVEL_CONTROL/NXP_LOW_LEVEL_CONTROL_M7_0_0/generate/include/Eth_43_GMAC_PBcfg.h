/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           : GMAC
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 6.0.0
*   Build Version        : S32K3_RTD_6_0_0_D2506_ASR_REL_4_7_REV_0000_20250610
*
*   Copyright 2020 - 2025 NXP
*
*   NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

#ifndef ETH_43_GMAC_PBCFG_H
#define ETH_43_GMAC_PBCFG_H

/**
*   @file
*
*   @addtogroup ETH_43_GMAC_DRIVER_CONFIGURATION Ethernet Driver Configuration
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

#if defined(ETH_43_GMAC_MACSEC_SUPPORT)
#if (STD_ON == ETH_43_GMAC_MACSEC_SUPPORT)

#include "Eth_43_GMAC_MACsec_Types.h"
#endif /* (STD_ON == ETH_43_GMAC_MACSEC_SUPPORT) */
#endif /* defined(ETH_43_GMAC_MACSEC_SUPPORT) */
/*==================================================================================================
                                SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define ETH_43_GMAC_PBCFG_VENDOR_ID                     43
#define ETH_43_GMAC_PBCFG_AR_RELEASE_MAJOR_VERSION      4
#define ETH_43_GMAC_PBCFG_AR_RELEASE_MINOR_VERSION      7
#define ETH_43_GMAC_PBCFG_AR_RELEASE_REVISION_VERSION   0
#define ETH_43_GMAC_PBCFG_SW_MAJOR_VERSION              6
#define ETH_43_GMAC_PBCFG_SW_MINOR_VERSION              0
#define ETH_43_GMAC_PBCFG_SW_PATCH_VERSION              0

/*==================================================================================================
                                      FILE VERSION CHECKS
==================================================================================================*/
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    #if defined(ETH_43_GMAC_MACSEC_SUPPORT)
    #if (STD_ON == ETH_43_GMAC_MACSEC_SUPPORT)
        /* Checks against ETH_43_GMAC_MACsec_Types.h */
        #if ((ETH_43_GMAC_PBCFG_AR_RELEASE_MAJOR_VERSION != ETH_43_GMAC_MACSEC_TYPES_AR_RELEASE_MAJOR_VERSION) || \
             (ETH_43_GMAC_PBCFG_AR_RELEASE_MINOR_VERSION != ETH_43_GMAC_MACSEC_TYPES_AR_RELEASE_MINOR_VERSION)    \
            )
            #error "AUTOSAR Version Numbers of Eth_43_GMAC_Ipw_Cfg.h and Eth_43_GMAC_MACsec_Types.h are different"
        #endif
    #endif
    #endif
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
*                                      DEFINES AND MACROS
==================================================================================================*/

#if defined(ETH_43_GMAC_MACSEC_SUPPORT)
#if (STD_ON == ETH_43_GMAC_MACSEC_SUPPORT)

#endif /* (STD_ON == ETH_43_GMAC_MACSEC_SUPPORT) */
#endif /* defined(ETH_43_GMAC_MACSEC_SUPPORT) */




#if !defined(ETH_43_GMAC_RX_IRQ_ENABLED)
    #define ETH_43_GMAC_RX_IRQ_ENABLED      (STD_ON)
#endif /* !defined(ETH_43_GMAC_RX_IRQ_ENABLED) */

#if !defined(ETH_43_GMAC_TX_IRQ_ENABLED)
    #define ETH_43_GMAC_TX_IRQ_ENABLED        (STD_ON)
#endif /* !defined(ETH_43_GMAC_TX_IRQ_ENABLED) */

#if !defined(EthConf_EthCtrlConfig_EthCtrlConfig_0)
    /*! @brief Controller symbolic name to be passed to API functions that require CtrlIdx */
    #define EthConf_EthCtrlConfig_EthCtrlConfig_0    (0U)
#elif (EthConf_EthCtrlConfig_EthCtrlConfig_0 != 0)
    #error "[TPS_ECUC_06074] Invalid configuration due to symbolic name values"
#endif /* !defined(EthConf_EthCtrlConfig_EthCtrlConfig_0) */

#if !defined(EthConf_EthCtrlConfigIngressFifo_EthCtrlConfigIngressFifo_0)
    /*! @brief Controller symbolic name to be passed to API functions that require FifoIdx */
    #define EthConf_EthCtrlConfigIngressFifo_EthCtrlConfigIngressFifo_0        (0U)
#elif (EthConf_EthCtrlConfigIngressFifo_EthCtrlConfigIngressFifo_0 != 0)
    #error "[TPS_ECUC_06074] Invalid configuration due to symbolic name values"
#endif /* !defined(EthConf_EthCtrlConfigIngressFifo_EthCtrlConfigIngressFifo_0) */

/* Maximum number of configured Tx FIFOs */
#if !defined(ETH_43_GMAC_MAX_TXFIFO_SUPPORTED)
    /*! @brief Maximum number of configured Tx FIFOs */
    #define ETH_43_GMAC_MAX_TXFIFO_SUPPORTED        (1U)
#elif (ETH_43_GMAC_MAX_TXFIFO_SUPPORTED < 1)
    #undef ETH_43_GMAC_MAX_TXFIFO_SUPPORTED
    #define ETH_43_GMAC_MAX_TXFIFO_SUPPORTED        (1U)
#endif /* !defined(ETH_43_GMAC_MAX_TXFIFO_SUPPORTED) */

/* Maximum number of configured Rx FIFOs */
#if !defined(ETH_43_GMAC_MAX_RXFIFO_SUPPORTED)
    #define ETH_43_GMAC_MAX_RXFIFO_SUPPORTED        (1U)
#elif (ETH_43_GMAC_MAX_RXFIFO_SUPPORTED < 1)
    #undef ETH_43_GMAC_MAX_RXFIFO_SUPPORTED
    #define ETH_43_GMAC_MAX_RXFIFO_SUPPORTED        (1U)
#endif /* !defined(ETH_43_GMAC_MAX_RXFIFO_SUPPORTED) */

/* Maximum number of configured buffers per Tx FIFO */
#if !defined(ETH_43_GMAC_MAX_TXBUFF_SUPPORTED)
    #define ETH_43_GMAC_MAX_TXBUFF_SUPPORTED        (16U)
#elif (ETH_43_GMAC_MAX_TXBUFF_SUPPORTED < 16)
    #undef ETH_43_GMAC_MAX_TXBUFF_SUPPORTED
    #define ETH_43_GMAC_MAX_TXBUFF_SUPPORTED        (16U)
#endif /* !defined(ETH_43_GMAC_MAX_TXBUFF_SUPPORTED) */

/* Maximum number of configured buffers per Rx FIFO */
#if !defined(ETH_43_GMAC_MAX_RXBUFF_SUPPORTED)
    #define ETH_43_GMAC_MAX_RXBUFF_SUPPORTED        (32U)
#elif (ETH_43_GMAC_MAX_RXBUFF_SUPPORTED < 32)
    #undef ETH_43_GMAC_MAX_RXBUFF_SUPPORTED
    #define ETH_43_GMAC_MAX_RXBUFF_SUPPORTED        (32U)
#endif /* !defined(ETH_43_GMAC_MAX_RXBUFF_SUPPORTED) */

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

#endif /* ETH_43_GMAC_PBCFG_H */

