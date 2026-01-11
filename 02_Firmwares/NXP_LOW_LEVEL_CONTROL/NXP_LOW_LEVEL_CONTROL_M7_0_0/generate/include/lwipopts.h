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


#ifndef LWIP_OPTS_H
#define LWIP_OPTS_H

#include "stdint.h"

/**
* @page misra_violations MISRA-C:2012 violations
*
* @section [global]
* Violates MISRA 2012 Advisory Rule 2.5, Global macro not referenced.
* The global macro will be used in function call of the module.
*/

/* ---------- Application APIs ---------- */
#define NO_SYS                     0
#define LWIP_RAW                   1
#define LWIP_SOCKET                (NO_SYS==0)
#define LWIP_NETCONN               (NO_SYS==0)

/* ---------- ARP options ---------- */
#define LWIP_ARP                   1
#define ARP_TABLE_SIZE             10
#define ARP_QUEUEING               1

/* ---------- IP options ---------- */
#define LWIP_IPV6                  1

/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of interfaces */
#define LWIP_DHCP                  0
/* 1 if you want to do an ARP check on the offered address (recommended) */
#define DHCP_DOES_ARP_CHECK        (LWIP_DHCP)

/* ---------- AUTOIP options ---------- */
#define LWIP_AUTOIP                    0
#define LWIP_DHCP_AUTOIP_COOP          (LWIP_DHCP && LWIP_AUTOIP)
#define LWIP_DHCP_AUTOIP_COOP_TRIES    3

/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD                     0

/* IP reassembly and segmentation.These are orthogonal even
* if they both deal with IP fragments */
#define IP_REASSEMBLY                  1
#define IP_REASS_MAX_PBUFS             10
#define MEMP_NUM_REASSDATA             10
#define IP_FRAG                        1

/* ---------- IP options ---------- */
#define LWIP_IPV4                      1
#define ICMP_TTL                       255
#define LWIP_ICMP                      1
#define LWIP_IGMP                      1

/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         0

/* TCP Maximum segment size. */
#define TCP_MSS                 1460

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             7300

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN       (4 * TCP_SND_BUF/TCP_MSS)

/* TCP writable space (bytes). This must be less than or equal
   to TCP_SND_BUF. It is the amount of space which must be
   available in the tcp snd_buf for select to return writable */
#define TCP_SNDLOWAT           (TCP_SND_BUF/2)

/* TCP receive window. */
#define TCP_WND                 7300

/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX              2

/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX           2

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define LWIP_UDPLITE            LWIP_UDP
#define UDP_TTL                 255

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
/* MSVC port: intel processors do not need 4-byte alignment,
   but are faster that way! */

/*!
 * @brief Custom memcpy function used to optimize and improve the lwip copy speed process.
 *
 * This function copies len characters from memory area src to memory area dest using 64 bit alignment.
 */
void memcpy_64(uint64_t *dst, uint64_t * src, unsigned int len);

/*!
 * @brief Custom memcpy function used to optimize and improve the lwip copy speed process.
 *
 * This function copies len characters from memory area src to memory area dest.
 */
void memcpy_custom(void *dst, const void * src, unsigned int len);

#define MEMCPY(dst,src,len)         memcpy(dst,src,len)

#define MEM_ALIGNMENT               16

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                    65535

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF               8

/* MEMP_NUM_RAW_PCB: the number of UDP protocol control blocks. One
   per active RAW connection. */
#define MEMP_NUM_RAW_PCB            4

/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP connection. */
#define MEMP_NUM_UDP_PCB            8

/* MEMP_NUM_TCP_PCB: the number of simultaneously active TCP connections. */
#define MEMP_NUM_TCP_PCB            8

/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP connections. */
#define MEMP_NUM_TCP_PCB_LISTEN     8

/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments.*/
#define MEMP_NUM_TCP_SEG            32

/* MEMP_NUM_SYS_TIMEOUT: the number of simultaneously active timeouts. */
#define MEMP_NUM_SYS_TIMEOUT        15

/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF             20

/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN            32

/* MEMP_NUM_TCPIP_MSG_*: the number of struct tcpip_msg, which is used
   for sequential API communication and incoming packets. Used in
   src/api/tcpip.c. */
#define MEMP_NUM_TCPIP_MSG_API      20
#define MEMP_NUM_TCPIP_MSG_INPKT    20

/* MEM_USE_POOLS==1: Use an alternative to malloc()
   To use this, MEMP_USE_CUSTOM_POOLS also has to be enabled. */
