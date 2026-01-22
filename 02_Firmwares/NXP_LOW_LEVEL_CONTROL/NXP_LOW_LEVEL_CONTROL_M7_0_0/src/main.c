/**
 * \file            main.c
 * \brief           LAN9646 + Eth + FreeRTOS + lwIP
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "S32K388.h"
#include "Mcal.h"
#include "Mcu.h"
#include "Port.h"
#include "OsIf.h"
#include "Platform.h"
#include "Gpt.h"
#include "ethif_port.h"
#include "Eth_43_GMAC.h"

#include "lan9646.h"
#include "lan9646_switch.h"
#include "s32k3xx_soft_i2c.h"
#include "log_debug.h"

/* lwIP includes */
#if defined(USING_OS_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#endif

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

#define TAG "MAIN"

#define LAN9646_SCL_CHANNEL     DioConf_DioChannel_SCL_CH
#define LAN9646_SDA_CHANNEL     DioConf_DioChannel_SDA_CH
#define LAN9646_I2C_SPEED       5U

#ifndef LWIP_INIT_COMPLETE_CALLBACK
#define LWIP_INIT_COMPLETE_CALLBACK 0
#endif

static lan9646_t g_lan9646;
static softi2c_t g_i2c;

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

#if LWIP_INIT_COMPLETE_CALLBACK
void tcpip_init_complete_callback(void);
#endif

/* I2C Callbacks */
static lan9646r_t i2c_init_cb(void) {
    softi2c_pins_t pins = {
        .scl_channel = LAN9646_SCL_CHANNEL,
        .sda_channel = LAN9646_SDA_CHANNEL,
        .delay_us = LAN9646_I2C_SPEED,
    };
    return (softi2c_init(&g_i2c, &pins) == softi2cOK) ? lan9646OK : lan9646ERR;
}

static lan9646r_t i2c_write_cb(uint8_t dev_addr, const uint8_t* data, uint16_t len) {
    return (softi2c_write(&g_i2c, dev_addr, data, len) == softi2cOK) ? lan9646OK : lan9646ERR;
}

static lan9646r_t i2c_read_cb(uint8_t dev_addr, uint8_t* data, uint16_t len) {
    return (softi2c_read(&g_i2c, dev_addr, data, len) == softi2cOK) ? lan9646OK : lan9646ERR;
}

static lan9646r_t i2c_mem_write_cb(uint8_t dev_addr, uint16_t mem_addr,
                                   const uint8_t* data, uint16_t len) {
    return (softi2c_mem_write(&g_i2c, dev_addr, mem_addr, 2, data, len) == softi2cOK)
           ? lan9646OK : lan9646ERR;
}

static lan9646r_t i2c_mem_read_cb(uint8_t dev_addr, uint16_t mem_addr,
                                  uint8_t* data, uint16_t len) {
    return (softi2c_mem_read(&g_i2c, dev_addr, mem_addr, 2, data, len) == softi2cOK)
           ? lan9646OK : lan9646ERR;
}

static void delay_ms(uint32_t ms) {
    volatile uint32_t i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 16000; j++) {
            __asm("NOP");
        }
    }
}

