      /*
 * Copyright 2019-2022 NXP
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
 * Violates MISRA 2012 Required Rule 1.3,  Taking address of near auto variable.
 * The code is not dynamically linked. An absolute stack address is obtained
 * when taking the address of the near auto variable.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 2.1, A project shall not contain unreachable code.
 * These are safety checks to avoid dereferencing NULL pointers.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.4, A compatible declaration shall be
 * visible when an object or function with external linkage is defined.
 * These are symbols weak symbols defined in platform startup files (.s).
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, Could define variable at block scope
 * The variable is used in pal c file, so it must remain global.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.13, Pointer parameter could be declared as pointing to const
 * Type definition is done in another file.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.1, Unpermitted operand to operator '&&'
 * Variable is of essential boolean type
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.3, The value of an expression shall not be assigned to an
 * object with a narrower essential type or a different essential type category.
 * This is a string that will be concatenated to a macro variable to define a new one.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.4, Both operands of an operator in which the usual arithmetic
 * conversions are performed shall have the same essential type category.
 * These are bitwise operations used to enable flags or check their state.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.8, Impermissible cast of composite expression
 * Required in comparisons between constants and numerical types.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.3, Cast performed between a pointer to object type
 * and a pointer to a different object type.
 * This is used to check if transmission is complete.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.2, Conversion between a pointer to incomplete type and another type
 * The is a fake finding
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and
 * integer type.
 * The cast is required to initialize a pointer with an unsigned long define,
 * representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.5, Conversion from pointer to void to pointer to other type.
 * The conversion is needed to allocate or free the memory.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, A cast shall not be performed between
 * pointer to void and an arithmetic type.
 * The cast is required to comply with the lwip API that mandates passing arguments
 * to threads using a pointer to void type.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 13.5, The right hand operand of a logical && or || operator shall
 * not contain persistent side effects.
 * This is required in order to reduce code complexity.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 14.3, Controlling expressions shall not be invariant
 * This behaviour is intentional, some functions must always be called in current implementation.
 * Condition is still checked in event of further modifications for other applications.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 14.4, Conditional expression should have essentially Boolean type.
 * This is required for macro constructs in form do {...} while(0).
 *
 * @section [global]
 * Violates MISRA 2012 Mandatory Rule 17.3, Symbol undeclared, assumed
 * to return int.
 * The symbol is defined in another file.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 18.2, Substract operator applied to pointers.
 * Operation is required to compute the aligned address of the memory zone that further
 * needs to be freed.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 18.3, Relational or substract operator applied
 * to pointers.
 * Substraction is needed to compute a memory address.
 *
 * @section [global]
 * Violates MISRA 2012 Mandatory Rule 17.4, All exit paths from a function with non-void
 * return type shall have an explicit return statement with an expression.
 * Return value is available but cannot be reached due to multi threads.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.3, An identifier declared in an inner scope
 * shall not hide an identifier declared in an outer scope.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.6, A typedef name shall be a unique identifier.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.7, A tag name shall be a unique identifier.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 5.8, Identifiers that define objects or functions
 * with external linkage shall be unique.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 5.9, Identifiers that define objects or functions
 * with internal linkage should be unique.
 * Symbol redeclaration is declared at different local functions.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 15.5, A function should have a single point of exit
 * at the end.
 * This is acceptable because of multi threading application.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 1.2, Language extensions should not be used.
 * This is required for debug symbol assert.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.1, The program shall contain no violations of the
 * standard C syntax and constraints, and shall not exceed the implementation's translation limits.
 * This is required for debug symbol assert.
 *
 */

#include <string.h>
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "lwip/tcpip.h"

#include "lwipcfg.h"
#include "lwip/sys.h"

#include "ethif_port.h"

#include "netifcfg.h"

#include "PlatformTypes.h"

#if defined D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON)
#include "Cache_Ip.h"
#endif /* D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) */


#if defined(USING_OS_FREERTOS)
#include "FreeRTOS.h"
#endif /* defined(USING_OS_FREERTOS) */

#define IFNAME0 'e'
#define IFNAME1 'n'

#define ETHIF_SINGLE_PBUF 1
#define ETHIF_CHAINED_PBUF 0

struct netif * g_netif[ETH_INSTANCE_COUNT] = { NULL };

#if !NO_SYS
struct pbuf dummy_char2;
#endif /* !NO_SYS */

#if !NO_SYS
/* Lock to synchronize access on TX side, since the frames are sent from different threads */
sys_mutex_t ethif_tx_lock;
#endif /* !NO_SYS */

