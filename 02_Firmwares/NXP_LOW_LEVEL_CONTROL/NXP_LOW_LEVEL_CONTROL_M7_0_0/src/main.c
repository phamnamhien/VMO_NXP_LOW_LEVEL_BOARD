/**
 * \file            main.c
 * \brief           LAN9646 + GMAC + lwIP - RGMII 100Mbps
 * \note            RGMII Delay: LAN9646 TX_DLY=ON, RX_DLY=ON -> S32K388 no internal delay
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
#include "Eth_43_GMAC.h"

#include "lan9646.h"
#include "s32k3xx_soft_i2c.h"
#include "log_debug.h"

#if defined(USING_OS_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#endif

#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/etharp.h"
#include "netif/ethernet.h"
#include "netifcfg.h"
#include "lwipcfg.h"

#if LWIP_HTTPD_APP
#include "lwip/apps/httpd.h"
#endif
#if LWIP_LWIPERF_APP
#include "lwip/apps/lwiperf.h"
#endif
#if LWIP_TCPECHO_APP
#include "apps/tcpecho_raw/tcpecho_raw.h"
#endif
#if LWIP_UDPECHO_APP
#include "apps/udpecho_raw/udpecho_raw.h"
#endif

#include "ethif_port.h"

#define TAG "MAIN"

#define LAN9646_SCL_CHANNEL     DioConf_DioChannel_SCL_CH
#define LAN9646_SDA_CHANNEL     DioConf_DioChannel_SDA_CH
#define LAN9646_I2C_SPEED       5U

#define ETH_CTRL_IDX            0U

/*===========================================================================*/
/*                     RGMII DELAY CONFIGURATION                              */
/*===========================================================================*/
/*
 * RGMII requires ~2ns delay on TXC and RXC for proper data sampling.
 * The delay can be added by either side, but NOT both.
 *
 * Configuration options:
 *   Option A: LAN9646 adds delay (recommended)
 *     - LAN9646: TX_DLY=1, RX_DLY=1
 *     - S32K388: No internal delay
 *
 *   Option B: S32K388 adds delay
 *     - LAN9646: TX_DLY=0, RX_DLY=0
 *     - S32K388: Internal delay enabled
 *
 * Current: Option A
 */
#define RGMII_DELAY_OPTION_A    1   /* LAN9646 adds delay */
#define RGMII_DELAY_OPTION_B    0   /* S32K388 adds delay */

/*===========================================================================*/
/*                          GLOBAL VARIABLES                                  */
/*===========================================================================*/

static lan9646_t g_lan9646;
static softi2c_t g_i2c;
struct netif network_interfaces[ETHIF_NUMBER];

#if LWIP_DHCP
struct dhcp netif_dhcp;
#endif

uint32 start_time = 0;
uint32 tests_timeout = 1200;

extern void sys_init(void);

/*===========================================================================*/
/*                          DELAY FUNCTION                                    */
/*===========================================================================*/

static void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 16000; i++) {
        __asm("nop");
    }
}

/*===========================================================================*/
/*                          I2C CALLBACKS                                     */
/*===========================================================================*/

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

/*===========================================================================*/
/*                          DEBUG FUNCTIONS                                   */
/*===========================================================================*/

