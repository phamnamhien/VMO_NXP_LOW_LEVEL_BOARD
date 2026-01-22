/*
 * test.c - lwIP test with LAN9646 + GMAC
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#if defined(USING_OS_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "Eth_43_GMAC.h"
#include "Gmac_Ip.h"

#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/api.h"
#include "lwip/arch.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/etharp.h"
#include "netif/ethernet.h"
#include "netifcfg.h"
#include "lwipcfg.h"

#if LWIP_HTTPD_APP
#if LWIP_HTTPD_APP_NETCONN
#include "apps/httpserver/httpserver-netconn.h"
#else
#include "lwip/apps/httpd.h"
#endif
#endif

#if LWIP_LWIPERF_APP
#include "lwip/apps/lwiperf.h"
#endif

#if LWIP_NETBIOS_APP
#include "lwip/apps/netbiosns.h"
#endif

#if LWIP_SNTP_APP
#include "lwip/apps/sntp.h"
#endif

#include "lwip/apps/mdns.h"

#if LWIP_TCPECHO_APP
#if LWIP_TCPECHO_APP_NETCONN
#include "apps/tcpecho/tcpecho.h"
#else
#include "apps/tcpecho_raw/tcpecho_raw.h"
#endif
#endif

#if LWIP_UDPECHO_APP
#if LWIP_UDPECHO_APP_NETCONN
#include "apps/udpecho/udpecho.h"
#else
#include "apps/udpecho_raw/udpecho_raw.h"
#endif
#endif

#if !NO_SYS
#include "apps/ccov/ccov.h"
#endif

#include "apps/netif_shutdown/netif_shutdown.h"

#if NO_SYS
#include "lwip/ip4_frag.h"
#include "lwip/igmp.h"
#endif

#if defined(GMACIF_NUMBER)
#include "gmacif.h"
#else
#include "ethif_port.h"
#endif

#include "log_debug.h"

#define TAG "LWIP"

#ifndef LWIP_INIT_COMPLETE_CALLBACK
#define LWIP_INIT_COMPLETE_CALLBACK 0
#endif

struct netif network_interfaces[ETHIF_NUMBER];

#if LWIP_DHCP
struct dhcp netif_dhcp;
#endif

#if LWIP_AUTOIP
struct autoip netif_autoip;
#endif

#if defined(USING_RTD)
uint32 start_time = 0;
#else
uint32_t start_time = 0;
#endif

uint32 tests_timeout = 1200;

extern void sys_init(void);

#if defined(USING_OS_FREERTOS)
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
#define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#endif

static void mainLoopTask(void *pvParameters);
void start_example(void);

#if LWIP_INIT_COMPLETE_CALLBACK
void tcpip_init_complete_callback(void);
#endif

/* Debug RX frames */
static void debug_rx_frames(void)
{
    Eth_RxStatusType rxStatus;

    /* Gọi Eth_43_GMAC_Receive để xử lý RX */
    Eth_43_GMAC_Receive(0, 0, &rxStatus);

    /* Check RX status */
    if (rxStatus != ETH_NOT_RECEIVED) {
        LOG_I(TAG, "RX Status: %d (0=NOT_RECEIVED, 1=RECEIVED, 2=RECEIVED_MORE)", rxStatus);

        /* In thêm RX counters */
        LOG_I(TAG, "RX Good: %lu, RX CRC Err: %lu",
              (unsigned long)IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD,
              (unsigned long)IP_GMAC_0->RX_CRC_ERROR_PACKETS);
        LOG_I(TAG, "RX Unicast: %lu, RX Broadcast: %lu",
              (unsigned long)IP_GMAC_0->RX_UNICAST_PACKETS_GOOD,
              (unsigned long)IP_GMAC_0->RX_BROADCAST_PACKETS_GOOD);
    }
}

