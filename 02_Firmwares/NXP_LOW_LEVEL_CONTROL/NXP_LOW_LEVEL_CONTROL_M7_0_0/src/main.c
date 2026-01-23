/**
 * \file            main.c
 * \brief           LAN9646 + GMAC + lwIP - RGMII 100Mbps - FULL DEBUG VERSION
 * \note            Debug mode để khoanh vùng lỗi phần cứng/phần mềm
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
 * RGMII delay options - thử từng option để tìm cấu hình đúng
 *
 * Option 0: No delay (PCB trace đã có delay)
 * Option 1: LAN9646 TX delay only
 * Option 2: LAN9646 RX delay only
 * Option 3: LAN9646 both TX+RX delay
 */
#define RGMII_DELAY_OPTION      3   /* Thay đổi từ 0-3 để test */

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

/* Error counters for tracking */
static uint32_t g_prev_rx_good = 0;
static uint32_t g_prev_rx_crc = 0;
static uint32_t g_prev_rx_align = 0;
static uint32_t g_prev_tx_good = 0;

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
/*                    DEBUG: S32K388 GMAC REGISTERS                           */
/*===========================================================================*/

static void debug_s32k388_clocks(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "╔══════════════════════════════════════════════════════════╗");
    LOG_I(TAG, "║           S32K388 CLOCK CONFIGURATION                    ║");
    LOG_I(TAG, "╚══════════════════════════════════════════════════════════╝");

    /* MUX_8: GMAC TX Clock */
    uint32_t mux8_csc = IP_MC_CGM->MUX_8_CSC;
    uint32_t mux8_css = IP_MC_CGM->MUX_8_CSS;
    uint32_t mux8_dc0 = IP_MC_CGM->MUX_8_DC_0;

    LOG_I(TAG, "MUX_8 (GMAC0_TX_CLK):");
    LOG_I(TAG, "  CSC=0x%08lX CSS=0x%08lX", (unsigned long)mux8_csc, (unsigned long)mux8_css);
    LOG_I(TAG, "  DC_0=0x%08lX", (unsigned long)mux8_dc0);

    uint32_t src = (mux8_css >> 24) & 0x3F;
    uint32_t div_en = (mux8_dc0 >> 31) & 1;
    uint32_t div_val = (mux8_dc0 & 0xFF) + 1;

    const char* src_name;
    switch (src) {
        case 0:  src_name = "FIRC"; break;
        case 8:  src_name = "FXOSC"; break;
        case 12: src_name = "PLL_PHI0"; break;
        case 14: src_name = "PLLAUX_PHI0"; break;
        case 18: src_name = "PLL_PHI1"; break;
        default: src_name = "UNKNOWN"; break;
    }

    LOG_I(TAG, "  Source: %s (sel=%lu)", src_name, (unsigned long)src);
    LOG_I(TAG, "  Divider: %s, value=%lu", div_en ? "ENABLED" : "DISABLED", (unsigned long)div_val);

    /* Estimate output frequency (assuming 200MHz PLL) */
    if (div_en && src == 14) {
        LOG_I(TAG, "  Output: ~%lu MHz (assuming 200MHz PLLAUX)",
              (unsigned long)(200 / div_val));
    }
}

static void debug_s32k388_dcm(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "╔══════════════════════════════════════════════════════════╗");
    LOG_I(TAG, "║           S32K388 DCM_GPR REGISTERS                      ║");
    LOG_I(TAG, "╚══════════════════════════════════════════════════════════╝");

    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;

    LOG_I(TAG, "DCMRWF1 = 0x%08lX", (unsigned long)dcmrwf1);

    uint32_t mac_conf = dcmrwf1 & 0x7;
    const char* mac_mode;
    switch (mac_conf) {
        case 0: mac_mode = "MII"; break;
        case 1: mac_mode = "RMII"; break;
        case 2: mac_mode = "RGMII"; break;
        default: mac_mode = "INVALID"; break;
    }
    LOG_I(TAG, "  MAC_CONF_SEL = %lu (%s)", (unsigned long)mac_conf, mac_mode);

    LOG_I(TAG, "DCMRWF3 = 0x%08lX", (unsigned long)dcmrwf3);
    LOG_I(TAG, "  Bit0 GMAC_RX_CLK_MUX_BYPASS = %lu", dcmrwf3 & 1);
    LOG_I(TAG, "  Bit1 GMAC_RX_CLK_RES_EN     = %lu", (dcmrwf3 >> 1) & 1);
    LOG_I(TAG, "  Bit2 GMAC_TX_CLK_RES_EN     = %lu", (dcmrwf3 >> 2) & 1);
    LOG_I(TAG, "  Bit3 GMAC_TX_CLK_OUT_EN     = %lu", (dcmrwf3 >> 3) & 1);
}