/* This handler is called before a frame is dispatched from the ETH driver to the TCPIP stack.
   If extra processing is needed before the dispatch is done, one must implement this handler and
   register it via ethif_register_rx_buff_process_condition_handler.
*/
static rx_buff_process_condition_handler_t rx_buff_process_handler = NULL;

__attribute__ ((section (".int_sram_no_cacheable")))
VAR_ALIGN(uint8 ethif_DataBuffer[ETH_RXBD_NUM * ETH_43_ETH_MAX_RXBUFFLEN_SUPPORTED], 64)

struct pbuf * volatile tx_pbufs[ETH_TXBD_NUM];

uint8 *rx_bufs[ETH_RXBD_NUM];

#if !NO_SYS

/* In order to support zero-copy operation, on the RX side we are using custom pbufs, with the payload pointing to the
   receive buffer obtained from the driver. When the pbuf is eventually freed, the receive buffer is given back to the driver.
   On the TX side we are incrementing the reference count on the pbuf and giving its payload storage to the driver. Once we
   detect the transmission is complete, we are freeing our reference to the pbuf. */

/* Memory pool for RX custom pbufs
   The pool only holds the pbuf_custom structures, not the storage for actual payload */
LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RXBD_NUM, sizeof(struct pbuf_custom), "Zero-copy RX PBUF pool")

/* Queue for holding pbufs which have been sent to the driver for transmission. They will be released once transmission is complete
  (detected by polling Netc_Eth_Ip_GetTransmitStatus) */
static sys_mbox_t in_flight_tx_pbufs;

static sys_thread_t poll_thread;

/**
 * Callback function called when a custom pbuf is freed
 *
 * @param p - the custom pbuf structure
 * Implements ethif_pbuf_free_custom_Activity
 */
static void ethif_pbuf_free_custom(struct pbuf *p)
{
    LWIP_ASSERT("NULL pointer", p != NULL);
    struct pbuf_custom* pc = (struct pbuf_custom*)p;
    pc->pbuf.if_idx -= 1;
#if (ETH_HAS_EXTERNAL_RX_BUFFERS == STD_ON)
    #if (TCPIP_RELEASE_RX_RESOURCE == TRUE)
        sys_arch_protect();
        Eth_ProvideRxBuffer(netif_cfg[pc->pbuf.if_idx]->num, 0, pc->pbuf.rx_buf);
        sys_arch_unprotect(0);
    #endif
#endif
    LWIP_MEMPOOL_FREE(RX_POOL, pc);
}

/**
 * Transmit a packet.
 * The packet is contained in the pbuf that is passed to the function. This pbuf might be chained.
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * @param p - the pbuf structure
 * Implements ethif_low_level_output_Activity
 */
static err_t ethif_low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;;
    uint8_t pbuf_chain_type = ETHIF_SINGLE_PBUF;
    Eth_BufIdxType bufferIndex;
    BufReq_ReturnType status  = BUFREQ_E_NOT_OK;
    err_t pbuf_status = ERR_BUF;
    LWIP_ASSERT("Output packet buffer empty", p);
#if defined(LWIP_DEBUG) && LWIP_NETIF_TX_SINGLE_PBUF && !(LWIP_IPV4 && IP_FRAG) && (LWIP_IPV6 && LWIP_IPV6_FRAG)
    LWIP_ASSERT("p->next == NULL && p->len == p->tot_len", p->next == NULL && p->len == p->tot_len);
#endif /* LWIP_DEBUG && LWIP_NETIF_TX_SINGLE_PBUF && !(LWIP_IPV4 && IP_FRAG) && (LWIP_IPV6 && LWIP_IPV6_FRAG */