static void debug_rgmii_config(void) {
    LOG_I(TAG, "=== RGMII Configuration ===");

    /* LAN9646 Side */
    uint8_t ctrl0, ctrl1, port_status;
    lan9646_read_reg8(&g_lan9646, 0x6300, &ctrl0);
    lan9646_read_reg8(&g_lan9646, 0x6301, &ctrl1);
    lan9646_read_reg8(&g_lan9646, 0x6030, &port_status);

    LOG_I(TAG, "[LAN9646 Port 6]");
    LOG_I(TAG, "  XMII_CTRL0=0x%02X XMII_CTRL1=0x%02X", ctrl0, ctrl1);
    LOG_I(TAG, "  Speed: %s", (ctrl1 & 0x40) ? "100M" : "1000M");
    LOG_I(TAG, "  Duplex: %s", (ctrl0 & 0x40) ? "Full" : "Half");
    LOG_I(TAG, "  TX_DLY: %s", (ctrl1 & 0x08) ? "ON (+1.5ns)" : "OFF");
    LOG_I(TAG, "  RX_DLY: %s", (ctrl1 & 0x10) ? "ON (+1.5ns)" : "OFF");
    LOG_I(TAG, "  PORT_STATUS=0x%02X", port_status);

    /* S32K388 Side */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;

    LOG_I(TAG, "[S32K388 GMAC]");
    LOG_I(TAG, "  DCMRWF1=0x%08lX (MAC_CONF_SEL=%lu)",
          (unsigned long)dcmrwf1, dcmrwf1 & 0x7);
    LOG_I(TAG, "  DCMRWF3=0x%08lX", (unsigned long)dcmrwf3);
    LOG_I(TAG, "    RX_CLK_MUX_BYPASS=%lu", dcmrwf3 & 1);
    LOG_I(TAG, "    TX_CLK_OUT_EN=%lu", (dcmrwf3 >> 3) & 1);
    LOG_I(TAG, "  MAC_CFG=0x%08lX", (unsigned long)mac_cfg);
    LOG_I(TAG, "    PS=%lu FES=%lu DM=%lu",
          (mac_cfg >> 15) & 1, (mac_cfg >> 14) & 1, (mac_cfg >> 13) & 1);

    /* Summary */
    LOG_I(TAG, "[Delay Summary]");
#if RGMII_DELAY_OPTION_A
    LOG_I(TAG, "  Mode: LAN9646 adds delay");
    LOG_I(TAG, "  Expected: LAN9646 TX/RX_DLY=ON, S32K no delay");
#else
    LOG_I(TAG, "  Mode: S32K388 adds delay");
    LOG_I(TAG, "  Expected: LAN9646 TX/RX_DLY=OFF, S32K delay ON");
#endif
}

static void debug_eth_stats(void) {
    LOG_I(TAG, "=== ETH Statistics ===");

    /* GMAC counters */
    LOG_I(TAG, "GMAC TX Good: %lu", (unsigned long)IP_GMAC_0->TX_PACKET_COUNT_GOOD);
    LOG_I(TAG, "GMAC RX Good: %lu", (unsigned long)IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD);
    LOG_I(TAG, "GMAC RX CRC Err: %lu", (unsigned long)IP_GMAC_0->RX_CRC_ERROR_PACKETS);
    LOG_I(TAG, "GMAC RX Align Err: %lu", (unsigned long)IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS);
    LOG_I(TAG, "GMAC RX Runt: %lu", (unsigned long)IP_GMAC_0->RX_RUNT_ERROR_PACKETS);
    LOG_I(TAG, "GMAC RX Jabber: %lu", (unsigned long)IP_GMAC_0->RX_JABBER_ERROR_PACKETS);
}

/* MIB Counter indices */
#define MIB_RX_BROADCAST        0x0A
#define MIB_RX_UNICAST          0x0C
#define MIB_TX_BROADCAST        0x18
#define MIB_TX_UNICAST          0x1A

static uint32_t read_mib_counter(uint8_t port, uint8_t index) {
    uint16_t base = (uint16_t)port << 12;
    uint32_t ctrl, data = 0;
    uint32_t timeout = 1000;

    ctrl = ((uint32_t)index << 16) | 0x02000000UL;
    lan9646_write_reg32(&g_lan9646, base | 0x0500, ctrl);

    do {
        lan9646_read_reg32(&g_lan9646, base | 0x0500, &ctrl);
        if (--timeout == 0) break;
    } while (ctrl & 0x02000000UL);

    lan9646_read_reg32(&g_lan9646, base | 0x0504, &data);
    return data;
}