static void debug_s32k388_gmac_mac(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "╔══════════════════════════════════════════════════════════╗");
    LOG_I(TAG, "║           S32K388 GMAC MAC REGISTERS                     ║");
    LOG_I(TAG, "╚══════════════════════════════════════════════════════════╝");

    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    uint32_t mac_ext = IP_GMAC_0->MAC_EXT_CONFIGURATION;
    uint32_t mac_pkt_flt = IP_GMAC_0->MAC_PACKET_FILTER;

    LOG_I(TAG, "MAC_CONFIGURATION = 0x%08lX", (unsigned long)mac_cfg);
    LOG_I(TAG, "  Bit0  RE  (RX Enable)      = %lu", mac_cfg & 1);
    LOG_I(TAG, "  Bit1  TE  (TX Enable)      = %lu", (mac_cfg >> 1) & 1);
    LOG_I(TAG, "  Bit2  PRELEN               = %lu", (mac_cfg >> 2) & 3);
    LOG_I(TAG, "  Bit10 DCRS                 = %lu", (mac_cfg >> 10) & 1);
    LOG_I(TAG, "  Bit11 ECRSFD               = %lu", (mac_cfg >> 11) & 1);
    LOG_I(TAG, "  Bit12 LM  (Loopback)       = %lu", (mac_cfg >> 12) & 1);
    LOG_I(TAG, "  Bit13 DM  (Duplex Mode)    = %lu (%s)",
          (mac_cfg >> 13) & 1, ((mac_cfg >> 13) & 1) ? "Full" : "Half");
    LOG_I(TAG, "  Bit14 FES (Fast Ethernet)  = %lu (%s)",
          (mac_cfg >> 14) & 1, ((mac_cfg >> 14) & 1) ? "100Mbps" : "10Mbps");
    LOG_I(TAG, "  Bit15 PS  (Port Select)    = %lu (%s)",
          (mac_cfg >> 15) & 1, ((mac_cfg >> 15) & 1) ? "MII/10-100" : "GMII/1000");
    LOG_I(TAG, "  Bit16 JE  (Jumbo Enable)   = %lu", (mac_cfg >> 16) & 1);
    LOG_I(TAG, "  Bit19 JD  (Jabber Disable) = %lu", (mac_cfg >> 19) & 1);
    LOG_I(TAG, "  Bit21 CST (CRC Strip)      = %lu", (mac_cfg >> 21) & 1);
    LOG_I(TAG, "  Bit25 IPC (Checksum Offload) = %lu", (mac_cfg >> 25) & 1);

    LOG_I(TAG, "MAC_EXT_CONFIGURATION = 0x%08lX", (unsigned long)mac_ext);
    LOG_I(TAG, "MAC_PACKET_FILTER = 0x%08lX", (unsigned long)mac_pkt_flt);
    LOG_I(TAG, "  Bit0 PR (Promiscuous) = %lu", mac_pkt_flt & 1);
    LOG_I(TAG, "  Bit4 PM (Pass All Multicast) = %lu", (mac_pkt_flt >> 4) & 1);
    LOG_I(TAG, "  Bit31 RA (Receive All) = %lu", (mac_pkt_flt >> 31) & 1);
}

static void debug_s32k388_gmac_dma(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "╔══════════════════════════════════════════════════════════╗");
    LOG_I(TAG, "║           S32K388 GMAC DMA STATUS                        ║");
    LOG_I(TAG, "╚══════════════════════════════════════════════════════════╝");

    uint32_t dma_mode = IP_GMAC_0->DMA_MODE;
    uint32_t dma_status = IP_GMAC_0->DMA_DEBUG_STATUS0;
    uint32_t ch0_ctrl = IP_GMAC_0->DMA_CH0_CONTROL;
    uint32_t ch0_tx_ctrl = IP_GMAC_0->DMA_CH0_TX_CONTROL;
    uint32_t ch0_rx_ctrl = IP_GMAC_0->DMA_CH0_RX_CONTROL;
    uint32_t ch0_status = IP_GMAC_0->DMA_CH0_STATUS;

    LOG_I(TAG, "DMA_MODE = 0x%08lX", (unsigned long)dma_mode);
    LOG_I(TAG, "DMA_DEBUG_STATUS0 = 0x%08lX", (unsigned long)dma_status);

    uint32_t tx_state = (dma_status >> 12) & 0xF;
    uint32_t rx_state = (dma_status >> 8) & 0xF;

    const char* tx_states[] = {"Stopped", "FetchDesc", "Wait", "ReadData",
                               "Suspend", "CloseDesc", "WriteTS", "???"};
    const char* rx_states[] = {"Stopped", "FetchDesc", "???", "Wait",
                               "Suspend", "CloseDesc", "WriteTS", "Transfer"};

    LOG_I(TAG, "  TX DMA State: %s (%lu)", tx_states[tx_state & 7], (unsigned long)tx_state);
    LOG_I(TAG, "  RX DMA State: %s (%lu)", rx_states[rx_state & 7], (unsigned long)rx_state);

    LOG_I(TAG, "DMA_CH0_CONTROL = 0x%08lX", (unsigned long)ch0_ctrl);
    LOG_I(TAG, "DMA_CH0_TX_CONTROL = 0x%08lX [ST=%lu]",
          (unsigned long)ch0_tx_ctrl, ch0_tx_ctrl & 1);
    LOG_I(TAG, "DMA_CH0_RX_CONTROL = 0x%08lX [SR=%lu]",
          (unsigned long)ch0_rx_ctrl, ch0_rx_ctrl & 1);
    LOG_I(TAG, "DMA_CH0_STATUS = 0x%08lX", (unsigned long)ch0_status);
}