#if (ETH_HAS_SEND_MULTI_BUFFER_FRAME == STD_ON)
    uint8_t bufs_num;
    Eth_MultiBufferFrameType multiFrame;
    Eth_BufIdxType bufIdx;
    uint32 i = 0u;

    /* Increment our reference on p */
    pbuf_ref(p);

    bufs_num = pbuf_clen(p);
    LWIP_ASSERT("number of buffers to send are to big", bufs_num <= 16);
    multiFrame.NumBuffers = bufs_num;

    q = p;
    /* parse pbufs and fill bd data and length*/
    i = 0;
    do{
        multiFrame.BufferData[i] = q->payload;
        multiFrame.BufferLength[i] = q->len;
#if defined D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined CPU_CORTEX_M7
        DataCacheCleanbyAddr((uint32)q->payload, q->tot_len);
#endif /* D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined CPU_CORTEX_M7 */
        i++;
    } while((q = q->next) != NULL);

    while (ERR_BUF == pbuf_status)
    {
        for(i=0; i < ETH_TXBD_NUM; i++)
        {
            sys_arch_protect();
            if (tx_pbufs[i] == NULL)
            {
                status = Eth_SendMultiBufferFrame(netif_cfg[netif->num]->num, ETH_QUEUE, multiFrame, &bufIdx, TRUE);
                if (BUFREQ_OK == status)
                {
                    tx_pbufs[i]=p;
                    pbuf_status=ERR_OK;
                }

            }
            sys_arch_unprotect(0);
            if (ERR_OK == pbuf_status)
            {
                break;
            }
        }
    }

    if (BUFREQ_OK != status)
    {
        /* Decrement the ref (either p's ref in case it was a single pbuf, or the coalesed q's ref) */
        (void)pbuf_free(p);
        tx_pbufs[i]=NULL;
    }
#else /* ETH_HAS_SEND_MULTI_BUFFER_FRAME */

    /* Check whether this was single or a chained pbuf */
    if (NULL != p->next)
    {
       /* This is a chained pbuf, save info into a local variable, as p will be lost if allocation does not fail */
       pbuf_chain_type = ETHIF_CHAINED_PBUF;
    }

    /* Increment our reference on p */
    pbuf_ref(p);

    /* If p was a pbuf chain instead, p's ref was decreased and we got another q pbuf with ref 1
    Either way, q has a +1 ref that we need to free in case we're not keeping the buffer - ie in case of errors*/

    q = pbuf_coalesce(p, PBUF_RAW);

    /* If this was a chained pbuf, check allocation */
    if ((ETHIF_CHAINED_PBUF == pbuf_chain_type) && (q == p))
    {
        /* Memory allocation failed */
        pbuf_status = ERR_MEM;
    }

    else
    {
        /* Keep trying to send the frame as long as the driver says there is not enough space in the queue */

        do
        {
            status = Eth_SendFrame(netif_cfg[netif->num]->num, 0, q->payload, &q->tot_len, &bufferIndex, TRUE);
        }
        while (BUFREQ_OK != status);

        /* Decrement the ref (either p's ref in case it was a single pbuf, or the coalesed q's ref) */
        (void)pbuf_free(q);

        pbuf_status = ERR_OK;
    }

#endif /* (ETH_43_NETC_SEND_MULTI_BUFFER_FRAME_API == STD_ON) */




    return pbuf_status;
}

/**
 * This function is called when a packet is ready to be read from the interface.
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * @param data - the pointer to the received data buffer
 * @param size - the length of received data buffer
 * @return ERR_OK if the packet is being handled (we take ownership of the data buffer)
 *         ERR_MEM if the packet cannot be handled (we don't take ownership of the data buffer,
 *         therefore the caller should release it)
 * Implements ethif_input_Activity
 */
static err_t ethif_input(struct netif *netif, uint8_t * data, uint16_t size)
{
    err_t ret = ERR_MEM;

    /* Allocate a custom PBUF_REF pointing to the receive buffer */
    struct pbuf_custom* ethif_pbuf  = (struct pbuf_custom*)LWIP_MEMPOOL_ALLOC(RX_POOL);
    if (NULL != ethif_pbuf)
    {

        ethif_pbuf->custom_free_function = ethif_pbuf_free_custom;
        struct pbuf* p = pbuf_alloced_custom(PBUF_RAW, size, PBUF_REF, ethif_pbuf, data, size);

        p->if_idx = netif_get_index(netif);

        /* Saving receive buffer for further calling on provide Rx buffer */
        p->rx_buf = data;

        ret  = netif->input(p, netif);
        if (ERR_OK != ret)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethif_input: IP input error\n"));
            (void)pbuf_free(p);
        }
    }
    return ret;
}

#else /* !NO_SYS */

/**
 * Transmit a packet.
 * The packet is contained in the pbuf that is passed to the function. This pbuf might be chained.
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * @param p - the pbuf structure
 * Implements ethif_low_level_output_Activity
 */
static err_t ethif_low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;
    ETHIF_BUFFER_t bd;
    Std_ReturnType status;
    err_t pbuf_status = ERR_BUF;
    uint8_t pbuf_chain_type = ETHIF_SINGLE_PBUF;
    Eth_BufIdxType bufIdx;

    LWIP_ASSERT("Output packet buffer empty", p);