static void debug_lan9646_mib(void) {
    LOG_I(TAG, "=== LAN9646 MIB ===");

    for (int port = 1; port <= 4; port++) {
        uint32_t tx_uc = read_mib_counter(port, MIB_TX_UNICAST);
        uint32_t tx_bc = read_mib_counter(port, MIB_TX_BROADCAST);
        uint32_t rx_uc = read_mib_counter(port, MIB_RX_UNICAST);
        uint32_t rx_bc = read_mib_counter(port, MIB_RX_BROADCAST);
        LOG_I(TAG, "Port %d: TX(uc=%lu bc=%lu) RX(uc=%lu bc=%lu)",
              port, (unsigned long)tx_uc, (unsigned long)tx_bc,
              (unsigned long)rx_uc, (unsigned long)rx_bc);
    }

    uint32_t tx6_uc = read_mib_counter(6, MIB_TX_UNICAST);
    uint32_t tx6_bc = read_mib_counter(6, MIB_TX_BROADCAST);
    uint32_t rx6_uc = read_mib_counter(6, MIB_RX_UNICAST);
    uint32_t rx6_bc = read_mib_counter(6, MIB_RX_BROADCAST);
    LOG_I(TAG, "Port 6: TX(uc=%lu bc=%lu) RX(uc=%lu bc=%lu)",
          (unsigned long)tx6_uc, (unsigned long)tx6_bc,
          (unsigned long)rx6_uc, (unsigned long)rx6_bc);
}

/*===========================================================================*/
/*                    LAN9646 RGMII CONFIGURATION                             */
/*===========================================================================*/

static lan9646r_t configure_lan9646_port6_rgmii(void) {
    LOG_I(TAG, "Configuring LAN9646 Port 6 for RGMII 100Mbps...");

    /*
     * XMII_CTRL0 (0x6300):
     *   Bit 6: Duplex (1=Full)
     *   Bit 5: TX Flow Control
     *   Bit 4: Speed 10/100 (1=100M when not 1G mode)
     *   Bit 3: RX Flow Control
     */
    uint8_t ctrl0 = 0x78;  /* Full duplex, Flow control, 100M */

    /*
     * XMII_CTRL1 (0x6301):
     *   Bit 6: Speed 1G select (0=1G, 1=10/100)
     *   Bit 4: RX Ingress Delay (+1.5ns)
     *   Bit 3: TX Egress Delay (+1.5ns)
     */
#if RGMII_DELAY_OPTION_A
    /* LAN9646 adds delay: TX_DLY=1, RX_DLY=1 */
    uint8_t ctrl1 = 0x40 | 0x18;  /* 100M mode + TX_DLY + RX_DLY */
    LOG_I(TAG, "  Delay Mode: LAN9646 adds delay (TX_DLY=1, RX_DLY=1)");
#else
    /* S32K388 adds delay: TX_DLY=0, RX_DLY=0 */
    uint8_t ctrl1 = 0x40;  /* 100M mode only, no delay */
    LOG_I(TAG, "  Delay Mode: S32K388 adds delay (TX_DLY=0, RX_DLY=0)");
#endif

    lan9646_write_reg8(&g_lan9646, 0x6300, ctrl0);
    lan9646_write_reg8(&g_lan9646, 0x6301, ctrl1);

    LOG_I(TAG, "  XMII_CTRL0=0x%02X XMII_CTRL1=0x%02X", ctrl0, ctrl1);

    /* Disable VLAN filtering */
    uint8_t lue_ctrl0;
    lan9646_read_reg8(&g_lan9646, 0x0310, &lue_ctrl0);
    lue_ctrl0 &= ~0x10;
    lan9646_write_reg8(&g_lan9646, 0x0310, lue_ctrl0);

    /* Enable Switch */
    lan9646_write_reg8(&g_lan9646, 0x0300, 0x01);

    /* Port membership: Port 6 can forward to ports 1-4, and vice versa */
    lan9646_write_reg32(&g_lan9646, 0x6A04, 0x4F);  /* Port 6 -> 1,2,3,4,6,7 */
    lan9646_write_reg32(&g_lan9646, 0x1A04, 0x6E);  /* Port 1 -> 2,3,4,6,7 */
    lan9646_write_reg32(&g_lan9646, 0x2A04, 0x6D);  /* Port 2 -> 1,3,4,6,7 */
    lan9646_write_reg32(&g_lan9646, 0x3A04, 0x6B);  /* Port 3 -> 1,2,4,6,7 */
    lan9646_write_reg32(&g_lan9646, 0x4A04, 0x67);  /* Port 4 -> 1,2,3,6,7 */

    /* MSTP State - Enable TX/RX for all ports */
    for (int port = 1; port <= 4; port++) {
        uint16_t base = (uint16_t)port << 12;
        lan9646_write_reg8(&g_lan9646, base | 0x0B01, 0x00);
        lan9646_write_reg8(&g_lan9646, base | 0x0B04, 0x07);  /* TX+RX+Learning */
    }
    lan9646_write_reg8(&g_lan9646, 0x6B01, 0x00);
    lan9646_write_reg8(&g_lan9646, 0x6B04, 0x07);

    LOG_I(TAG, "LAN9646 Port 6 configured");
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
    configure_lan9646_port6_rgmii();
    delay_ms(100);

    LOG_I(TAG, "LAN9646 ready");
}