static void debug_s32k388_gmac_counters(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "╔══════════════════════════════════════════════════════════╗");
    LOG_I(TAG, "║           S32K388 GMAC PACKET COUNTERS                   ║");
    LOG_I(TAG, "╚══════════════════════════════════════════════════════════╝");

    /* TX Counters */
    uint32_t tx_pkt_good = IP_GMAC_0->TX_PACKET_COUNT_GOOD;
    uint32_t tx_broadcast = IP_GMAC_0->TX_BROADCAST_PACKETS_GOOD;
    uint32_t tx_multicast = IP_GMAC_0->TX_MULTICAST_PACKETS_GOOD;
    uint32_t tx_unicast = IP_GMAC_0->TX_UNICAST_PACKETS_GOOD;
    uint32_t tx_underflow = IP_GMAC_0->TX_UNDERFLOW_ERROR_PACKETS;

    LOG_I(TAG, "TX Counters:");
    LOG_I(TAG, "  Total Good:  %lu (delta: %lu)",
          (unsigned long)tx_pkt_good, (unsigned long)(tx_pkt_good - g_prev_tx_good));
    LOG_I(TAG, "  Broadcast:   %lu", (unsigned long)tx_broadcast);
    LOG_I(TAG, "  Multicast:   %lu", (unsigned long)tx_multicast);
    LOG_I(TAG, "  Unicast:     %lu", (unsigned long)tx_unicast);
    LOG_I(TAG, "  Underflow:   %lu %s", (unsigned long)tx_underflow,
          tx_underflow ? "<-- DMA ISSUE!" : "");

    g_prev_tx_good = tx_pkt_good;

    /* RX Counters */
    uint32_t rx_pkt_good = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
    uint32_t rx_broadcast = IP_GMAC_0->RX_BROADCAST_PACKETS_GOOD;
    uint32_t rx_multicast = IP_GMAC_0->RX_MULTICAST_PACKETS_GOOD;
    uint32_t rx_unicast = IP_GMAC_0->RX_UNICAST_PACKETS_GOOD;
    uint32_t rx_crc_err = IP_GMAC_0->RX_CRC_ERROR_PACKETS;
    uint32_t rx_align_err = IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS;
    uint32_t rx_runt = IP_GMAC_0->RX_RUNT_ERROR_PACKETS;
    uint32_t rx_jabber = IP_GMAC_0->RX_JABBER_ERROR_PACKETS;
    uint32_t rx_oversize = IP_GMAC_0->RX_OVERSIZE_PACKETS_GOOD;
    uint32_t rx_undersize = IP_GMAC_0->RX_UNDERSIZE_PACKETS_GOOD;
    uint32_t rx_length_err = IP_GMAC_0->RX_LENGTH_ERROR_PACKETS;
    uint32_t rx_fifo_overflow = IP_GMAC_0->RX_FIFO_OVERFLOW_PACKETS;

    LOG_I(TAG, "RX Counters:");
    LOG_I(TAG, "  Total:       %lu (delta: %lu)",
          (unsigned long)rx_pkt_good, (unsigned long)(rx_pkt_good - g_prev_rx_good));
    LOG_I(TAG, "  Broadcast:   %lu", (unsigned long)rx_broadcast);
    LOG_I(TAG, "  Multicast:   %lu", (unsigned long)rx_multicast);
    LOG_I(TAG, "  Unicast:     %lu", (unsigned long)rx_unicast);

    g_prev_rx_good = rx_pkt_good;

    LOG_I(TAG, "RX Errors:");
    LOG_I(TAG, "  CRC Error:   %lu (delta: %lu) %s",
          (unsigned long)rx_crc_err, (unsigned long)(rx_crc_err - g_prev_rx_crc),
          (rx_crc_err > g_prev_rx_crc) ? "<-- TIMING ISSUE!" : "");
    LOG_I(TAG, "  Align Error: %lu (delta: %lu) %s",
          (unsigned long)rx_align_err, (unsigned long)(rx_align_err - g_prev_rx_align),
          (rx_align_err > g_prev_rx_align) ? "<-- TIMING ISSUE!" : "");
    LOG_I(TAG, "  Runt:        %lu %s", (unsigned long)rx_runt,
          rx_runt ? "<-- SHORT FRAMES!" : "");
    LOG_I(TAG, "  Jabber:      %lu %s", (unsigned long)rx_jabber,
          rx_jabber ? "<-- LONG FRAMES!" : "");
    LOG_I(TAG, "  Oversize:    %lu", (unsigned long)rx_oversize);
    LOG_I(TAG, "  Undersize:   %lu", (unsigned long)rx_undersize);
    LOG_I(TAG, "  Length Err:  %lu", (unsigned long)rx_length_err);
    LOG_I(TAG, "  FIFO Ovfl:   %lu %s", (unsigned long)rx_fifo_overflow,
          rx_fifo_overflow ? "<-- DMA TOO SLOW!" : "");

    g_prev_rx_crc = rx_crc_err;
    g_prev_rx_align = rx_align_err;
}

