/* ###################################################################
**     Copyright 2020-2024 NXP
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


#ifndef LWIP_CFG_H
#define LWIP_CFG_H

/**
* @page misra_violations MISRA-C:2012 violations
*
* @section [global]
* Violates MISRA 2012 Advisory Rule 2.5, Global macro not referenced.
* The global macro will be used in function call of the module.
*/

#if defined(__NEWLIB__)
#include <sys/time.h>
#elif defined(__EWL__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#include <time.h>
#pragma GCC diagnostic pop
#endif /* defined(__NEWLIB__) */

#define LWIP_HTTPD_APP              0
#define LWIP_HTTPD_APP_NETCONN      0
#define LWIP_TCPECHO_APP            1
#define LWIP_TCPECHO_APP_NETCONN    0
#define LWIP_UDPECHO_APP            0
#define LWIP_UDPECHO_APP_NETCONN    0

#define LWIP_LWIPERF_APP            0
#define LWIP_NETBIOS_APP            1
#define LWIP_SNTP_APP               0
#define LWIP_NETIO_APP              0

#define LWIP_DNS_APP                0
#define LWIP_SHELL_APP              0

#define LWIP_CHARGEN_APP            0
#define LWIP_SOCKET_EXAMPLES_APP    0
#define LWIP_RTP_APP                0

#ifdef BYTE_ORDER
#undef BYTE_ORDER
#define BYTE_ORDER                  LITTLE_ENDIAN
#else
#define BYTE_ORDER                  LITTLE_ENDIAN
#endif /* BYTE_ORDER */

/****************************************************************************
 * ADDITIONAL USERS SETTINGS
 ****************************************************************************/


#endif /* LWIP_CFG_H */