#if defined(LWIP_DEBUG) && LWIP_NETIF_TX_SINGLE_PBUF && !(LWIP_IPV4 && IP_FRAG) && (LWIP_IPV6 && LWIP_IPV6_FRAG)
    LWIP_ASSERT("p->next == NULL && p->len == p->tot_len", p->next == NULL && p->len == p->tot_len);
#endif /* LWIP_DEBUG && LWIP_NETIF_TX_SINGLE_PBUF && !(LWIP_IPV4 && IP_FRAG) && (LWIP_IPV6 && LWIP_IPV6_FRAG */

    /* Increment our reference on p */
    pbuf_ref(p);

#if (ETH_HAS_SEND_MULTI_BUFFER_FRAME == STD_ON)
    uint8_t bufs_num;
    uint8_t i;
    Eth_MultiBufferFrameType multiFrame;
    bufs_num = pbuf_clen(p);
    LWIP_ASSERT("number of buffers to send are to big", bufs_num <= 16);
    multiFrame.NumBuffers = bufs_num;

    q = p;
    /* parse pbufs and fill bd data and length*/
    i = 0;
    do{
        multiFrame.BufferData[i] = q->payload;
        multiFrame.BufferLength[i] = q->len;
#if defined D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined CPU_CORTEX_M7
        DataCacheCleanbyAddr((uint32)q->payload, q->tot_len);
#endif /* D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined CPU_CORTEX_M7 */
        i++;
    } while((q = q->next) != NULL);

    while (ERR_BUF == pbuf_status)
    {
        for(i=0; i < ETH_TXBD_NUM; i++)
        {
            if (tx_pbufs[i] == NULL)
            {
                OsIf_SuspendAllInterrupts();
                status = Eth_SendMultiBufferFrame(netif_cfg[netif->num]->num, ETH_QUEUE, multiFrame, &bufIdx, TRUE);
                if (BUFREQ_OK == status)
                {
                    tx_pbufs[i]=p;
                    pbuf_status=ERR_OK;
                }
                OsIf_ResumeAllInterrupts();
                if (ERR_OK == pbuf_status)
                {
                    break;
                }
            }
        }
    }

    if (BUFREQ_OK != status)
    {
        /* Decrement the ref (either p's ref in case it was a single pbuf, or the coalesed q's ref) */
        (void)pbuf_free(p);
        tx_pbufs[i]=NULL;
    }
#else /* ETH_HAS_SEND_MULTI_BUFFER_FRAME */

    /* Check whether this was single or a chained pbuf */
    if (NULL != p->next)
    {
       /* This is a chained pbuf, save info into a local variable, as p will be lost if allocation does not fail */
       pbuf_chain_type = ETHIF_CHAINED_PBUF;
    }

    /* If p was a pbuf chain instead, p's ref was decreased and we got another q pbuf with ref 1
    Either way, q has a +1 ref that we need to free in case we're not keeping the buffer - ie in case of errors*/
    q = pbuf_coalesce(p, PBUF_RAW);

    /* If this was a chained pbuf, check allocation */
    if ((ETHIF_CHAINED_PBUF == pbuf_chain_type) && (q == p))
    {
        /* Memory allocation failed */
        pbuf_status = ERR_MEM;
    }
    else
    {
        bd.Data = q->payload;
        bd.Length = q->tot_len;

#if defined D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined CPU_CORTEX_M7
        DataCacheCleanbyAddr((uint32)bd.Data, bd.Length);
#endif /* D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined CPU_CORTEX_M7 */

        /* Keep trying to send the frame as long as the driver says there is not enough space in the queue */
        do
        {
            status = Eth_SendFrame(netif_cfg[netif->num]->num, ETH_QUEUE, &bd.Data[0], &bd.Length, &bufIdx, TRUE);

        } while (BUFREQ_E_BUSY == status);
    }

    if (BUFREQ_OK == status)
    {

    }

    /* Decrement the ref (either p's ref in case it was a single pbuf, or the coalesce q's ref) */
    (void)pbuf_free(q);

    pbuf_status = ERR_OK;

#endif /* ETH_HAS_SEND_MULTI_BUFFER_FRAME */

    return pbuf_status;
}

/**
 * This function is called when a packet is ready to be read from the interface.
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * @param data - the pointer to the received data buffer
 * @param size - the length of received data buffer
 * @return ERR_OK if the packet is being handled (we take ownership of the data buffer)
 *         ERR_MEM if the packet cannot be handled (we don't take ownership of the data buffer,
 *         therefore the caller should release it)
 * Implements ethif_input_Activity
 */