/*===========================================================================*/
/*                    DEBUG: LAN9646 REGISTERS                                */
/*===========================================================================*/

/* MIB Counter indices */
#define MIB_RX_TOTAL            0x01
#define MIB_RX_BROADCAST        0x06
#define MIB_RX_MULTICAST        0x08
#define MIB_RX_UNICAST          0x0A
#define MIB_RX_CRC_ERR          0x18
#define MIB_RX_UNDERSIZE        0x1A
#define MIB_RX_OVERSIZE         0x1E
#define MIB_RX_FRAGMENT         0x1C
#define MIB_RX_JABBER           0x20
#define MIB_RX_SYMBOL_ERR       0x22
#define MIB_TX_TOTAL            0x24
#define MIB_TX_BROADCAST        0x26
#define MIB_TX_MULTICAST        0x28
#define MIB_TX_UNICAST          0x2A
#define MIB_TX_LATE_COL         0x2E
#define MIB_TX_EXCESS_COL       0x30
#define MIB_TX_SINGLE_COL       0x32
#define MIB_TX_MULTI_COL        0x34

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

static void debug_lan9646_global(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "╔══════════════════════════════════════════════════════════╗");
    LOG_I(TAG, "║           LAN9646 GLOBAL REGISTERS                       ║");
    LOG_I(TAG, "╚══════════════════════════════════════════════════════════╝");

    uint8_t switch_ctrl;
    uint16_t chip_id;
    uint8_t chip_id_h, chip_id_l, revision;

    lan9646_read_reg8(&g_lan9646, 0x0300, &switch_ctrl);
    lan9646_read_reg8(&g_lan9646, 0x0000, &chip_id_l);
    lan9646_read_reg8(&g_lan9646, 0x0001, &chip_id_h);
    lan9646_read_reg8(&g_lan9646, 0x0002, &revision);

    chip_id = ((uint16_t)chip_id_h << 8) | chip_id_l;

    LOG_I(TAG, "Chip ID: 0x%04X Rev: %d", chip_id, revision);
    LOG_I(TAG, "Switch Control (0x0300) = 0x%02X", switch_ctrl);
    LOG_I(TAG, "  Bit0 Start Switch = %d", switch_ctrl & 1);
}