#define MEM_USE_POOLS               0
#define MEMP_USE_CUSTOM_POOLS       0

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE              0

/** SYS_LIGHTWEIGHT_PROT
* define SYS_LIGHTWEIGHT_PROT in lwipopts.h if you want inter-task protection
* for certain critical regions during buffer allocation, deallocation and memory
* allocation and deallocation. */
#define SYS_LIGHTWEIGHT_PROT        1

/**
 * Set this to 1 if you want to free PBUF_RAM pbufs (or call mem_free()) from
 * interrupt context (or another context that doesn't allow waiting for a
 * semaphore).
 * If set to 1, mem_malloc will be protected by a semaphore and SYS_ARCH_PROTECT,
 * while mem_free will only use SYS_ARCH_PROTECT. mem_malloc SYS_ARCH_UNPROTECTs
 * with each loop so that mem_free can run.
 *
 * ATTENTION: As you can see from the above description, this leads to dis-/enabling
 * interrupts often, which can be slow! Also, on low memory, mem_malloc
 * can need longer.
 *
 * If you don't want that, at least for NO_SYS=0, you can still use the following
 * functions to enqueue a deallocation call which then runs in the tcpip_thread
 * context:
 * - pbuf_free_callback(p);
 * - mem_free_callback(m); */
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 0

#define NETIF_CHECKSUM_SETTING      NETIF_CHECKSUM_DISABLE_ALL
/* ---------- Statistics options ---------- */
#define LWIP_STATS                      1
#define LWIP_STATS_DISPLAY              0

#if LWIP_STATS
#define LINK_STATS                      1
#define IP_STATS                        1
#define ICMP_STATS                      1
#define IGMP_STATS                      1
#define IPFRAG_STATS                    1
#define UDP_STATS                       1
#define TCP_STATS                       1
#define MEM_STATS                       1
#define MEMP_STATS                      1
#define PBUF_STATS                      1
#define SYS_STATS                       1
#endif /* LWIP_STATS */

/* ---------- PPP options ---------- */
#define PPP_SUPPORT                     0

/* ---------- Other options ---------- */

/* (LWIP_ETHERNET == 1) Enable ETHERNET support even though ARP might be disabled */
#define LWIP_ETHERNET                   1

#define TCPIP_MBOX_SIZE                 40

#define DEFAULT_UDP_RECVMBOX_SIZE       20
#define DEFAULT_TCP_RECVMBOX_SIZE       20
#define DEFAULT_RAW_RECVMBOX_SIZE       10
#define DEFAULT_ACCEPTMBOX_SIZE         10
#define LWIP_NETIF_TX_SINGLE_PBUF       1	

#define LWIP_SUPPORT_CUSTOM_PBUF        1

#define LWIP_SNMP                       LWIP_UDP
#define MIB2_STATS                      LWIP_SNMP

#define LWIP_DNS                        LWIP_UDP
#define LWIP_MDNS_RESPONDER             LWIP_UDP

#define LWIP_NUM_NETIF_CLIENT_DATA      (LWIP_MDNS_RESPONDER)

#define LWIP_HAVE_LOOPIF                0
#define LWIP_NETIF_LOOPBACK             0

#define TCP_LISTEN_BACKLOG              1

#define LWIP_COMPAT_SOCKETS             1
#define LWIP_SO_RCVTIMEO                1
#define LWIP_SO_RCVBUF                  1

#define LWIP_TCPIP_CORE_LOCKING         1

#define LWIP_NETIF_LINK_CALLBACK        0
#define LWIP_NETIF_STATUS_CALLBACK      0

#define LWIP_NETCONN_SEM_PER_THREAD     0

#define LWIP_SOCKET_SET_ERRNO           1

#define LWIP_NETIF_HOSTNAME             1

/* ---------- TIMEVAL options ---------- */
#define LWIP_TIMEVAL_PRIVATE            0

/* ---------- Thread options ---------- */
#define TCPIP_THREAD_NAME               "tcpip_thread"
#define TCPIP_THREAD_STACKSIZE          8192
#define TCPIP_THREAD_PRIO               1
#define DEFAULT_THREAD_STACKSIZE        8192
#define DEFAULT_THREAD_PRIO             1

/* ---------- Pbuf data ---------- */
#define LWIP_PBUF_CUSTOM_DATA 			uint8_t* rx_buf;

#endif /* LWIP_OPTS_H */