#if LWIP_NETIF_STATUS_CALLBACK
static void status_callback(struct netif *state_netif)
{
    if (netif_is_up(state_netif)) {
#if LWIP_IPV4
        LOG_I(TAG, "Network UP - IP: %s", ip4addr_ntoa(netif_ip4_addr(state_netif)));
        LOG_I(TAG, "  Netmask: %s", ip4addr_ntoa(netif_ip4_netmask(state_netif)));
        LOG_I(TAG, "  Gateway: %s", ip4addr_ntoa(netif_ip4_gw(state_netif)));
#else
        LOG_I(TAG, "Network UP");
#endif
#if LWIP_MDNS_RESPONDER
        mdns_resp_netif_settings_changed(state_netif);
#endif
    } else {
        LOG_W(TAG, "Network DOWN");
    }
}
#endif

#if LWIP_NETIF_LINK_CALLBACK
static void link_callback(struct netif *state_netif)
{
    if (netif_is_link_up(state_netif)) {
        LOG_I(TAG, "Link UP");
    } else {
        LOG_W(TAG, "Link DOWN");
    }
}
#endif

static void interface_init(void)
{
    LOG_I(TAG, "Initializing network interfaces...");

    for (int i = 0; i < ETHIF_NUMBER; i++) {
#if LWIP_IPV4
        ip4_addr_t ipaddr, netmask, gw;
#endif
#if LWIP_DHCP || LWIP_AUTOIP
        err_t err;
#endif

#if LWIP_IPV4
        ip4_addr_set_zero(&gw);
        ip4_addr_set_zero(&ipaddr);
        ip4_addr_set_zero(&netmask);

        if ((!netif_cfg[i]->has_dhcp) && (!netif_cfg[i]->has_auto_ip)) {
            IP4_ADDR((&gw), netif_cfg[i]->gw[0], netif_cfg[i]->gw[1],
                     netif_cfg[i]->gw[2], netif_cfg[i]->gw[3]);
            IP4_ADDR((&ipaddr), netif_cfg[i]->ip_addr[0], netif_cfg[i]->ip_addr[1],
                     netif_cfg[i]->ip_addr[2], netif_cfg[i]->ip_addr[3]);
            IP4_ADDR((&netmask), netif_cfg[i]->netmask[0], netif_cfg[i]->netmask[1],
                     netif_cfg[i]->netmask[2], netif_cfg[i]->netmask[3]);

            LOG_I(TAG, "Interface %d: Static IP %d.%d.%d.%d", i,
                  netif_cfg[i]->ip_addr[0], netif_cfg[i]->ip_addr[1],
                  netif_cfg[i]->ip_addr[2], netif_cfg[i]->ip_addr[3]);
        }
#endif

#if NO_SYS
        netif_set_default(netif_add(&network_interfaces[i], &ipaddr, &netmask,
                                    &gw, NULL, ETHIF_INIT, netif_input));
#else
        netif_set_default(netif_add(&network_interfaces[i], &ipaddr, &netmask,
                                    &gw, NULL, ETHIF_INIT, tcpip_input));
#endif

#if LWIP_IPV6
        if (netif_cfg[i]->has_IPv6) {
            netif_create_ip6_linklocal_address(&network_interfaces[i], 1);
            LOG_I(TAG, "IPv6 link-local address created");
        }
#endif

#if LWIP_NETIF_STATUS_CALLBACK
        netif_set_status_callback(&network_interfaces[i], status_callback);
#endif

#if LWIP_NETIF_LINK_CALLBACK
        netif_set_link_callback(&network_interfaces[i], link_callback);
#endif

#if LWIP_AUTOIP
        if (netif_cfg[i]->has_auto_ip) {
            autoip_set_struct(&network_interfaces[i], &netif_autoip);
        }
#endif

#if LWIP_DHCP
        if (netif_cfg[i]->has_dhcp) {
            dhcp_set_struct(&network_interfaces[i], &netif_dhcp);
            LOG_I(TAG, "Interface %d: DHCP enabled", i);
        }
#endif

        netif_set_up(&network_interfaces[i]);
        LOG_I(TAG, "Interface %d: UP", i);

#if LWIP_DHCP
        if (netif_cfg[i]->has_dhcp) {
            err = dhcp_start((struct netif *)&network_interfaces[i]);
            if (err == ERR_OK) {
                LOG_I(TAG, "DHCP started");
            } else {
                LOG_E(TAG, "DHCP start failed: %d", err);
            }
        }
#endif

#if LWIP_AUTOIP
        else if (netif_cfg[i]->has_auto_ip) {
            err = autoip_start(&network_interfaces[i]);
            if (err == ERR_OK) {
                LOG_I(TAG, "AutoIP started");
            } else {
                LOG_E(TAG, "AutoIP start failed: %d", err);
            }
        }
#endif
    }
}