static void debug_rgmii_clocks(void)
{
    LOG_I(TAG, "=== RGMII Clock Debug ===");

    /* DCM_GPR clock config */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;

    LOG_I(TAG, "DCMRWF1: 0x%08lX", (unsigned long)dcmrwf1);
    LOG_I(TAG, "  MAC_CONF_SEL: %lu (0=MII 1=RMII 2=RGMII)",
          (unsigned long)(dcmrwf1 & 0x7));

    LOG_I(TAG, "DCMRWF3: 0x%08lX", (unsigned long)dcmrwf3);
    LOG_I(TAG, "  RX_CLK_MUX_BYPASS: %lu", (unsigned long)(dcmrwf3 & 0x1));
    LOG_I(TAG, "  TX_CLK_TERM_EN: %lu", (unsigned long)((dcmrwf3 >> 1) & 0x1));
    LOG_I(TAG, "  RX_CLK_TERM_EN: %lu", (unsigned long)((dcmrwf3 >> 2) & 0x1));
    LOG_I(TAG, "  TX_CLK_OUT_EN: %lu", (unsigned long)((dcmrwf3 >> 3) & 0x1));
    LOG_I(TAG, "  TX_CLK_DLY_EN: %lu", (unsigned long)((dcmrwf3 >> 4) & 0x1));
    LOG_I(TAG, "  RX_CLK_DLY_EN: %lu", (unsigned long)((dcmrwf3 >> 5) & 0x1));

    /* MC_CGM Clock MUXes - ĐÚNG! */
    LOG_I(TAG, "MC_CGM MUX_7_CSS: 0x%08lX (GMAC0_RX_CLK)",
          (unsigned long)IP_MC_CGM->MUX_7_CSS);
    LOG_I(TAG, "MC_CGM MUX_8_CSS: 0x%08lX (GMAC0_TX_CLK)",
          (unsigned long)IP_MC_CGM->MUX_8_CSS);
    LOG_I(TAG, "MC_CGM MUX_9_CSS: 0x%08lX (GMAC0_TS_CLK)",
          (unsigned long)IP_MC_CGM->MUX_9_CSS);

    /* Dividers */
    LOG_I(TAG, "MC_CGM MUX_8_DC_0: 0x%08lX (GMAC0_TX_CLK divider)",
          (unsigned long)IP_MC_CGM->MUX_8_DC_0);

    /* Check LAN9646 clock output */
    uint8_t lan_clk_ctrl;
    lan9646_read_reg8(&g_lan9646, 0x0024, &lan_clk_ctrl);
    LOG_I(TAG, "LAN9646 Output Clock (0x0024): 0x%02X", lan_clk_ctrl);
    LOG_I(TAG, "  SYNCLKO Enable: %d", (lan_clk_ctrl >> 4) & 1);
    LOG_I(TAG, "  Frequency: %s", (lan_clk_ctrl & 0x08) ? "125MHz" : "25MHz");
    LOG_I(TAG, "  Source: %d (0=XI, 1-4=Port RX)", lan_clk_ctrl & 0x7);
}

static void debug_gmac_clocks(void)
{
    LOG_I(TAG, "=== GMAC Clock Source ===");

    /* MUX_8 = GMAC0_TX_CLK */
    uint32_t mux8_css = IP_MC_CGM->MUX_8_CSS;
    uint32_t mux8_dc0 = IP_MC_CGM->MUX_8_DC_0;

    LOG_I(TAG, "MUX_8_CSS: 0x%08lX", (unsigned long)mux8_css);
    LOG_I(TAG, "  GMAC TX Clock Select: %lu", (unsigned long)((mux8_css >> 24) & 0x3F));
    LOG_I(TAG, "  Switch Status: 0x%lX", (unsigned long)(mux8_css & 0xFFFFFF));

    LOG_I(TAG, "MUX_8_DC_0: 0x%08lX", (unsigned long)mux8_dc0);
    LOG_I(TAG, "  Divider Enable: %lu", (unsigned long)((mux8_dc0 >> 31) & 1));
    LOG_I(TAG, "  Divider: %lu", (unsigned long)(mux8_dc0 & 0xFF));

    /* MUX_7 = GMAC0_RX_CLK */
    uint32_t mux7_css = IP_MC_CGM->MUX_7_CSS;
    LOG_I(TAG, "MUX_7_CSS: 0x%08lX (GMAC RX Clock)", (unsigned long)mux7_css);
}

