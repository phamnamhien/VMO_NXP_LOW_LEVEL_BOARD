/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading, installing, activating and/or otherwise
 * using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software. The production use license in
 * Section 2.3 is expressly granted for this software.
 *
 * This file is derived from the Ethernet Interface Skeleton in lwIP with the following copyright:
 *
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 */

#ifndef ETHIF_PORT_IPW_H
#define ETHIF_PORT_IPW_H

/**
*   @file
*
*   @addtogroup ETHIF Ethernet Interface
*   @{
*/

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Eth_43_GMAC.h"

#include "Gmac_Ip.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
*                                          CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/
/* Define the Ethernet RTD functions again*/
#define  Eth_Init                        Eth_43_GMAC_Init
#define  Eth_Receive                     Eth_43_GMAC_Receive
#define  Eth_Transmit                    Eth_43_GMAC_Transmit
#define  Eth_SetControllerMode           Eth_43_GMAC_SetControllerMode
#define  Eth_TxConfirmation              Eth_43_GMAC_TxConfirmation
#define  Eth_SendFrame                   Eth_43_GMAC_SendFrame
#define  Eth_SendMultiBufferFrame        Eth_43_GMAC_SendMultiBufferFrame
#define  Eth_ProvideTxBuffer             Eth_43_GMAC_ProvideTxBuffer
#define  Eth_ProvideRxBuffer             Eth_43_GMAC_ProvideRxBuffer
#define  Eth_UpdatePhysAddrFilter        Eth_43_GMAC_UpdatePhysAddrFilter
#define  Eth_MultiBufferFrameType        Eth_43_GMAC_MultiBufferFrameType
#define  Eth_ReportTransmission          Eth_43_GMAC_ReportTransmission

/* Defines used for Ethernet. */
#define ETH_HAS_EXTERNAL_TX_BUFFERS      ETH_43_GMAC_HAS_EXTERNAL_TX_BUFFERS
#define ETH_HAS_EXTERNAL_RX_BUFFERS      ETH_43_GMAC_HAS_EXTERNAL_RX_BUFFERS
#define ETH_UPDATE_PHYS_ADDR_FILTER_API  ETH_43_GMAC_UPDATE_PHYS_ADDR_FILTER_API
#define ETH_HAS_SEND_MULTI_BUFFER_FRAME  ETH_43_GMAC_SEND_MULTI_BUFFER_FRAME_API
#define ETH_RXBD_NUM                     ETH_43_ETH_RXBD_NUM
#define ETH_TXBD_NUM                     ETH_43_ETH_TXBD_NUM
#define ETH_INSTANCE_COUNT               (1u)
#define ETH_FRAME_MAX_FRAMELEN          (1520U)
#define ETH_INSTANCE                     ETH_43_GMAC_DRIVER_INSTANCE
#define ETH_QUEUE                        0U
#define ETH_BUFF_ALIGNMENT               64U
#define ETH_BUFF_ALIGN(x)                (((uint32_t)(x) + (ETH_BUFF_ALIGNMENT - 1UL)) & ~(ETH_BUFF_ALIGNMENT - 1UL))
#define ETH_RXBUFF_SIZE                  ETH_BUFF_ALIGN(ETH_FRAME_MAX_FRAMELEN)
#define ETH_TX_RETRY_COUNT               100000U

/* Code returned by the pre-input handler in the case when the frame should be forwarded to the stack */
#define                                   FORWARD_FRAME   (0U)


/* Also defined on driver Eth.c file */
#define ETHIF_FRAME_MACDST_OFFSET     (0U)
#define ETHIF_FRAME_MACSRC_OFFSET     (6U)
#define ETHIF_FRAME_ETHTYPE_OFFSET    (12U)
#define ETHIF_FRAME_PAYLOAD_OFFSET    (14U)

#define ETHIF_FRAME_MACDST_LENGTH     (6U)
#define ETHIF_FRAME_MACSRC_LENGTH     (6U)
#define ETHIF_FRAME_ETHTYPE_LENGTH    (2U)
#define ETHIF_FRAME_HEADER_LENGTH     (ETHIF_FRAME_MACDST_LENGTH + ETHIF_FRAME_MACSRC_LENGTH + ETHIF_FRAME_ETHTYPE_LENGTH)

/* API's exposed to the Stack */
#define ETHIF_INIT ethif_ethernetif_init
#define ETHIF_SHUTDOWN ethif_ethernetif_shutdown
#define ETHIF_BUFFER_t Gmac_Ip_BufferType
#define ETHIF_POLL_INTERFACE ethif_poll_interface
#define ETHIF_REGISTER_RX_BUFF_PROCESS_CONDITION_HANDLER \
    ethif_register_rx_buff_process_condition_handler
/*==================================================================================================
*                                            ENUMS
==================================================================================================*/

/*==================================================================================================
*                                STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
*                                GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/


/*==================================================================================================
*                                    FUNCTION PROTOTYPES
==================================================================================================*/

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* ETHIF_PORT_IPW_H */          
