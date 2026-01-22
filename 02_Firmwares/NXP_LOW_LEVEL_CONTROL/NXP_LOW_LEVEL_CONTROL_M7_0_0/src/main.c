/**
 * \file            main.c
 * \brief           LAN9646 + GMAC + lwIP - RGMII 100Mbps
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

//static void delay_ms(uint32_t ms) {
//    uint32_t start = OsIf_GetCounter(OSIF_COUNTER_DUMMY);
//    uint32_t ticks = OsIf_MicrosToTicks(ms * 1000, OSIF_COUNTER_DUMMY);
//    while ((OsIf_GetCounter(OSIF_COUNTER_DUMMY) - start) < ticks) {}
//}
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

static void debug_gmac_clocks(void) {
    LOG_I(TAG, "=== GMAC Clock Source ===");
    uint32_t mux8_css = IP_MC_CGM->MUX_8_CSS;
    uint32_t mux8_dc0 = IP_MC_CGM->MUX_8_DC_0;

    uint32_t source = (mux8_css >> 24) & 0x3F;
    uint32_t div_en = (mux8_dc0 >> 31) & 1;
    uint32_t div_val = (mux8_dc0 & 0xFF) + 1;

    LOG_I(TAG, "MUX_8_CSS: 0x%08lX (Source=%lu)", (unsigned long)mux8_css, (unsigned long)source);
    LOG_I(TAG, "MUX_8_DC_0: 0x%08lX (Enable=%lu, Divider=%lu)",
          (unsigned long)mux8_dc0, (unsigned long)div_en, (unsigned long)div_val);

    /* Tính tần số thực tế - giả sử source 12 = 125MHz */
    if (div_en) {
        LOG_I(TAG, "TX CLK = 125MHz / %lu = %lu MHz",
              (unsigned long)div_val, (unsigned long)(125 / div_val));
    }
}

static void debug_gmac_status(void) {
    LOG_I(TAG, "=== GMAC Status ===");
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    uint32_t dma_tx = IP_GMAC_0->DMA_CH0_TX_CONTROL;
    uint32_t dma_rx = IP_GMAC_0->DMA_CH0_RX_CONTROL;
    uint32_t dma_stat = IP_GMAC_0->DMA_DEBUG_STATUS0;

    LOG_I(TAG, "MAC_CFG: 0x%08lX [TE=%lu RE=%lu PS=%lu FES=%lu]",
          (unsigned long)mac_cfg,
          (mac_cfg >> 1) & 1, (mac_cfg >> 0) & 1,
          (mac_cfg >> 15) & 1, (mac_cfg >> 14) & 1);
    LOG_I(TAG, "DMA_TX: 0x%08lX [ST=%lu]", (unsigned long)dma_tx, dma_tx & 1);
    LOG_I(TAG, "DMA_RX: 0x%08lX [SR=%lu]", (unsigned long)dma_rx, dma_rx & 1);
    LOG_I(TAG, "TX Packets: %lu", IP_GMAC_0->TX_PACKET_COUNT_GOOD);
    LOG_I(TAG, "RX Packets: %lu", IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD);
    LOG_I(TAG, "RX CRC Err: %lu", IP_GMAC_0->RX_CRC_ERROR_PACKETS);
    LOG_I(TAG, "DMA Status: 0x%08lX", (unsigned long)dma_stat);
}

static void debug_lan9646_detail(void) {
    LOG_I(TAG, "=== LAN9646 Port 6 Detail ===");
    uint8_t xmii_ctrl0, xmii_ctrl1, port_status, mstp_state;
    uint32_t membership;

    lan9646_read_reg8(&g_lan9646, 0x6300, &xmii_ctrl0);
    lan9646_read_reg8(&g_lan9646, 0x6301, &xmii_ctrl1);
    lan9646_read_reg8(&g_lan9646, 0x6030, &port_status);
    lan9646_read_reg8(&g_lan9646, 0x6B04, &mstp_state);
    lan9646_read_reg32(&g_lan9646, 0x6A04, &membership);

    LOG_I(TAG, "XMII_CTRL0: 0x%02X", xmii_ctrl0);
    LOG_I(TAG, "XMII_CTRL1: 0x%02X [TX_DLY=%d RX_DLY=%d Speed1G=%d]",
          xmii_ctrl1, (xmii_ctrl1 >> 3) & 1, (xmii_ctrl1 >> 4) & 1, !(xmii_ctrl1 & 0x40));
    LOG_I(TAG, "PORT_STATUS: 0x%02X [Speed=%d Duplex=%d]",
          port_status, (port_status >> 3) & 3, (port_status >> 2) & 1);
    LOG_I(TAG, "MSTP_STATE: 0x%02X [TX=%d RX=%d]",
          mstp_state, (mstp_state >> 2) & 1, (mstp_state >> 1) & 1);
    LOG_I(TAG, "MEMBERSHIP: 0x%08lX", (unsigned long)membership);
}