static err_t ethif_input(struct netif *netif, const uint8_t * data, uint16_t size)
{
    err_t ret = ERR_MEM;

    /* Allocate a PBUF_REF pointing to the receive buffer */
    struct pbuf* p  = pbuf_alloc(PBUF_RAW, size, PBUF_REF);
    if (NULL != p)
    {
        ret = ERR_OK;
        p->payload = data;
        if (ERR_OK != netif->input(p, netif))
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethif_input: IP input error\n"));
            (void)pbuf_free(p);
        }
    }

    return ret;
}

#endif /* !NO_SYS */

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif - the already initialized lwip network interface structure
 *        for this ethernetif
 * Implements ethif_low_level_init_Activity
 */
static void ethif_low_level_init(struct netif *netif)
{
    uint8_t i = 0U;

    /* set MAC hardware address length */
    netif->hwaddr_len = NETIF_MAX_HWADDR_LEN;

    /* set MAC hardware address */
    for (i = 0U; i < NETIF_MAX_HWADDR_LEN; i++)
    {
      netif->hwaddr[i] = netif_cfg[netif->num]->hwaddr[i];
    }

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = (u8_t)(NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET);
#if LWIP_IGMP
    netif->flags = netif->flags | (u8_t)NETIF_FLAG_IGMP;
    /* Will add the function igmp_mac_filter to the netif */
    (netif)->igmp_mac_filter = igmp_eth_filter;
#endif /*LWIP_IGMP*/

    NETIF_SET_CHECKSUM_CTRL(netif, NETIF_CHECKSUM_SETTING);

    g_netif[netif->num] = netif;

#if (ETH_HAS_EXTERNAL_RX_BUFFERS == STD_ON)
    ETHIF_BUFFER_t bd;
    /* fill in all descriptors in the Ring*/
    for(uint8_t i = 0; i < ETH_RXBD_NUM; i++)
    {
        bd.Data = &ethif_DataBuffer[i * ETH_43_ETH_MAX_RXBUFFLEN_SUPPORTED];
        Eth_ProvideRxBuffer(netif_cfg[netif->num]->num, EthConf_EthCtrlConfigIngressFifo_EthCtrlConfigIngressFifo_0, &bd.Data[0]);
        rx_bufs[i] = NULL;
    }
#endif

    Eth_SetControllerMode(netif_cfg[netif->num]->num, ETH_MODE_ACTIVE);

#if STD_ON == ETH_UPDATE_PHYS_ADDR_FILTER_API
    Eth_UpdatePhysAddrFilter(netif_cfg[netif->num]->num, &netif_cfg[netif->num]->hwaddr[0], ETH_ADD_TO_FILTER);
#else
#warning "This feature is enabled in the TCP/IP stack but it is not enabled in the driver."
#endif /* ETH_UPDATE_PHYS_ADDR_FILTER_API */

    /* Enable ARP Off-loading:
     *   - the board will reply to ARP requests with the
     *     IP (e.g. 192.168.0.200) to MAC (e.g. 10:11:12:00:00:00) address mapping. */
#if 0 /* FEATURE_ETH_ARP_EN */
    for (i = 0U; i < ETHIF_NUMBER; i++)
    {
        if ((!netif_cfg[i]->has_dhcp) && (!netif_cfg[i]->has_auto_ip))
        {
            ETH_Ip_SetArpOffloading(netif_cfg[netif->num]->num, netif_cfg[i]->ip_addr, true);
        }
    }
#endif /* FEATURE_ETH_ARP_EN */

#if defined(PHY_KSZ9031_SPEED10_100)
    /* Restart negotiation to speed 10/100M, applied only for PHY_KSZ9031 */
    if ((ETH_SPEED == ETH_SPEED_10M) || (ETH_SPEED == ETH_SPEED_100M))
    {
        /* Set 10/100Mbit/s operation and restart negotiation */

        /* not implemented */
    }
#endif /* defined(PHY_KSZ9031_SPEED10_100) */

#if LWIP_IPV6 && LWIP_IPV6_MLD
    if (netif_cfg[netif->num]->has_IPv6)
    {
        netif->flags = netif->flags | (u8_t)NETIF_FLAG_MLD6;
        /*If flag MLD6 flag is set, add mdl_ETH_filter function to netif*/
        (netif)->mld_mac_filter = mld_eth_filter;
        /*
        * For hardware/netifs that implement MAC filtering.
        * All-nodes link-local is handled by default, so we must let the hardware know
        * to allow multicast packets in.
        * Should set mld_mac_filter previously. */
         ip6_addr_t ip6_allnodes_ll;
         ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
         (void)netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
    }
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

    netif_set_link_up(netif);
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 * Implements ethif_ethernetif_init_Activity
 */
err_t ethif_ethernetif_init(struct netif *netif)
{
    err_t ret = ERR_OK;
    uint8_t i;
    LWIP_ASSERT("netif != NULL", (netif != NULL));

#if !NO_SYS
    LWIP_MEMPOOL_INIT(RX_POOL);
    err_t status = sys_mutex_new(&ethif_tx_lock);
    ret =sys_mbox_new((sys_mbox_t *)&in_flight_tx_pbufs, ETH_TXBD_NUM);
    LWIP_ASSERT("status == ETH_STATUS_SUCCESS", ret == ERR_OK);
    LWIP_ASSERT("status == ETH_STATUS_SUCCESS", status == ERR_OK);
    (void)status;
#endif /* !NO_SYS */
    for (i=0;i<ETH_TXBD_NUM;i++)
    {
        tx_pbufs[i] = NULL;
    }

    netif->name[0] = netif_cfg[netif->num]->name[0];
    netif->name[1] = netif_cfg[netif->num]->name[1];

#if LWIP_IPV4
#if LWIP_ARP
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    netif->output = etharp_output;
#else /* LWIP_ARP */
    netif->output = NULL; /* not used for PPPoE */
#endif /* LWIP_ARP */
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
    if (netif_cfg[netif->num]->has_IPv6)
    {
        netif->output_ip6 = ethip6_output;
    }
#endif /* LWIP_IPV6 */
    netif->linkoutput = ethif_low_level_output;
#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    if(NULL != netif_cfg[netif->num]->hostname)
    {
        netif->hostname = netif_cfg[netif->num]->hostname;
    }
#endif /* LWIP_NETIF_HOSTNAME */
#if LWIP_SNMP
    /*
    * Initialize the snmp variables and counters inside the struct netif.
    * The last argument should be replaced with your link speed, in units
    * of bits per second.
    */
    NETIF_INIT_SNMP(netif, (u8_t)snmp_ifType_ethernet_csmacd, (u32_t)100000000);
#endif /* LWIP_SNMP */

    /* initialize the hardware */
    ethif_low_level_init(netif);

    return ret;
}

/**
 * Clean up network interface and internal structures
 *
 * @param netif - the lwip network interface structure for this ethernetif
 * Implements ethif_ethernetif_shutdown_Activity
 */
void ethif_ethernetif_shutdown(struct netif *netif)
{
#if !NO_SYS
    struct pbuf *p;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

    /* Kill the polling thread */
    sys_thread_delete(poll_thread);

    /* Empty and free the mboxes */
    while (0 == sys_arch_mbox_tryfetch((sys_mbox_t *)&in_flight_tx_pbufs, (void**)&p))
    {
        (void)pbuf_free_callback(p);
    }
    sys_mbox_free((sys_mbox_t *)&in_flight_tx_pbufs);

    Eth_SetControllerMode(netif_cfg[netif->num]->num, ETH_MODE_DOWN);

    (void)sys_mutex_free(&ethif_tx_lock);

#else
    Eth_SetControllerMode(netif_cfg[netif->num]->num, ETH_MODE_DOWN);
#endif /* !NO_SYS */
}

#if !NO_SYS
void send_tx_pbuffs_dummy_char(void)
{
    struct pbuf* dummy  = pbuf_alloc(PBUF_RAW, ETH_RXBUFF_SIZE, PBUF_RAM);
    sys_mbox_post((sys_mbox_t *)&in_flight_tx_pbufs, dummy);
}

#endif /* !NO_SYS */

#if LWIP_IPV6
/**
 * @ingroup netif_ip6
 * Modify/Configure ETH driver setting  to forward (or stop forwarding) multicast packet for MLD (ICMPv6)
 * if "action" = NETIF_ADD_MAC_FILTER , ETH module will forward multicast packet of the group corresponding to "group"
 * if "action" = NETIF_DEL_MAC_FILTER , ETH module will stop forwarding multicast packet of the group corresponding to "group"
 *
 * @param netif the network interface
 * @*group IP address of the Multicast group the message have to be forwarded by the ETH module
 * @action action to be done (remove group from the forwarded packet or add group)
 * Implements design_id_IPv6_Activity
 */
err_t mld_eth_filter (struct netif *netif,
                       const ip6_addr_t *group,
                       enum netif_mac_filter_action action)
{
    /* Generate MAC address based on IPv6 address */
    uint8_t group_MAC[6];
    group_MAC[0] = 0x33;
    group_MAC[1] = 0x33;
    group_MAC[2] = (uint8_t)((IP6_ADDR_BLOCK7(group)) >> 8);
    group_MAC[3] = (uint8_t)(IP6_ADDR_BLOCK7(group));
    group_MAC[4] = (uint8_t)((IP6_ADDR_BLOCK8(group)) >> 8);
    group_MAC[5] = (uint8_t)(IP6_ADDR_BLOCK8(group));

    /* Configure Eth driver setting to forward or stop forwarding multicast packet for MLD (IPv6) */
    if (action != NETIF_DEL_MAC_FILTER)
    {
        /* Forward multicast packet of the group corresponding to group_MAC */
        Eth_UpdatePhysAddrFilter(netif_cfg[netif->num]->num, group_MAC, ETH_ADD_TO_FILTER);
    }
    else
    {
        /* Stop forwarding multicast packet of the group corresponding to group_MAC */
        Eth_UpdatePhysAddrFilter(netif_cfg[netif->num]->num, group_MAC, ETH_REMOVE_FROM_FILTER);
    }

    return ERR_OK;
}
#endif /*LWIP_IPV6*/

#if LWIP_IGMP && LWIP_IPV4
/**
 * @ingroup netif_ip4
 * Modify/Configure ETH driver setting  to forward (or stop forwarding) multicast packet for IGMP (IPv4)
 * if "action" = NETIF_ADD_MAC_FILTER , ETH module will forward multicast packet of the group corresponding to "group"
 * if "action" = NETIF_DEL_MAC_FILTER , ETH module will stop forwarding multicast packet of the group corresponding to "group"
 *
 * @param netif the network interface
 * @*group IP address of the Multicast group the message have to be forwarded by the ETH module
 * @action action to be done (remove group from the forwarded packet or add group)
 * Implements design_id_IGMP_Activity
 */

err_t igmp_eth_filter (struct netif *netif,
                        const ip4_addr_t *group,
                        enum netif_mac_filter_action action)
{
    /* Generate MAC address based on IPv4 address */
    uint8_t group_MAC[6];
    group_MAC[0] = 0x01;
    group_MAC[1] = 0x00;
    group_MAC[2] = 0x5e;
    group_MAC[3] = (0x7f & ip4_addr2(group));
    group_MAC[4] = ip4_addr3(group);
    group_MAC[5] = ip4_addr4(group);

    /* Configure ETH driver setting to forward or stop forwarding multicast packet for IGMP (IPv4) */
    if (action != NETIF_DEL_MAC_FILTER)
    {
        /* Forward multicast packet of the group corresponding to group_MAC */
        Eth_UpdatePhysAddrFilter(netif_cfg[netif->num]->num, group_MAC, ETH_ADD_TO_FILTER);
    }
    else
    {
        /* Stop forwarding multicast packet of the group corresponding to group_MAC */
        Eth_UpdatePhysAddrFilter(netif_cfg[netif->num]->num, group_MAC, ETH_REMOVE_FROM_FILTER);
    }

    return ERR_OK;
}
#endif /*LWIP_IGMP && LWIP_IPV4*/

/**
 * Register pre-input handler
 * This handler is called before a frame is input to the TCPIP stack
 * If returns 0, the frame should be forwarded to the stack
 * If returns something else, the frame is used by other applications
 *
 * @param handler - the handler to be installed
 */
void ethif_register_rx_buff_process_condition_handler(rx_buff_process_condition_handler_t handler)
{
    rx_buff_process_handler = handler;
}

void memcpy_64(uint64_t *dst, uint64_t * src, unsigned int len)
{
    len = len >> 3;
    while(len --)
    {
        *dst++ = *src++;
    }
}
void memcpy_custom(void *dst, const void * src, unsigned int len)
{
    if (len <= 32)
    {
        memcpy(dst,src,len);
    }
    else /* len > 32*/
    {
        unsigned int last_src = (unsigned int)src;
        unsigned int last_dest = (unsigned int)dst;
        char *pd = dst;
        const char *ps = src;
        if (0 == (((last_src) ^ (last_dest)) & (0xf)))
        {
            unsigned int alignment =  (last_dest) & (0xf);
            /* copy first unaligned bytes */
            alignment = 16 - alignment;
            memcpy(pd,ps, alignment);
            pd += alignment;
            ps += alignment;
            len -= alignment;
            /* Copy 64 bit aligned */
            memcpy_64((uint64_t*)pd,(uint64_t*)ps,len);
            /* Copy remainder, in case this applies */
            if(0 != (len & 0xf))
            {
                /* Keep track of how much memcpy_64 has copied */
                pd += (((len) >> 3) << 3);
                ps += (((len) >> 3) << 3);
                memcpy(pd,ps, (len & 0xf));
            }
        }
        else
        {
            /* Vectors not alligned */
            memcpy(dst,src,len);
        }
    }
}

volatile uint32 EthIf_RxIndications[10]    = {0};
volatile uint32 EthIf_TxConfirmations[10]  = {0};
volatile boolean EthIf_ModeIndications[10] = {0};
volatile uint16 EthIf_ChecksumValue[10]    = {0};

/**
* @brief          This function handles the received Ethernet frame.
* @details        Function should parse the received frame and pass the gathered
*                 information to the appropriate upper layer module.
* @note           The passed data buffer is no longer valid after the function
*                 is exited.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the Eth module.
* @param[in]      CtrlIdx Index of the controller which received the frame.
* @param[in]      FrameType The received frame Ethertype (from the frame header)
* @param[in]      IsBroadcast The value TRUE indicates that the received frame
*                 was sent to broadcast address (ff-ff-ff-ff-ff-ff)
* @param[in]      PhysAddrPtr Pointer to received frame source MAC address
*                 (6 bytes).
* @param[in]      DataPtr Data buffer containing the received Ethernet frame 
*                 payload.
* @param[in]      LenByte Length of the data in the buffer DataPtr.
*
*/
void EthIf_RxIndication(\
                        uint8 CtrlIdx,\
                        Eth_FrameType FrameType, \
                        boolean IsBroadcast, \
                        const uint8* PhysAddrPtr, \
                        const Eth_DataType* DataPtr,\
                        uint16 LenByte)
{
    (void)FrameType;
    (void)IsBroadcast;
    (void)PhysAddrPtr;

    ++EthIf_RxIndications[CtrlIdx];
    EthIf_ChecksumValue[CtrlIdx] = *((uint16 *)(&DataPtr[10U]));

    DataPtr -= ETHIF_FRAME_PAYLOAD_OFFSET;
    LenByte += ETHIF_FRAME_HEADER_LENGTH;

#if defined D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined CPU_CORTEX_M7
    DataCacheInvbyAddr((uint32)bd.Data, bd.Length);
#endif /*  D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON) && defined CPU_CORTEX_M7 */

    (void)ethif_input((struct netif *)g_netif[CtrlIdx], (uint8_t*)DataPtr, (uint16_t)LenByte);

}

/**
* @brief          This function confirms that transmission of an Ethernet frame
*                 was finished.
* @details        Function should notify the appropriate upper layer module that
*                 the data transmission was successfully finished.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the Eth module.
* @param[in]      CtrlIdx Index of the controller which transmitted the frame.
* @param[in]      BufIdx Index of the transmitted data buffer.

*/
void EthIf_TxConfirmation(uint8 CtrlIdx, \
                          Eth_BufIdxType BufIdx, \
                          Std_ReturnType Result)
{
    (void)CtrlIdx;
    (void)BufIdx;
    (void)Result;
    uint8_t i = 0;

    /* This is an empty stub function */
    ++EthIf_TxConfirmations[CtrlIdx];

    for( i =0; i < ETH_TXBD_NUM; i++)
    {
        if (tx_pbufs[i] != NULL )
        {
#if NO_SYS
            (void)pbuf_free(tx_pbufs[i]);
#else /* NO_SYS */
            /* request to free the outstanding pbuf on tcpip thread */
            (void)pbuf_free_callback(tx_pbufs[i]);
#endif /* NO_SYS */
            tx_pbufs[i] = NULL;
        }
    }
}

/**
* @brief          This function indicate that driver mode has been changed
* @details        Called asynchronously when mode has been read out. Triggered by previous
*                 Eth_SetControllerMode call. Can directly be called within the trigger functions.
* @warning        This is only an empty stub function provided only to be able
*                 to compile and link the Eth module.
* @param[in]      CtrlIdx Index of the controller which mode has been changed.
* @param[in]      CtrlMode New mode of correspond Eth driver.

*/
void EthIf_CtrlModeIndication( \
                                uint8 CtrlIdx, \
                                Eth_ModeType CtrlMode \
                             )
{
    /* This is an empty stub function */
    EthIf_ModeIndications[CtrlIdx] = TRUE;
    (void)CtrlMode;
}