static void debug_lan9646_detail(void)
{
    LOG_I(TAG, "=== LAN9646 Port 6 Detail ===");

    uint8_t xmii_ctrl0, xmii_ctrl1, port_status, mstp_state;
    uint32_t membership;

    /* XMII Control */
    lan9646_read_reg8(&g_lan9646, 0x6300, &xmii_ctrl0);
    lan9646_read_reg8(&g_lan9646, 0x6301, &xmii_ctrl1);
    LOG_I(TAG, "XMII_CTRL0: 0x%02X", xmii_ctrl0);
    LOG_I(TAG, "XMII_CTRL1: 0x%02X [TX_DLY=%d RX_DLY=%d]",
          xmii_ctrl1, (xmii_ctrl1 >> 3) & 1, (xmii_ctrl1 >> 4) & 1);

    /* Port Status */
    lan9646_read_reg8(&g_lan9646, 0x6030, &port_status);
    LOG_I(TAG, "PORT_STATUS: 0x%02X [Speed=%d Duplex=%d]",
          port_status, (port_status >> 3) & 3, (port_status >> 2) & 1);

    /* MSTP State */
    lan9646_read_reg8(&g_lan9646, 0x6B04, &mstp_state);
    LOG_I(TAG, "MSTP_STATE: 0x%02X [TX=%d RX=%d Learn=%d]",
          mstp_state,
          (mstp_state >> 2) & 1,
          (mstp_state >> 1) & 1,
          !(mstp_state & 1));

    /* Membership */
    lan9646_read_reg32(&g_lan9646, 0x6A04, &membership);
    LOG_I(TAG, "MEMBERSHIP: 0x%08lX", (unsigned long)membership);

    /* MAC Control */
    uint8_t mac_ctrl0, mac_ctrl1;
    lan9646_read_reg8(&g_lan9646, 0x6400, &mac_ctrl0);
    lan9646_read_reg8(&g_lan9646, 0x6401, &mac_ctrl1);
    LOG_I(TAG, "MAC_CTRL0: 0x%02X", mac_ctrl0);
    LOG_I(TAG, "MAC_CTRL1: 0x%02X", mac_ctrl1);

    /* Switch Operation */
    uint8_t sw_op;
    lan9646_read_reg8(&g_lan9646, 0x0300, &sw_op);
    LOG_I(TAG, "SWITCH_OP: 0x%02X", sw_op);

    /* LUE Control */
    uint8_t lue_ctrl0;
    lan9646_read_reg8(&g_lan9646, 0x0310, &lue_ctrl0);
    LOG_I(TAG, "LUE_CTRL0: 0x%02X", lue_ctrl0);
}

static void debug_lan9646_mib(void)
{
    LOG_I(TAG, "=== LAN9646 Port 6 MIB ===");

    uint32_t ctrl, data;

    /* TX Broadcast */
    ctrl = (0x63 << 16) | 0x02000000;
    lan9646_write_reg32(&g_lan9646, 0x6500, ctrl);
    delay_ms(1);
    lan9646_read_reg32(&g_lan9646, 0x6504, &data);
    LOG_I(TAG, "P6 TX Broadcast: %lu", (unsigned long)data);

    /* TX Unicast */
    ctrl = (0x65 << 16) | 0x02000000;
    lan9646_write_reg32(&g_lan9646, 0x6500, ctrl);
    delay_ms(1);
    lan9646_read_reg32(&g_lan9646, 0x6504, &data);
    LOG_I(TAG, "P6 TX Unicast: %lu", (unsigned long)data);

    /* RX Broadcast */
    ctrl = (0x0A << 16) | 0x02000000;
    lan9646_write_reg32(&g_lan9646, 0x6500, ctrl);
    delay_ms(1);
    lan9646_read_reg32(&g_lan9646, 0x6504, &data);
    LOG_I(TAG, "P6 RX Broadcast: %lu", (unsigned long)data);

    /* RX Unicast */
    ctrl = (0x0C << 16) | 0x02000000;
    lan9646_write_reg32(&g_lan9646, 0x6500, ctrl);
    delay_ms(1);
    lan9646_read_reg32(&g_lan9646, 0x6504, &data);
    LOG_I(TAG, "P6 RX Unicast: %lu", (unsigned long)data);
}
static void debug_gmac_status(void)
{
    LOG_I(TAG, "=== GMAC Status ===");

    /* MAC config */
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    LOG_I(TAG, "MAC_CFG: 0x%08lX [TE=%d RE=%d]",
          (unsigned long)mac_cfg,
          (int)((mac_cfg >> 1) & 1),  /* TE - TX Enable */
          (int)((mac_cfg >> 0) & 1)); /* RE - RX Enable */

    /* DMA status */
    uint32_t dma_ch0_tx = IP_GMAC_0->DMA_CH0_TX_CONTROL;
    uint32_t dma_ch0_rx = IP_GMAC_0->DMA_CH0_RX_CONTROL;
    LOG_I(TAG, "DMA_TX: 0x%08lX [ST=%d]", (unsigned long)dma_ch0_tx, (int)(dma_ch0_tx & 1));
    LOG_I(TAG, "DMA_RX: 0x%08lX [SR=%d]", (unsigned long)dma_ch0_rx, (int)(dma_ch0_rx & 1));

    /* TX/RX counters */
    LOG_I(TAG, "TX Packets: %lu", (unsigned long)IP_GMAC_0->TX_PACKET_COUNT_GOOD_BAD);
    LOG_I(TAG, "RX Packets: %lu", (unsigned long)IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD);
    LOG_I(TAG, "RX CRC Err: %lu", (unsigned long)IP_GMAC_0->RX_CRC_ERROR_PACKETS);

    /* DMA running status */
    uint32_t dma_status = IP_GMAC_0->DMA_CH0_STATUS;
    LOG_I(TAG, "DMA Status: 0x%08lX [TPS=%lu RPS=%lu]",
          (unsigned long)dma_status,
          (unsigned long)((dma_status >> 12) & 0xF),  /* TX Process State */
          (unsigned long)((dma_status >> 8) & 0xF));  /* RX Process State */
}