/* MIB Counter indices - CHÍNH XÁC theo datasheet */
#define MIB_RX_BROADCAST        0x0A
#define MIB_RX_UNICAST          0x0C
#define MIB_TX_BROADCAST        0x18
#define MIB_TX_UNICAST          0x1A
static uint32_t read_mib_counter(uint8_t port, uint8_t index) {
    uint16_t base = (uint16_t)port << 12;
    uint32_t ctrl, data = 0;
    uint32_t timeout = 1000;

    /* Set MIB Index [23:16] and Read Enable [25] */
    ctrl = ((uint32_t)index << 16) | 0x02000000UL;
    lan9646_write_reg32(&g_lan9646, base | 0x0500, ctrl);

    /* Poll until bit 25 auto-clears */
    do {
        lan9646_read_reg32(&g_lan9646, base | 0x0500, &ctrl);
        if (--timeout == 0) break;
    } while (ctrl & 0x02000000UL);

    /* Read counter data from 0xN504 */
    lan9646_read_reg32(&g_lan9646, base | 0x0504, &data);
    return data;
}

static void debug_lan9646_mib(void) {
    LOG_I(TAG, "=== LAN9646 Port 6 MIB ===");

    uint32_t tx_bc = read_mib_counter(6, 0x03);
    uint32_t tx_uc = read_mib_counter(6, 0x05);
    uint32_t rx_bc = read_mib_counter(6, 0x22);
    uint32_t rx_uc = read_mib_counter(6, 0x24);

    LOG_I(TAG, "P6 TX Broadcast: %lu", (unsigned long)tx_bc);
    LOG_I(TAG, "P6 TX Unicast: %lu", (unsigned long)tx_uc);
    LOG_I(TAG, "P6 RX Broadcast: %lu", (unsigned long)rx_bc);
    LOG_I(TAG, "P6 RX Unicast: %lu", (unsigned long)rx_uc);
}

static void debug_rgmii_clocks(void) {
    LOG_I(TAG, "=== RGMII Clock Debug ===");
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;

    LOG_I(TAG, "DCMRWF1: 0x%08lX [MAC_CONF_SEL=%lu]",
          (unsigned long)dcmrwf1, dcmrwf1 & 0x7);
    LOG_I(TAG, "DCMRWF3: 0x%08lX", (unsigned long)dcmrwf3);
    LOG_I(TAG, "  RX_CLK_MUX_BYPASS: %lu", dcmrwf3 & 1);
    LOG_I(TAG, "  TX_CLK_OUT_EN: %lu", (dcmrwf3 >> 3) & 1);
}

