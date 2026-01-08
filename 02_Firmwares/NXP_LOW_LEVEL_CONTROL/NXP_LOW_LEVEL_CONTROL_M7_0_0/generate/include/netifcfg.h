/* ###################################################################
**     Copyright 2020 NXP
**     All rights reserved.
**
**     Redistribution and use in source and binary forms, with or without modification,
**     are permitted provided that the following conditions are met:
*
**     1. Redistributions of source code must retain the above copyright notice,
**        this list of conditions and the following disclaimer.
**     2. Redistributions in binary form must reproduce the above copyright notice,
**        this list of conditions and the following disclaimer in the documentation
**        and/or other materials provided with the distribution.
**     3. The name of the author may not be used to endorse or promote products
**        derived from this software without specific prior written permission.
*
**     This software is owned or controlled by NXP and may only be
**     used strictly in accordance with the applicable license terms. By expressly
**     accepting such terms or by downloading, installing, activating and/or otherwise
**     using the software, you are agreeing that you have read, and that you agree to
**     comply with and are bound by, such license terms. If you do not agree to be
**     bound by the applicable license terms, then you may not retain, install,
**     activate or otherwise use the software. The production use license in
**     Section 2.3 is expressly granted for this software.
*
**     This file is part of the lwIP TCP/IP stack.
*
**     Author: Adam Dunkels <adam@sics.se>
*
** ###################################################################*/
/*
 * Function will find all component in appear in function group
 *
 * @param functionalGroups  functional group in UI
 * @param instanceName      Name of instance
 * @param componentName     Name of component
 * @return                  A array of components
 */


#ifndef LWIP_NETIFCFG_H
#define LWIP_NETIFCFG_H

#include "Gmac_Ip.h"
#include "lwip/netif.h"
#include "stdbool.h"

/* Structure containing network interfaces configuration. */
typedef struct {
    u8_t num;                               /* Hardware interface number */
    uint8_t hwaddr[NETIF_MAX_HWADDR_LEN];   /* MAC address */
    bool has_dhcp;                          /* Variable containing information whether dhcp is enabled or not */
    bool has_auto_ip;                       /* Variable containing information whether auto ip is enabled or not */
    bool has_IPv6;                          /* Variable containing information whether ipv6 is enabled or not */
    u8_t ip_addr[4];                        /* Ip address of board needs to be set if dhcp is turned off. */
    u8_t netmask[4];                        /* Netmask of board needs to be set if dhcp is turned off. */
    u8_t gw[4];                             /* Network gateway of board needs to be set if dhcp is turned off. */
    const char *hostname;                   /* Hostname of board used for dns. */
    char name[2];                           /* Interface name */
} netif_custom_t;

/* Number of active network interfaces. */
#define GMACIF_NUMBER      1

/* Number of Ethernet Interfaces for the stack */
#define ETHIF_NUMBER       GMACIF_NUMBER

/* Number of buffer descriptors for Rx ring */
#define GMAC_RXBD_NUM      16

/* Number of buffer descriptors for Tx ring */
#define GMAC_TXBD_NUM      8

/* Buffer length for Rx */
#define GMAC_MAX_RXBUFFLEN_SUPPORTED      1536

/* Buffer length for Tx */
#define GMAC_MAX_TXBUFFLEN_SUPPORTED      1536

/* Enable/Disable release of RX resource in TCPIP stack */
#define TCPIP_RELEASE_RX_RESOURCE    TRUE

/* Network interfaces configuration */
extern netif_custom_t *netif_cfg[ETHIF_NUMBER];


#endif /* LWIP_NETIFCFG_H */