/*===========================================================================*/
/*                    S32K388 GMAC RGMII CONFIGURATION                        */
/*===========================================================================*/

static void configure_gmac_rgmii(void) {
    LOG_I(TAG, "Configuring S32K388 GMAC for RGMII 100Mbps...");

    /*
     * DCMRWF1: MAC Configuration Select
     *   Bits [2:0] MAC_CONF_SEL:
     *     0 = MII
     *     1 = RMII
     *     2 = RGMII
     */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    dcmrwf1 = (dcmrwf1 & ~0x7U) | 2U;  /* RGMII mode */
    IP_DCM_GPR->DCMRWF1 = dcmrwf1;
    LOG_I(TAG, "  DCMRWF1=0x%08lX (RGMII mode)", (unsigned long)IP_DCM_GPR->DCMRWF1);

    /*
     * DCMRWF3: RGMII Clock Configuration
     *   Bit 0: GMAC_RX_CLK_MUX_BYPASS (1=use external RX_CLK directly)
     *   Bit 1: GMAC_RX_CLK_RES_EN (termination)
     *   Bit 2: GMAC_TX_CLK_RES_EN (termination)
     *   Bit 3: GMAC_TX_CLK_OUT_EN (1=enable TX_CLK output)
     *
     * Note: S32K388 does NOT have internal RGMII delay registers.
     *       Delay must be provided by PHY/Switch side.
     */
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;

#if RGMII_DELAY_OPTION_A
    /* LAN9646 adds delay -> S32K388 no special config needed */
    dcmrwf3 |= (1U << 0);   /* RX_CLK_MUX_BYPASS = 1 (use external clock) */
    dcmrwf3 |= (1U << 3);   /* TX_CLK_OUT_EN = 1 */
    LOG_I(TAG, "  Delay Mode: LAN9646 provides delay");
#else
    /* S32K388 should add delay - but hardware doesn't support it! */
    /* In this case, may need external delay line or adjust LAN9646 */
    dcmrwf3 |= (1U << 0);
    dcmrwf3 |= (1U << 3);
    LOG_W(TAG, "  WARNING: S32K388 has no internal RGMII delay!");
    LOG_W(TAG, "  Consider using LAN9646 delay instead (Option A)");
#endif

    IP_DCM_GPR->DCMRWF3 = dcmrwf3;
    LOG_I(TAG, "  DCMRWF3=0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF3);
}

static void configure_gmac_clock_100m(void) {
    LOG_I(TAG, "Configuring GMAC TX Clock for 100Mbps (25MHz)...");

    /* Disable before changing */
    IP_MC_CGM->MUX_8_DC_0 = 0x00000000;
    for (volatile int i = 0; i < 1000; i++);

    /*
     * MUX_8 is for GMAC0_TX_CLK
     * Source is typically 125MHz from PLL
     * For 100Mbps RGMII: need 25MHz -> divide by 5 (divider value = 4)
     * For 1000Mbps RGMII: need 125MHz -> divide by 1 (divider value = 0)
     */
    IP_MC_CGM->MUX_8_DC_0 = 0x80000004;  /* Enable + div by 5 */

    LOG_I(TAG, "  MUX_8_DC_0=0x%08lX (25MHz)", (unsigned long)IP_MC_CGM->MUX_8_DC_0);
}

static void configure_gmac_mac(void) {
    LOG_I(TAG, "Configuring GMAC MAC for 100Mbps Full Duplex...");

    /*
     * MAC_CONFIGURATION register:
     *   Bit 15 (PS): Port Select (1=MII/RMII, 0=GMII/RGMII for 1G)
     *                For 10/100: PS=1
     *   Bit 14 (FES): Fast Ethernet Speed (1=100Mbps, 0=10Mbps) when PS=1
     *   Bit 13 (DM): Duplex Mode (1=Full, 0=Half)
     *   Bit 11 (ECRSFD): Enable Carrier Sense Before TX in Full-Duplex
     *   Bit 1 (TE): Transmitter Enable
     *   Bit 0 (RE): Receiver Enable
     *
     * For 100Mbps Full Duplex: PS=1, FES=1, DM=1
     */
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;

    /* Clear speed/duplex bits first */
    mac_cfg &= ~((1U << 15) | (1U << 14) | (1U << 13));

    /* Set for 100Mbps Full Duplex */
    mac_cfg |= (1U << 15);  /* PS = 1 (10/100 mode) */
    mac_cfg |= (1U << 14);  /* FES = 1 (100Mbps) */
    mac_cfg |= (1U << 13);  /* DM = 1 (Full Duplex) */
    mac_cfg |= (1U << 11);  /* ECRSFD */

    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;

    LOG_I(TAG, "  MAC_CFG=0x%08lX [PS=%lu FES=%lu DM=%lu]",
          (unsigned long)mac_cfg,
          (mac_cfg >> 15) & 1, (mac_cfg >> 14) & 1, (mac_cfg >> 13) & 1);

    /* MAC_EXT_CONFIGURATION: Enable extended status */
    uint32_t ext_cfg = IP_GMAC_0->MAC_EXT_CONFIGURATION;
    ext_cfg |= (1U << 12);  /* Enable extended status */
    IP_GMAC_0->MAC_EXT_CONFIGURATION = ext_cfg;
}

/*===========================================================================*/
/*                          NETWORK INTERFACE                                 */
/*===========================================================================*/

#if LWIP_NETIF_STATUS_CALLBACK
static void status_callback(struct netif *state_netif) {
    if (netif_is_up(state_netif)) {
        LOG_I(TAG, "Network UP - IP: %s", ip4addr_ntoa(netif_ip4_addr(state_netif)));
    } else {
        LOG_W(TAG, "Network DOWN");
    }
}
#endif

#if LWIP_NETIF_LINK_CALLBACK
static void link_callback(struct netif *state_netif) {
    LOG_I(TAG, "Link %s", netif_is_link_up(state_netif) ? "UP" : "DOWN");
}
#endif

static void interface_init(void) {
    LOG_I(TAG, "Initializing network interfaces...");

    for (int i = 0; i < ETHIF_NUMBER; i++) {
        ip4_addr_t ipaddr, netmask, gw;

        ip4_addr_set_zero(&gw);
        ip4_addr_set_zero(&ipaddr);
        ip4_addr_set_zero(&netmask);

        if ((!netif_cfg[i]->has_dhcp) && (!netif_cfg[i]->has_auto_ip)) {
            IP4_ADDR(&gw, netif_cfg[i]->gw[0], netif_cfg[i]->gw[1],
                     netif_cfg[i]->gw[2], netif_cfg[i]->gw[3]);
            IP4_ADDR(&ipaddr, netif_cfg[i]->ip_addr[0], netif_cfg[i]->ip_addr[1],
                     netif_cfg[i]->ip_addr[2], netif_cfg[i]->ip_addr[3]);
            IP4_ADDR(&netmask, netif_cfg[i]->netmask[0], netif_cfg[i]->netmask[1],
                     netif_cfg[i]->netmask[2], netif_cfg[i]->netmask[3]);

            LOG_I(TAG, "Static IP: %d.%d.%d.%d",
                  netif_cfg[i]->ip_addr[0], netif_cfg[i]->ip_addr[1],
                  netif_cfg[i]->ip_addr[2], netif_cfg[i]->ip_addr[3]);
        }

#if NO_SYS
        netif_set_default(netif_add(&network_interfaces[i], &ipaddr, &netmask,
                                    &gw, NULL, ETHIF_INIT, netif_input));
#else
        netif_set_default(netif_add(&network_interfaces[i], &ipaddr, &netmask,
                                    &gw, NULL, ETHIF_INIT, tcpip_input));
#endif

#if LWIP_IPV6
        netif_create_ip6_linklocal_address(&network_interfaces[i], 1);
#endif

#if LWIP_NETIF_STATUS_CALLBACK
        netif_set_status_callback(&network_interfaces[i], status_callback);
#endif
#if LWIP_NETIF_LINK_CALLBACK
        netif_set_link_callback(&network_interfaces[i], link_callback);
#endif

        netif_set_up(&network_interfaces[i]);

#if LWIP_DHCP
        if (netif_cfg[i]->has_dhcp) {
            dhcp_start(&network_interfaces[i]);
            LOG_I(TAG, "DHCP started");
        }
#endif
    }
}

/*===========================================================================*/
/*                          APPLICATIONS                                      */
/*===========================================================================*/

static void apps_init(void) {
    LOG_I(TAG, "Initializing applications...");

#if LWIP_HTTPD_APP && LWIP_TCP
    httpd_init();
    LOG_I(TAG, "HTTP server initialized");
#endif

#if LWIP_TCPECHO_APP
    tcpecho_raw_init();
    LOG_I(TAG, "TCP Echo initialized");
#endif

#if LWIP_UDPECHO_APP
    udpecho_raw_init();
    LOG_I(TAG, "UDP Echo initialized");
#endif

#if LWIP_LWIPERF_APP
    lwiperf_start_tcp_server_default(NULL, NULL);
    LOG_I(TAG, "IPERF server initialized");
#endif
}

/*===========================================================================*/
/*                          MAIN TASK                                         */
/*===========================================================================*/

static void test_init(void* arg) {
#if !NO_SYS
    sys_sem_t* init_sem = (sys_sem_t*)arg;
#else
    (void)arg;
#endif

    start_time = OsIf_GetMilliseconds() / 1000;

    interface_init();

    LOG_I(TAG, "Setting ETH controller to ACTIVE...");
    Std_ReturnType ret = Eth_43_GMAC_SetControllerMode(ETH_CTRL_IDX, ETH_MODE_ACTIVE);
    if (ret == E_OK) {
        LOG_I(TAG, "ETH controller ACTIVE");
    } else {
        LOG_E(TAG, "ETH controller activation FAILED!");
    }

    apps_init();

#if !NO_SYS
    sys_sem_signal(init_sem);
#endif
}

static void mainLoopTask(void* pvParameters) {
    (void)pvParameters;

#if !NO_SYS
    err_t err;
    sys_sem_t init_sem;
    err = sys_sem_new(&init_sem, 0);
    LWIP_ASSERT("failed to create init_sem", err == ERR_OK);
    (void)err;

    tcpip_init(test_init, (void*)&init_sem);
    sys_sem_wait(&init_sem);
    sys_sem_free(&init_sem);
#else
    sys_init();
    lwip_init();
    test_init(NULL);
#endif

    LOG_I(TAG, "Entering main loop...");

    /* Initial debug output */
    delay_ms(1000);
    debug_rgmii_config();
    debug_eth_stats();
    debug_lan9646_mib();

    uint32_t last_print = 0;

    while (1) {
#if NO_SYS
        sys_check_timeouts();
#else
        sys_msleep(5000);
#endif

        uint32_t time_now = OsIf_GetMilliseconds() / 1000;

        if (time_now - last_print >= 10) {
            last_print = time_now;
            LOG_I(TAG, "--- Stats at %lu sec ---", (unsigned long)time_now);
            LOG_I(TAG, "IP: %s", ip4addr_ntoa(netif_ip4_addr(&network_interfaces[0])));
            debug_eth_stats();
            debug_lan9646_mib();
        }

        if (time_now - start_time >= tests_timeout) {
            LOG_W(TAG, "Test timeout");
            break;
        }
    }
}

static void start_example(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "  lwIP + LAN9646 + GMAC (RGMII 100M)");
    LOG_I(TAG, "========================================");

#if defined(USING_OS_FREERTOS)
    xTaskCreate(mainLoopTask, "mainloop", 512U, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();
    for (;;) {}
#else
    mainLoopTask(NULL);
#endif
}

/*===========================================================================*/
/*                          DEVICE INIT                                       */
/*===========================================================================*/

static void device_init(void) {
    OsIf_Init(NULL_PTR);
    Port_Init(NULL_PTR);

    Mcu_Init(NULL_PTR);
    Mcu_InitClock(McuClockSettingConfig_0);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {}
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

    LOG_I(TAG, "=== Device Initialization ===");

    /* Configure S32K388 RGMII before LAN9646 */
    configure_gmac_rgmii();
    configure_gmac_clock_100m();

    /* Initialize LAN9646 switch */
    lan9646_init_device();

    /* Initialize Ethernet driver (AUTOSAR) */
    LOG_I(TAG, "Initializing Ethernet (AUTOSAR)...");
    Eth_Init(NULL_PTR);

    /* Configure GMAC MAC after Eth_Init */
    configure_gmac_mac();

    /* Print MAC address */
    uint8_t mac[6];
    Eth_43_GMAC_GetPhysAddr(ETH_CTRL_IDX, mac);
    LOG_I(TAG, "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/*===========================================================================*/
/*                          FREERTOS HOOKS                                    */
/*===========================================================================*/

#if defined(USING_OS_FREERTOS)
void vAssertCalled(uint32_t ulLine, const char* const pcFileName) {
    LOG_E(TAG, "ASSERT! Line %lu, file %s", ulLine, pcFileName);
    taskENTER_CRITICAL();
    while (1) {}
    taskEXIT_CRITICAL();
}

void vApplicationMallocFailedHook(void) {
    LOG_E(TAG, "Malloc failed!");
    vAssertCalled(__LINE__, __FILE__);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName) {
    (void)pxTask;
    LOG_E(TAG, "Stack overflow: %s", pcTaskName);
    vAssertCalled(__LINE__, __FILE__);
}

void vMainConfigureTimerForRunTimeStats(void) {}
uint32_t ulMainGetRunTimeCounterValue(void) { return 0UL; }
#endif

/*===========================================================================*/
/*                          MAIN                                              */
/*===========================================================================*/

int main(void) {
    device_init();
    start_example();
    return 0;
}