static lan9646r_t configure_port6_rgmii_1g(void) {
    uint8_t ctrl0, ctrl1;

    LOG_I(TAG, "Configuring Port 6 for RGMII 1G...");

    /* XMII Control - THỬ ENABLE TX/RX DELAY */
    ctrl0 = 0x68;  /* Full duplex, 1G, TX/RX FC enabled */
    ctrl1 = 0x18;  /* TX_DLY=ON (bit3), RX_DLY=ON (bit4) */

    lan9646_write_reg8(&g_lan9646, 0x6300, ctrl0);
    lan9646_write_reg8(&g_lan9646, 0x6301, ctrl1);

    LOG_I(TAG, "XMII: CTRL0=0x%02X CTRL1=0x%02X (TX_DLY=ON RX_DLY=ON)", ctrl0, ctrl1);

    /* Disable VLAN */
    uint8_t lue_ctrl0;
    lan9646_read_reg8(&g_lan9646, 0x0310, &lue_ctrl0);
    lue_ctrl0 &= ~0x10;
    lan9646_write_reg8(&g_lan9646, 0x0310, lue_ctrl0);

    /* Enable Switch */
    lan9646_write_reg8(&g_lan9646, 0x0300, 0x01);

    /* Port membership */
    lan9646_write_reg32(&g_lan9646, 0x6A04, 0x4F);
    lan9646_write_reg32(&g_lan9646, 0x1A04, 0x6E);
    lan9646_write_reg32(&g_lan9646, 0x2A04, 0x6D);
    lan9646_write_reg32(&g_lan9646, 0x3A04, 0x6B);
    lan9646_write_reg32(&g_lan9646, 0x4A04, 0x67);

    /* MSTP State */
    for (int port = 1; port <= 4; port++) {
        uint16_t base = (uint16_t)port << 12;
        lan9646_write_reg8(&g_lan9646, base | 0x0B01, 0x00);
        lan9646_write_reg8(&g_lan9646, base | 0x0B04, 0x07);
    }

    lan9646_write_reg8(&g_lan9646, 0x6B01, 0x00);
    lan9646_write_reg8(&g_lan9646, 0x6B04, 0x07);

    /* Verify */
    delay_ms(10);
    debug_lan9646_detail();

    return lan9646OK;
}

static void lan9646_init_device(void) {
    uint16_t chip_id;
    uint8_t revision;

    lan9646_cfg_t cfg = {
        .if_type = LAN9646_IF_I2C,
        .i2c_addr = LAN9646_I2C_ADDR_DEFAULT,
        .ops.i2c = {
            .init_fn = i2c_init_cb,
            .write_fn = i2c_write_cb,
            .read_fn = i2c_read_cb,
            .mem_write_fn = i2c_mem_write_cb,
            .mem_read_fn = i2c_mem_read_cb
        }
    };

    LOG_I(TAG, "Initializing LAN9646...");
    if (lan9646_init(&g_lan9646, &cfg) != lan9646OK) {
        LOG_E(TAG, "LAN9646 init FAILED!");
        while(1);
    }

    if (lan9646_get_chip_id(&g_lan9646, &chip_id, &revision) != lan9646OK) {
        LOG_E(TAG, "Failed to read chip ID!");
        while(1);
    }
    LOG_I(TAG, "Chip: 0x%04X Rev:%d", chip_id, revision);

    delay_ms(100);
    configure_port6_rgmii_1g();
    delay_ms(500);

    LOG_I(TAG, "LAN9646 ready");
}