static void debug_lan9646_port6_config(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "╔══════════════════════════════════════════════════════════╗");
    LOG_I(TAG, "║           LAN9646 PORT 6 (RGMII) CONFIGURATION           ║");
    LOG_I(TAG, "╚══════════════════════════════════════════════════════════╝");

    uint8_t ctrl0, ctrl1, port_status, mstp_ptr, mstp_state;
    uint32_t membership;

    lan9646_read_reg8(&g_lan9646, 0x6300, &ctrl0);
    lan9646_read_reg8(&g_lan9646, 0x6301, &ctrl1);
    lan9646_read_reg8(&g_lan9646, 0x6030, &port_status);
    lan9646_read_reg8(&g_lan9646, 0x6B01, &mstp_ptr);
    lan9646_read_reg8(&g_lan9646, 0x6B04, &mstp_state);
    lan9646_read_reg32(&g_lan9646, 0x6A04, &membership);

    LOG_I(TAG, "XMII_CTRL0 (0x6300) = 0x%02X", ctrl0);
    LOG_I(TAG, "  Bit6 Duplex       = %d (%s)", (ctrl0 >> 6) & 1,
          ((ctrl0 >> 6) & 1) ? "Full" : "Half");
    LOG_I(TAG, "  Bit5 TX Flow Ctrl = %d", (ctrl0 >> 5) & 1);
    LOG_I(TAG, "  Bit4 Speed 100M   = %d", (ctrl0 >> 4) & 1);
    LOG_I(TAG, "  Bit3 RX Flow Ctrl = %d", (ctrl0 >> 3) & 1);

    LOG_I(TAG, "XMII_CTRL1 (0x6301) = 0x%02X", ctrl1);
    LOG_I(TAG, "  Bit6 Speed 1G Sel = %d (%s)", (ctrl1 >> 6) & 1,
          ((ctrl1 >> 6) & 1) ? "10/100 Mode" : "1000 Mode");
    LOG_I(TAG, "  Bit4 RX_DLY (Ingress) = %d (+1.3ns)", (ctrl1 >> 4) & 1);
    LOG_I(TAG, "  Bit3 TX_DLY (Egress)  = %d (+1.3ns)", (ctrl1 >> 3) & 1);

    LOG_I(TAG, "PORT_STATUS (0x6030) = 0x%02X", port_status);
    uint8_t spd = (port_status >> 3) & 0x03;
    LOG_I(TAG, "  Speed Status = %s",
          (spd == 2) ? "1000M" : (spd == 1) ? "100M" : "10M");
    LOG_I(TAG, "  Duplex Status = %s", (port_status & 0x04) ? "Full" : "Half");

    LOG_I(TAG, "MSTP_PTR (0x6B01) = 0x%02X", mstp_ptr);
    LOG_I(TAG, "MSTP_STATE (0x6B04) = 0x%02X", mstp_state);
    LOG_I(TAG, "  Bit2 TX Enable = %d", (mstp_state >> 2) & 1);
    LOG_I(TAG, "  Bit1 RX Enable = %d", (mstp_state >> 1) & 1);
    LOG_I(TAG, "  Bit0 Learning  = %d", mstp_state & 1);

    LOG_I(TAG, "PORT_MEMBERSHIP (0x6A04) = 0x%08lX", (unsigned long)membership);
    LOG_I(TAG, "  Can forward to: P1=%d P2=%d P3=%d P4=%d P6=%d P7=%d",
          (membership >> 0) & 1, (membership >> 1) & 1,
          (membership >> 2) & 1, (membership >> 3) & 1,
          (membership >> 5) & 1, (membership >> 6) & 1);
}