/*===========================================================================*/
/*                          LAN9646 CONFIGURATION - 100Mbps                   */
/*===========================================================================*/
static void debug_lan9646_all_ports_mib(void) {
    LOG_I(TAG, "=== LAN9646 All Ports MIB ===");

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

static lan9646r_t configure_port6_rgmii_100m(void) {
    LOG_I(TAG, "Configuring Port 6 for RGMII 100M...");

    uint8_t ctrl0 = 0x78;
    uint8_t ctrl1 = 0x40;

    lan9646_write_reg8(&g_lan9646, 0x6300, ctrl0);
    lan9646_write_reg8(&g_lan9646, 0x6301, ctrl1);

    LOG_I(TAG, "XMII: CTRL0=0x%02X CTRL1=0x%02X", ctrl0, ctrl1);
    LOG_I(TAG, "  TX_DLY=%d, RX_DLY=%d", (ctrl1 >> 3) & 1, (ctrl1 >> 4) & 1);


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

    /* MSTP State - Enable TX/RX */
    for (int port = 1; port <= 4; port++) {
        uint16_t base = (uint16_t)port << 12;
        lan9646_write_reg8(&g_lan9646, base | 0x0B01, 0x00);
        lan9646_write_reg8(&g_lan9646, base | 0x0B04, 0x07);
    }
    lan9646_write_reg8(&g_lan9646, 0x6B01, 0x00);
    lan9646_write_reg8(&g_lan9646, 0x6B04, 0x07);

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
    configure_port6_rgmii_100m();
    delay_ms(100);
    debug_lan9646_detail();

    LOG_I(TAG, "LAN9646 ready (100Mbps)");
}

/*===========================================================================*/
/*                          GMAC CONFIGURATION - 100Mbps                      */
/*===========================================================================*/

static void configure_gmac_100m(void) {
    LOG_I(TAG, "Configuring GMAC for 100Mbps...");

    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    mac_cfg &= ~((1U << 15) | (1U << 14) | (1U << 13));
    mac_cfg |= (1U << 15);  /* PS = 1 */
    mac_cfg |= (1U << 14);  /* FES = 1 */
    mac_cfg |= (1U << 13);  /* DM = 1 */
    mac_cfg |= (1U << 11);  /* ECRSFD */
    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;

    uint32_t ext_cfg = IP_GMAC_0->MAC_EXT_CONFIGURATION;
    ext_cfg |= (1U << 12);
    IP_GMAC_0->MAC_EXT_CONFIGURATION = ext_cfg;

    LOG_I(TAG, "MAC_CFG: 0x%08lX", (unsigned long)IP_GMAC_0->MAC_CONFIGURATION);
}

static void configure_gmac_tx_clock_100m(void) {
    LOG_I(TAG, "Configuring GMAC TX Clock for 25MHz...");

    /* Disable trước khi thay đổi */
    IP_MC_CGM->MUX_8_DC_0 = 0x00000000;
    for (volatile int i = 0; i < 1000; i++);

    /* 125MHz / 5 = 25MHz → divider value = 4 */
    IP_MC_CGM->MUX_8_DC_0 = 0x80000004;

    LOG_I(TAG, "MUX_8_DC_0: 0x%08lX", (unsigned long)IP_MC_CGM->MUX_8_DC_0);
}

static void configure_gmac_rgmii_delay(void) {
    LOG_I(TAG, "Configuring GMAC RGMII...");

    /* DCMRWF1: Set MAC_CONF_SEL = 2 (RGMII) */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    dcmrwf1 = (dcmrwf1 & ~0x7U) | 2U;
    IP_DCM_GPR->DCMRWF1 = dcmrwf1;

    /* DCMRWF3: Chỉ cần bypass và output enable, KHÔNG có delay bits */
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    dcmrwf3 |= (1U << 0);   /* RX_CLK_MUX_BYPASS = 1 */
    dcmrwf3 |= (1U << 3);   /* TX_CLK_OUT_EN = 1 */
    /* Bit 1,2 là termination, không phải delay */
    IP_DCM_GPR->DCMRWF3 = dcmrwf3;

    LOG_I(TAG, "DCMRWF1: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF1);
    LOG_I(TAG, "DCMRWF3: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF3);
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
        LOG_I(TAG, "IPv6 link-local created");
#endif

#if LWIP_NETIF_STATUS_CALLBACK
        netif_set_status_callback(&network_interfaces[i], status_callback);
#endif
#if LWIP_NETIF_LINK_CALLBACK
        netif_set_link_callback(&network_interfaces[i], link_callback);
#endif

        netif_set_up(&network_interfaces[i]);
        LOG_I(TAG, "Interface %d: UP", i);

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

static void mainLoopTask(void* pvParameters) {
    (void)pvParameters;
    LOG_I(TAG, "mainLoopTask started");

#if !NO_SYS
    err_t err;
    sys_sem_t init_sem;
    err = sys_sem_new(&init_sem, 0);
    LWIP_ASSERT("failed to create init_sem", err == ERR_OK);
    (void)err;

    LOG_I(TAG, "Initializing TCP/IP stack...");
    tcpip_init(test_init, (void*)&init_sem);
    sys_sem_wait(&init_sem);
    sys_sem_free(&init_sem);
#else
    sys_init();
    lwip_init();
    test_init(NULL);
#endif

    LOG_I(TAG, "Entering main loop...");

    delay_ms(1000);
    debug_gmac_status();
    debug_lan9646_all_ports_mib();
    debug_lan9646_detail();
    debug_rgmii_clocks();

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
            debug_gmac_status();
            debug_lan9646_all_ports_mib();
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
    LOG_I(TAG, "  lwIP + LAN9646 + GMAC (100Mbps)");
    LOG_I(TAG, "========================================");

#if defined(USING_OS_FREERTOS)
    xTaskCreate(mainLoopTask, "mainloop", 512U, NULL, tskIDLE_PRIORITY + 1, NULL);
    LOG_I(TAG, "Starting FreeRTOS scheduler...");
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

    debug_gmac_clocks();

    LOG_I(TAG, "Setting DCM for RGMII...");
    configure_gmac_rgmii_delay();  // <-- GỌI Ở ĐÂY

    lan9646_init_device();
    configure_gmac_tx_clock_100m();

    Eth_Init(NULL_PTR);
    configure_gmac_100m();

    debug_rgmii_clocks();

    uint8_t mac[6];
    Eth_43_GMAC_GetPhysAddr(0, mac);
    LOG_I(TAG, "GMAC MAC: %02X:%02X:%02X:%02X:%02X:%02X",
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
