/*
 * Copyright 2019-2020 NXP
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

 /**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.1, function has no explicit type
 * The type is defined in some place.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.2, function has no explicit type or class, int assumed
 * This is a fake finding.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.3, All declarations or an object or function shall use the
 * same names and type qualifiers.
 * Argument of function has the same name and type qualifier in both files. This is a fake finding.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Directive 4.9, A function should be used in preference
 * to a function-like macro where they are interchangeable.
 * Function-like macros are used instead of inline functions in order to ensure
 * that the performance will not be decreased if the functions will not be
 * inlined by the compiler.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.1, Unpermitted operand to operator '&&'
 * Variable is of essential boolean type
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.7, A tag name shall be a unique identifier.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.4, A compatible declaration shall be
 * visible when an object or function with external linkage is defined.
 * These are symbols weak symbols defined in platform startup files (.s).
 *
 */
#ifndef ETHIF_PORT_H
#define ETHIF_PORT_H

#include "lwip/err.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"

#include "OsIf.h"
#include "OsIf_rtd_port.h"

#include "netifcfg.h"
#include "ethif_port_ipw.h"

/*! @brief Data alignment. */
#if defined ( __GNUC__ ) || defined ( __ghs__ ) || defined (__ARMCC_VERSION)
    #define ALIGNED(x)      __attribute__((aligned(x)))
#elif defined ( __ICCARM__ )
    #define stringify(s) tostring(s)
    #define tostring(s) #s
    #define ALIGNED(x)      _Pragma(stringify(data_alignment=x))
#else
    /* Keep compatibility with software analysis tools */
    #define ALIGNED(x)
#endif

typedef unsigned int (*rx_buff_process_condition_handler_t)(uint8_t eth_instance, void *buff);

#if !NO_SYS
extern sys_mutex_t ethif_tx_lock;
#endif /* !NO_SYS */

err_t ethif_ethernetif_init(struct netif *netif);
void ethif_ethernetif_shutdown(struct netif *netif);

#if LWIP_IPV6
err_t mld_eth_filter (struct netif *netif, const ip6_addr_t *group, enum netif_mac_filter_action action);
#endif /*LWIP_IPV6*/

#if LWIP_IGMP
err_t igmp_eth_filter (struct netif *netif, const ip4_addr_t *group, enum netif_mac_filter_action action);
#endif /*LWIP_IGMP*/

#if NO_SYS
err_t ethif_poll_interface(struct netif *netif);
#endif /* NO_SYS */

#if !NO_SYS
/*!
 * @brief Send one byte to transfer buffer
 *
 * This function sends one byte to transfer buffer
 */
void send_tx_pbuffs_dummy_char(void);
/*!
 * @brief Send one byte to receive buffer
 *
 * This function sends one byte to receive buffer
 */
void send_rx_pbuffs_dummy_char(void);
#endif /* !NO_SYS */

void ethif_register_rx_buff_process_condition_handler(rx_buff_process_condition_handler_t handler);

#endif /* ETHIF_PORT_H */