static void debug_lan9646_port6_mib(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "╔══════════════════════════════════════════════════════════╗");
    LOG_I(TAG, "║           LAN9646 PORT 6 MIB COUNTERS                    ║");
    LOG_I(TAG, "╚══════════════════════════════════════════════════════════╝");

    /* Note: MIB counters are READ-CLEAR, so values reset after reading */
    uint32_t rx_total = read_mib_counter(6, MIB_RX_TOTAL);
    uint32_t rx_broadcast = read_mib_counter(6, MIB_RX_BROADCAST);
    uint32_t rx_multicast = read_mib_counter(6, MIB_RX_MULTICAST);
    uint32_t rx_unicast = read_mib_counter(6, MIB_RX_UNICAST);
    uint32_t rx_crc = read_mib_counter(6, MIB_RX_CRC_ERR);
    uint32_t rx_undersize = read_mib_counter(6, MIB_RX_UNDERSIZE);
    uint32_t rx_oversize = read_mib_counter(6, MIB_RX_OVERSIZE);
    uint32_t rx_fragment = read_mib_counter(6, MIB_RX_FRAGMENT);
    uint32_t rx_jabber = read_mib_counter(6, MIB_RX_JABBER);
    uint32_t rx_symbol = read_mib_counter(6, MIB_RX_SYMBOL_ERR);

    uint32_t tx_total = read_mib_counter(6, MIB_TX_TOTAL);
    uint32_t tx_broadcast = read_mib_counter(6, MIB_TX_BROADCAST);
    uint32_t tx_multicast = read_mib_counter(6, MIB_TX_MULTICAST);
    uint32_t tx_unicast = read_mib_counter(6, MIB_TX_UNICAST);
    uint32_t tx_late = read_mib_counter(6, MIB_TX_LATE_COL);
    uint32_t tx_excess = read_mib_counter(6, MIB_TX_EXCESS_COL);

    LOG_I(TAG, "RX from S32K388 (GMAC TX -> Port 6 RX):");
    LOG_I(TAG, "  Total:     %lu", (unsigned long)rx_total);
    LOG_I(TAG, "  Broadcast: %lu", (unsigned long)rx_broadcast);
    LOG_I(TAG, "  Multicast: %lu", (unsigned long)rx_multicast);
    LOG_I(TAG, "  Unicast:   %lu", (unsigned long)rx_unicast);

    LOG_I(TAG, "RX Errors (S32K388 -> LAN9646):");
    LOG_I(TAG, "  CRC:       %lu %s", (unsigned long)rx_crc,
          rx_crc ? "<-- S32K TX TIMING!" : "");
    LOG_I(TAG, "  Symbol:    %lu %s", (unsigned long)rx_symbol,
          rx_symbol ? "<-- SIGNAL QUALITY!" : "");
    LOG_I(TAG, "  Undersize: %lu", (unsigned long)rx_undersize);
    LOG_I(TAG, "  Oversize:  %lu", (unsigned long)rx_oversize);
    LOG_I(TAG, "  Fragment:  %lu", (unsigned long)rx_fragment);
    LOG_I(TAG, "  Jabber:    %lu", (unsigned long)rx_jabber);

    LOG_I(TAG, "TX to S32K388 (Port 6 TX -> GMAC RX):");
    LOG_I(TAG, "  Total:     %lu", (unsigned long)tx_total);
    LOG_I(TAG, "  Broadcast: %lu", (unsigned long)tx_broadcast);
    LOG_I(TAG, "  Multicast: %lu", (unsigned long)tx_multicast);
    LOG_I(TAG, "  Unicast:   %lu", (unsigned long)tx_unicast);

    LOG_I(TAG, "TX Errors (LAN9646 -> S32K388):");
    LOG_I(TAG, "  Late Col:  %lu %s", (unsigned long)tx_late,
          tx_late ? "<-- DUPLEX MISMATCH!" : "");
    LOG_I(TAG, "  Excess Col:%lu", (unsigned long)tx_excess);
}

static void debug_lan9646_all_ports_mib(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "╔══════════════════════════════════════════════════════════╗");
    LOG_I(TAG, "║           LAN9646 ALL PORTS MIB SUMMARY                  ║");
    LOG_I(TAG, "╚══════════════════════════════════════════════════════════╝");
    LOG_I(TAG, "Port | RX Total | RX UC | RX BC | TX Total | TX UC | TX BC");
    LOG_I(TAG, "-----+----------+-------+-------+----------+-------+------");

    for (int port = 1; port <= 4; port++) {
        uint32_t rx_t = read_mib_counter(port, MIB_RX_TOTAL);
        uint32_t rx_uc = read_mib_counter(port, MIB_RX_UNICAST);
        uint32_t rx_bc = read_mib_counter(port, MIB_RX_BROADCAST);
        uint32_t tx_t = read_mib_counter(port, MIB_TX_TOTAL);
        uint32_t tx_uc = read_mib_counter(port, MIB_TX_UNICAST);
        uint32_t tx_bc = read_mib_counter(port, MIB_TX_BROADCAST);
        LOG_I(TAG, "  %d  | %8lu | %5lu | %5lu | %8lu | %5lu | %5lu",
              port, (unsigned long)rx_t, (unsigned long)rx_uc, (unsigned long)rx_bc,
              (unsigned long)tx_t, (unsigned long)tx_uc, (unsigned long)tx_bc);
    }

    /* Port 6 */
    uint32_t rx_t = read_mib_counter(6, MIB_RX_TOTAL);
    uint32_t rx_uc = read_mib_counter(6, MIB_RX_UNICAST);
    uint32_t rx_bc = read_mib_counter(6, MIB_RX_BROADCAST);
    uint32_t tx_t = read_mib_counter(6, MIB_TX_TOTAL);
    uint32_t tx_uc = read_mib_counter(6, MIB_TX_UNICAST);
    uint32_t tx_bc = read_mib_counter(6, MIB_TX_BROADCAST);
    LOG_I(TAG, "  6* | %8lu | %5lu | %5lu | %8lu | %5lu | %5lu  (* RGMII)",
          (unsigned long)rx_t, (unsigned long)rx_uc, (unsigned long)rx_bc,
          (unsigned long)tx_t, (unsigned long)tx_uc, (unsigned long)tx_bc);
}

/*===========================================================================*/
/*                    DEBUG: COMPREHENSIVE ANALYSIS                           */
/*===========================================================================*/

