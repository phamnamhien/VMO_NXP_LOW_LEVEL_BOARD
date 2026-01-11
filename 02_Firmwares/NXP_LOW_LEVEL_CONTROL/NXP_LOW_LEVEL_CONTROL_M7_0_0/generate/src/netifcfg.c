/* ###################################################################
**     Copyright 2020-2025 NXP
**     Copyright (c) 2001-2003 Swedish Institute of Computer Science.
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


#include "BasicTypes.h"
#include "netifcfg.h"

#ifndef LWIP_NETIF_HOSTNAME_TEXT_0
#define LWIP_NETIF_HOSTNAME_TEXT_0          ("tcpipBoard0")
#endif /* LWIP_NETIF_HOSTNAME_TEXT_0 */

#ifndef IF_NAME_0
#define IF_NAME_0     'i', '1'
#endif /* IF_NAME_0 */

/* Each netif configuration */
netif_custom_t netifCfg_0 = {
    .num         = 0,
    .hwaddr      = { 0x10, 0x11, 0x22, 0x77, 0x77, 0x77 },
    .has_dhcp    = false,
    .has_auto_ip = false,
    .has_IPv6    = true,
    .ip_addr     = { 192,168,1,200 },
    .netmask     = { 255,255,255,0 },
    .gw          = { 192,168,1,1 },
    .hostname    = LWIP_NETIF_HOSTNAME_TEXT_0,
    .name        = { IF_NAME_0 }
};

/* Array of netif configurations */
netif_custom_t *netif_cfg[] = {
    &netifCfg_0
};