#if LWIP_NETIF_STATUS_CALLBACK
static void status_callback(struct netif *state_netif)
{
    if (netif_is_up(state_netif)) {
#if LWIP_IPV4
        LOG_I(TAG, "Network UP - IP: %s", ip4addr_ntoa(netif_ip4_addr(state_netif)));
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

            LOG_I(TAG, "Static IP: %d.%d.%d.%d",
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
            LOG_I(TAG, "IPv6 link-local created");
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
            LOG_I(TAG, "DHCP enabled");
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
                LOG_E(TAG, "DHCP failed: %d", err);
            }
        }
#endif

#if LWIP_AUTOIP
        else if (netif_cfg[i]->has_auto_ip) {
            err = autoip_start(&network_interfaces[i]);
            if (err == ERR_OK) {
                LOG_I(TAG, "AutoIP started");
            } else {
                LOG_E(TAG, "AutoIP failed: %d", err);
            }
        }
#endif
    }
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

    LOG_I(TAG, "IPERF: type=%d, remote=%s:%d, bytes=%lu, %lukbps",
          (int)report_type, ipaddr_ntoa(remote_addr), (int)remote_port,
          bytes_transferred, bandwidth_kbitpsec);
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

    LOG_I(TAG, "Setting GMAC to ACTIVE...");
    Std_ReturnType ret = Eth_43_GMAC_SetControllerMode(0, ETH_MODE_ACTIVE);
    LOG_I(TAG, "GMAC SetControllerMode: %d", ret);

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
    /* Debug GMAC ngay sau khi start */
    delay_ms(1000);
    debug_gmac_status();
    debug_lan9646_mib();
    debug_lan9646_detail();
    debug_rgmii_clocks();
    uint32_t last_print = 0;

    while (1) {
#if NO_SYS
        sys_check_timeouts();
#else
        sys_msleep(5000);
#endif

#if defined(USING_RTD)
        uint32 time_now = OsIf_GetMilliseconds() / 1000;
#else
        uint32_t time_now = OSIF_GetMilliseconds() / 1000;
#endif

        if (time_now - last_print >= 10) {
            last_print = time_now;
            LOG_I(TAG, "--- Stats at %lu sec ---", (unsigned long)time_now);
            LOG_I(TAG, "IP: %s", ip4addr_ntoa(netif_ip4_addr(&network_interfaces[0])));
#if LWIP_STATS
            LOG_I(TAG, "Link RX: %u, TX: %u", lwip_stats.link.recv, lwip_stats.link.xmit);
#endif
            /* Debug GMAC định kỳ */
         	debug_gmac_status();
         	debug_lan9646_mib();
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

void device_init(void)
{
    OsIf_Init(NULL_PTR);
    Port_Init(NULL_PTR);

    Mcu_Init(NULL_PTR);
    Mcu_InitClock(McuClockSettingConfig_0);

    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED){};
    Mcu_DistributePllClock();
    Mcu_SetMode(McuModeSettingConf_0);

    Platform_Init(NULL_PTR);

#ifndef USING_OS_FREERTOS
    Gpt_Init(NULL_PTR);
    Gpt_StartTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0, 40000000U);
    Gpt_EnableNotification(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);
    OsIf_SetTimerFrequency(160000000U, OSIF_USE_SYSTEM_TIMER);
#endif

    Uart_Init(NULL_PTR);
    log_init();

    /* === Check clock setup === */
    debug_gmac_clocks();

    LOG_I(TAG, "Setting DCM for RGMII...");

    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    dcmrwf3 |= (1U << 0);  /* RX_CLK_MUX_BYPASS */
    dcmrwf3 |= (1U << 3);  /* TX_CLK_OUT_EN */
    IP_DCM_GPR->DCMRWF3 = dcmrwf3;

    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    dcmrwf1 = (dcmrwf1 & ~0x7U) | 2U;
    IP_DCM_GPR->DCMRWF1 = dcmrwf1;

    lan9646_init_device();

    Eth_Init(NULL_PTR);

    LOG_I(TAG, "Fixing MAC registers...");

    uint32_t ext_cfg = IP_GMAC_0->MAC_EXT_CONFIGURATION;
    ext_cfg |= (1U << 12);
    IP_GMAC_0->MAC_EXT_CONFIGURATION = ext_cfg;

    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    mac_cfg &= ~(1U << 13);
    mac_cfg |= (1U << 15);
    mac_cfg |= (1U << 11);
    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;

    debug_rgmii_clocks();

    uint8_t mac[6];
    Eth_43_GMAC_GetPhysAddr(0, mac);
    LOG_I(TAG, "GMAC MAC: %02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
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

int main(void)
{
    device_init();
    start_example();
    return 0;
}