static void debug_gmac_status(void)
{
    Eth_ModeType mode;
    Eth_43_GMAC_GetControllerMode(0, &mode);
    LOG_I(TAG, "GMAC Controller Mode: %d", mode);

    /* Check GMAC IP registers directly */
    uint32_t mac_config = IP_GMAC_0->MAC_CONFIGURATION;
    uint32_t dma_mode = IP_GMAC_0->DMA_MODE;
    uint32_t dma_ch0_ctrl = IP_GMAC_0->DMA_CH0_CONTROL;
    uint32_t dma_ch0_tx = IP_GMAC_0->DMA_CH0_TX_CONTROL;
    uint32_t dma_ch0_rx = IP_GMAC_0->DMA_CH0_RX_CONTROL;
    uint32_t mtl_txq0 = IP_GMAC_0->MTL_TXQ0_OPERATION_MODE;
    uint32_t mtl_rxq0 = IP_GMAC_0->MTL_RXQ0_OPERATION_MODE;

    LOG_I(TAG, "MAC_CONFIG: 0x%08lX [TE=%d RE=%d]",
          (unsigned long)mac_config,
          (int)((mac_config >> 1) & 1),  /* TE - Transmitter Enable */
          (int)((mac_config >> 0) & 1)); /* RE - Receiver Enable */

    LOG_I(TAG, "DMA_MODE: 0x%08lX", (unsigned long)dma_mode);
    LOG_I(TAG, "DMA_CH0_CTRL: 0x%08lX", (unsigned long)dma_ch0_ctrl);
    LOG_I(TAG, "DMA_CH0_TX: 0x%08lX [ST=%d]",
          (unsigned long)dma_ch0_tx,
          (int)(dma_ch0_tx & 1));  /* ST - Start Transmission */
    LOG_I(TAG, "DMA_CH0_RX: 0x%08lX [SR=%d]",
          (unsigned long)dma_ch0_rx,
          (int)(dma_ch0_rx & 1));  /* SR - Start Receive */
    LOG_I(TAG, "MTL_TXQ0: 0x%08lX", (unsigned long)mtl_txq0);
    LOG_I(TAG, "MTL_RXQ0: 0x%08lX", (unsigned long)mtl_rxq0);
}
#if LWIP_LWIPERF_APP
static void lwiperf_report(void *arg, enum lwiperf_report_type report_type,
                           const ip_addr_t* local_addr, u16_t local_port,
                           const ip_addr_t* remote_addr, u16_t remote_port,
                           u32_t bytes_transferred, u32_t ms_duration,
                           u32_t bandwidth_kbitpsec)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(local_addr);
    LWIP_UNUSED_ARG(local_port);

    LOG_I(TAG, "IPERF: type=%d, remote=%s:%d, bytes=%lu, duration=%lums, %lukbps",
          (int)report_type, ipaddr_ntoa(remote_addr), (int)remote_port,
          bytes_transferred, ms_duration, bandwidth_kbitpsec);
}
#endif