static void debug_full_system(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "#                                                              #");
    LOG_I(TAG, "#              FULL SYSTEM DEBUG DUMP                          #");
    LOG_I(TAG, "#                                                              #");
    LOG_I(TAG, "################################################################");

    debug_s32k388_clocks();
    debug_s32k388_dcm();
    debug_s32k388_gmac_mac();
    debug_s32k388_gmac_dma();
    debug_s32k388_gmac_counters();

    debug_lan9646_global();
    debug_lan9646_port6_config();
    debug_lan9646_port6_mib();
    debug_lan9646_all_ports_mib();
}

static void debug_quick_status(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "=== Quick Status ===");

    /* S32K388 GMAC */
    uint32_t tx_good = IP_GMAC_0->TX_PACKET_COUNT_GOOD;
    uint32_t rx_good = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
    uint32_t rx_crc = IP_GMAC_0->RX_CRC_ERROR_PACKETS;
    uint32_t rx_align = IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS;

    LOG_I(TAG, "GMAC: TX=%lu RX=%lu CRC_Err=%lu Align_Err=%lu",
          (unsigned long)tx_good, (unsigned long)rx_good,
          (unsigned long)rx_crc, (unsigned long)rx_align);

    /* LAN9646 Port 6 */
    uint32_t p6_rx = read_mib_counter(6, MIB_RX_TOTAL);
    uint32_t p6_tx = read_mib_counter(6, MIB_TX_TOTAL);
    uint32_t p6_crc = read_mib_counter(6, MIB_RX_CRC_ERR);

    LOG_I(TAG, "LAN9646 P6: RX=%lu TX=%lu CRC_Err=%lu",
          (unsigned long)p6_rx, (unsigned long)p6_tx, (unsigned long)p6_crc);

    /* Analysis */
    if (rx_crc > 0 || rx_align > 0) {
        LOG_W(TAG, ">>> S32K388 RX has errors - check LAN9646 TX timing");
    }
    if (p6_crc > 0) {
        LOG_W(TAG, ">>> LAN9646 RX has errors - check S32K388 TX timing");
    }
    if (tx_good > 0 && p6_rx == 0) {
        LOG_W(TAG, ">>> S32K388 TX OK but LAN9646 not receiving - HW issue?");
    }
    if (p6_tx > 0 && rx_good == 0) {
        LOG_W(TAG, ">>> LAN9646 TX OK but S32K388 not receiving - HW issue?");
    }
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
     *   Bit 4: RX Ingress Delay (+1.3ns) - LAN9646 receives from GMAC
     *   Bit 3: TX Egress Delay (+1.3ns)  - LAN9646 transmits to GMAC
     */
    uint8_t ctrl1;

#if RGMII_DELAY_OPTION == 0
    /* No delay - use if PCB trace already has delay */
    ctrl1 = 0x40;  /* 100M mode only */
    LOG_I(TAG, "  Delay Option 0: No delay");
#elif RGMII_DELAY_OPTION == 1
    /* TX delay only - delays LAN9646 TX to GMAC RX */
    ctrl1 = 0x40 | 0x08;
    LOG_I(TAG, "  Delay Option 1: TX delay only (+1.3ns)");
#elif RGMII_DELAY_OPTION == 2
    /* RX delay only - delays LAN9646 RX from GMAC TX */
    ctrl1 = 0x40 | 0x10;
    LOG_I(TAG, "  Delay Option 2: RX delay only (+1.3ns)");
#else
    /* Both delays */
    ctrl1 = 0x40 | 0x18;
    LOG_I(TAG, "  Delay Option 3: Both TX+RX delay (+1.3ns each)");
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
     *   Bit 1: GMAC_RX_CLK_RES_EN (internal termination resistor)
     *   Bit 2: GMAC_TX_CLK_RES_EN (internal termination resistor)
     *   Bit 3: GMAC_TX_CLK_OUT_EN (1=enable TX_CLK output)
     *
     * Note: S32K388 does NOT have internal RGMII delay.
     *       All timing adjustment must be done on LAN9646 side.
     */
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    dcmrwf3 |= (1U << 0);   /* RX_CLK_MUX_BYPASS = 1 */
    dcmrwf3 |= (1U << 3);   /* TX_CLK_OUT_EN = 1 */
    IP_DCM_GPR->DCMRWF3 = dcmrwf3;

    LOG_I(TAG, "  DCMRWF3=0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF3);
    LOG_I(TAG, "  Note: S32K388 has NO internal RGMII delay");
}