static void apps_init(void)
{
    LOG_I(TAG, "Initializing applications...");

#if LWIP_NETBIOS_APP && LWIP_UDP
    netbiosns_init();
#if LWIP_NETIF_HOSTNAME
    netbiosns_set_name(netif_default->hostname);
#else
    netbiosns_set_name("NETBIOSLWIPDEV");
#endif
    LOG_I(TAG, "NetBIOS initialized");
#endif

#if LWIP_HTTPD_APP && LWIP_TCP
#if LWIP_HTTPD_APP_NETCONN
    http_server_netconn_init();
#else
    httpd_init();
#endif
    LOG_I(TAG, "HTTP server initialized");
#endif

#if LWIP_TCPECHO_APP
#if LWIP_NETCONN && LWIP_TCPECHO_APP_NETCONN
    tcpecho_init();
#else
    tcpecho_raw_init();
#endif
    LOG_I(TAG, "TCP Echo initialized");
#endif

#if LWIP_UDPECHO_APP
#if LWIP_NETCONN && LWIP_UDPECHO_APP_NETCONN
    for (int i = 0; i < ETHIF_NUMBER; i++) {
        udpecho_init(&network_interfaces[i]);
    }
#else
    udpecho_raw_init();
#endif
    LOG_I(TAG, "UDP Echo initialized");
#endif

#if LWIP_LWIPERF_APP
    (void)lwiperf_start_tcp_server_default(lwiperf_report, NULL);
    LOG_I(TAG, "IPERF server initialized");
#endif

#if !NO_SYS
    for (int i = 0; i < ETHIF_NUMBER; i++) {
        Coverage_init(&network_interfaces[i]);
    }
#endif
}