static void configure_gmac_mac(void) {
    LOG_I(TAG, "Configuring GMAC MAC for 100Mbps Full Duplex...");

    /*
     * MAC_CONFIGURATION register:
     *   Bit 15 (PS): Port Select (1=MII/10-100, 0=GMII/1000)
     *   Bit 14 (FES): Fast Ethernet Speed (1=100Mbps, 0=10Mbps) when PS=1
     *   Bit 13 (DM): Duplex Mode (1=Full, 0=Half)
     *   Bit 12 (LM): Loopback Mode
     *   Bit 11 (ECRSFD): Enable Carrier Sense
     *   Bit 1 (TE): Transmitter Enable
     *   Bit 0 (RE): Receiver Enable
     */
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;

    /* Clear speed/duplex/loopback bits */
    mac_cfg &= ~((1U << 15) | (1U << 14) | (1U << 13) | (1U << 12));

    /* Set for 100Mbps Full Duplex, no loopback */
    mac_cfg |= (1U << 15);  /* PS = 1 (10/100 mode) */
    mac_cfg |= (1U << 14);  /* FES = 1 (100Mbps) */
    mac_cfg |= (1U << 13);  /* DM = 1 (Full Duplex) */
    mac_cfg |= (1U << 11);  /* ECRSFD */

    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;

    LOG_I(TAG, "  MAC_CFG=0x%08lX", (unsigned long)IP_GMAC_0->MAC_CONFIGURATION);

    /* MAC_EXT_CONFIGURATION: Enable extended status */
    uint32_t ext_cfg = IP_GMAC_0->MAC_EXT_CONFIGURATION;
    ext_cfg |= (1U << 12);
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

    /* Full system debug on startup */
    delay_ms(1000);
    debug_full_system();

    uint32_t last_print = 0;
    uint32_t iteration = 0;

    while (1) {
#if NO_SYS
        sys_check_timeouts();
#else
        sys_msleep(5000);
#endif

        uint32_t time_now = OsIf_GetMilliseconds() / 1000;

        if (time_now - last_print >= 10) {
            last_print = time_now;
            iteration++;

            LOG_I(TAG, "");
            LOG_I(TAG, "############### ITERATION %lu @ %lu sec ###############",
                  (unsigned long)iteration, (unsigned long)time_now);
            LOG_I(TAG, "IP: %s", ip4addr_ntoa(netif_ip4_addr(&network_interfaces[0])));

            /* Quick status every iteration */
            debug_quick_status();

            /* Full counters every iteration */
            debug_s32k388_gmac_counters();
            debug_lan9646_port6_mib();

            /* Full system debug every 5 iterations */
            if (iteration % 5 == 0) {
                debug_full_system();
            }
        }

        if (time_now - start_time >= tests_timeout) {
            LOG_W(TAG, "Test timeout");
            break;
        }
    }
}

static void start_example(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "#                                                              #");
    LOG_I(TAG, "#     lwIP + LAN9646 + GMAC - RGMII 100M - DEBUG VERSION       #");
    LOG_I(TAG, "#                                                              #");
    LOG_I(TAG, "#     Delay Option: %d                                          #", RGMII_DELAY_OPTION);
    LOG_I(TAG, "#       0=None, 1=TX only, 2=RX only, 3=Both                   #");
    LOG_I(TAG, "#                                                              #");
    LOG_I(TAG, "################################################################");

#if defined(USING_OS_FREERTOS)
    xTaskCreate(mainLoopTask, "mainloop", 1024U, NULL, tskIDLE_PRIORITY + 1, NULL);
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

    LOG_I(TAG, "");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "              DEVICE INITIALIZATION - DEBUG MODE");
    LOG_I(TAG, "================================================================");

    /* Step 1: Configure S32K388 RGMII interface */
    LOG_I(TAG, "[Step 1] Configuring S32K388 RGMII...");
    configure_gmac_rgmii();

    /* Step 2: Initialize LAN9646 switch */
    LOG_I(TAG, "[Step 2] Initializing LAN9646...");
    lan9646_init_device();

    /* Step 3: Initialize Ethernet driver (AUTOSAR) */
    LOG_I(TAG, "[Step 3] Initializing Ethernet (AUTOSAR)...");
    Eth_Init(NULL_PTR);

    /* Step 4: Configure GMAC MAC after Eth_Init */
    LOG_I(TAG, "[Step 4] Configuring GMAC MAC...");
    configure_gmac_mac();

    /* Step 5: Print MAC address */
    uint8_t mac[6];
    Eth_43_GMAC_GetPhysAddr(ETH_CTRL_IDX, mac);
    LOG_I(TAG, "[Step 5] MAC: %02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    /* Initial state dump */
    LOG_I(TAG, "");
    LOG_I(TAG, "=== Initial Configuration Dump ===");
    debug_s32k388_clocks();
    debug_s32k388_dcm();
    debug_lan9646_port6_config();
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