static void test_init(void* arg)
{
#if NO_SYS
    LWIP_UNUSED_ARG(arg);
#else
    sys_sem_t* init_sem = (sys_sem_t*)arg;
    LWIP_ASSERT("init_sem != NULL", init_sem != NULL);
#endif

    start_time = OsIf_GetMilliseconds();
    start_time = start_time / (double)1000;

    LOG_I(TAG, "test_init started");

    interface_init();

    LOG_I(TAG, "=== GMAC Status BEFORE SetControllerMode ===");
    debug_gmac_status();

    LOG_I(TAG, "Setting GMAC to ACTIVE...");
    Std_ReturnType ret = Eth_43_GMAC_SetControllerMode(0, ETH_MODE_ACTIVE);
    LOG_I(TAG, "GMAC SetControllerMode result: %d", ret);

    /* Test raw TX */
    extern void test_raw_tx(void);
    for (int i = 0; i < 5; i++) {
        test_raw_tx();
        sys_msleep(1000);
    }
    extern void debug_lan9646_mib(void);
    debug_lan9646_mib();
    LOG_I(TAG, "=== GMAC Status AFTER SetControllerMode ===");
    debug_gmac_status();

    apps_init();

    LOG_I(TAG, "test_init complete");

#if !NO_SYS
    sys_sem_signal(init_sem);
#endif
}
static void mainLoopTask(void* pvParameters)
{
    (void)pvParameters;

    LOG_I(TAG, "mainLoopTask started");

#if !NO_SYS
    err_t err;
    sys_sem_t init_sem;

    err = sys_sem_new(&init_sem, 0);
    LWIP_ASSERT("failed to create init_sem", err == (err_t)ERR_OK);
    LWIP_UNUSED_ARG(err);

    LOG_I(TAG, "Initializing TCP/IP stack...");
    tcpip_init(test_init, (void*)&init_sem);

    (void)sys_sem_wait(&init_sem);
    sys_sem_free(&init_sem);

#if (LWIP_SOCKET || LWIP_NETCONN) && LWIP_NETCONN_SEM_PER_THREAD
    netconn_thread_init();
#endif
#else
    sys_init();
    lwip_init();
    test_init(NULL);
#endif

#if LWIP_INIT_COMPLETE_CALLBACK
    tcpip_init_complete_callback();
#endif

    LOG_I(TAG, "Entering main loop...");

    uint32_t last_print = 0;

    while (1) {
        /* Check RX frames liên tục */
        debug_rx_frames();

#if NO_SYS
        sys_check_timeouts();
#else
        sys_msleep(100);  /* Giảm xuống 100ms để check RX thường xuyên hơn */
#endif

#if defined(USING_RTD)
        uint32 time_now = OsIf_GetMilliseconds() / 1000;
#else
        uint32_t time_now = OSIF_GetMilliseconds() / 1000;
#endif

        /* Print stats every 5 seconds */
        if (time_now - last_print >= 5) {
            last_print = time_now;

            LOG_I(TAG, "--- Stats at %lu sec ---", (unsigned long)time_now);

            /* Debug MAC address */
            uint8_t mac[6];
            Eth_43_GMAC_GetPhysAddr(0, mac);
            LOG_I(TAG, "GMAC MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

            LOG_I(TAG, "IP: %s", ip4addr_ntoa(netif_ip4_addr(&network_interfaces[0])));
#if LWIP_STATS
            LOG_I(TAG, "Link RX: %u, TX: %u", lwip_stats.link.recv, lwip_stats.link.xmit);
            LOG_I(TAG, "ARP RX: %u, TX: %u", lwip_stats.etharp.recv, lwip_stats.etharp.xmit);
            LOG_I(TAG, "IP RX: %u, drop: %u", lwip_stats.ip.recv, lwip_stats.ip.drop);
            LOG_I(TAG, "ICMP RX: %u, TX: %u", lwip_stats.icmp.recv, lwip_stats.icmp.xmit);
#endif
            /* TX debug */
            LOG_I(TAG, "TX Packets: %lu", (unsigned long)IP_GMAC_0->TX_PACKET_COUNT_GOOD_BAD);
            LOG_I(TAG, "RX Packets: %lu", (unsigned long)IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD);

            /* GMAC TX debug */
            uint32_t dma_status = IP_GMAC_0->DMA_CH0_STATUS;
            uint32_t mtl_tx_debug = IP_GMAC_0->MTL_TXQ0_DEBUG;
            uint32_t mac_debug = IP_GMAC_0->MAC_DEBUG;

            LOG_I(TAG, "DMA_STATUS: 0x%08lX [TPS=%lu RPS=%lu]",
                  (unsigned long)dma_status,
                  (unsigned long)((dma_status >> 12) & 0xF),
                  (unsigned long)((dma_status >> 8) & 0xF));
            LOG_I(TAG, "MTL_TX_DEBUG: 0x%08lX", (unsigned long)mtl_tx_debug);
            LOG_I(TAG, "MAC_DEBUG: 0x%08lX", (unsigned long)mac_debug);
        }

        if (time_now - start_time >= tests_timeout) {
            LOG_W(TAG, "Test timeout, shutting down...");
            for (int i = 0; i < ETHIF_NUMBER; i++) {
                ETHIF_SHUTDOWN(&network_interfaces[i]);
            }
            end_tcpip_execution(NULL);
        }
    }
}
void start_example(void)
{
    LOG_I(TAG, "");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "  lwIP + LAN9646 + GMAC Starting...");
    LOG_I(TAG, "========================================");

#if defined(USING_OS_FREERTOS)
    BaseType_t ret = xTaskCreate(mainLoopTask, "mainloop", 512U, NULL,
                                  DEFAULT_THREAD_PRIO, NULL);
    LWIP_ASSERT("failed to create mainloop", ret == pdPASS);

    LOG_I(TAG, "Starting FreeRTOS scheduler...");
    vTaskStartScheduler();

    for (;;) {}
#else
    mainLoopTask(NULL);
#endif
}

#if defined(USING_OS_FREERTOS)
void vAssertCalled(uint32_t ulLine, const char * const pcFileName)
{
    LOG_E(TAG, "ASSERT! Line %lu, file %s", ulLine, pcFileName);
    taskENTER_CRITICAL();
    {
        while (1) {}
    }
    taskEXIT_CRITICAL();
}

void vApplicationMallocFailedHook(void)
{
    LOG_E(TAG, "Malloc failed!");
    vAssertCalled(__LINE__, __FILE__);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void)pxTask;
    LOG_E(TAG, "Stack overflow: %s", pcTaskName);
    vAssertCalled(__LINE__, __FILE__);
}

void vMainConfigureTimerForRunTimeStats(void)
{
}

uint32_t ulMainGetRunTimeCounterValue(void)
{
    return 0UL;
}
#endif

