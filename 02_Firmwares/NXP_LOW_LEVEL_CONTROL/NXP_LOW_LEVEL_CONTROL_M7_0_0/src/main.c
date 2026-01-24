/**
 * @file    main.c
 * @brief   RGMII 1Gbps Test - S32K388 GMAC + LAN9646 Port 6 (Baremetal)
 */

#include <string.h>
#include <stdint.h>

#include "S32K388.h"
#include "Mcal.h"
#include "Mcu.h"
#include "Mcu_Cfg.h"
#include "Port.h"
#include "Port_Cfg.h"
#include "OsIf.h"
#include "Platform.h"
#include "Gpt.h"
#include "Gpt_Cfg.h"
#include "Eth_43_GMAC.h"
#include "Eth_43_GMAC_Cfg.h"
#include "Gmac_Ip.h"

#include "lan9646.h"
#include "s32k3xx_soft_i2c.h"
#include "CDD_Uart.h"
#include "log_debug.h"

/* External config symbols from generated PBcfg files */
extern const Eth_43_GMAC_ConfigType Eth_43_GMAC_xPredefinedConfig;

#define TAG "RGMII"

/*===========================================================================*/
/*                          CONFIGURATION                                     */
/*===========================================================================*/

#define LAN9646_SCL_CHANNEL     DioConf_DioChannel_SCL_CH
#define LAN9646_SDA_CHANNEL     DioConf_DioChannel_SDA_CH
#define LAN9646_I2C_SPEED       5U
#define ETH_CTRL_IDX            0U

/*===========================================================================*/
/*                          GLOBAL VARIABLES                                  */
/*===========================================================================*/

static lan9646_t g_lan9646;
static softi2c_t g_i2c;

/*===========================================================================*/
/*                          DELAY FUNCTIONS                                   */
/*===========================================================================*/

static void delay_ms(uint32_t ms) {
    volatile uint32_t count;
    while (ms > 0) {
        count = 40000U;  /* ~1ms at 160MHz */
        while (count > 0) {
            count--;
        }
        ms--;
    }
}

static void delay_us(uint32_t us) {
    volatile uint32_t count;
    while (us > 0) {
        count = 40U;  /* ~1us at 160MHz */
        while (count > 0) {
            count--;
        }
        us--;
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
/*                          LAN9646 HELPERS                                   */
/*===========================================================================*/

static uint8_t lan_read8(uint16_t addr) {
    uint8_t val = 0;
    lan9646_read_reg8(&g_lan9646, addr, &val);
    return val;
}

static void lan_write8(uint16_t addr, uint8_t val) {
    lan9646_write_reg8(&g_lan9646, addr, val);
}

static uint32_t lan_read32(uint16_t addr) {
    uint32_t val = 0;
    lan9646_read_reg32(&g_lan9646, addr, &val);
    return val;
}

static void lan_write32(uint16_t addr, uint32_t val) {
    lan9646_write_reg32(&g_lan9646, addr, val);
}

/* Read MIB counter */
static uint32_t lan_read_mib(uint8_t port, uint8_t index) {
    uint16_t base = (uint16_t)port << 12;
    uint32_t ctrl;

    /* Set MIB index and read enable */
    ctrl = ((uint32_t)index << 16) | 0x02000000UL;
    lan_write32(base | 0x0500, ctrl);

    delay_us(10);

    /* Wait for read complete */
    uint32_t timeout = 1000;
    do {
        ctrl = lan_read32(base | 0x0500);
        if (--timeout == 0) break;
    } while (ctrl & 0x02000000UL);

    return lan_read32(base | 0x0504);
}

/*===========================================================================*/
/*                          LAN9646 INIT                                      */
/*===========================================================================*/

static lan9646r_t init_lan9646(void) {
    LOG_I(TAG, "Initializing LAN9646...");

    lan9646_cfg_t cfg = {
        .if_type = LAN9646_IF_I2C,
        .i2c_addr = 0x5F,
        .ops.i2c = {
            .init_fn = i2c_init_cb,
            .write_fn = i2c_write_cb,
            .read_fn = i2c_read_cb,
            .mem_write_fn = i2c_mem_write_cb,
            .mem_read_fn = i2c_mem_read_cb,
        },
    };

    if (lan9646_init(&g_lan9646, &cfg) != lan9646OK) {
        LOG_E(TAG, "LAN9646 init FAILED!");
        return lan9646ERR;
    }

    /* Read chip ID */
    uint16_t chip_id;
    uint8_t revision;
    lan9646_get_chip_id(&g_lan9646, &chip_id, &revision);
    LOG_I(TAG, "  Chip ID: 0x%04X, Rev: %d", chip_id, revision);

    /* Configure Port 6 for RGMII 1Gbps */
    LOG_I(TAG, "Configuring Port 6 for RGMII 1Gbps...");

    /* XMII_CTRL0: Full duplex, flow control */
    lan_write8(0x6300, 0x68);

    /* XMII_CTRL1: 1Gbps + TX ID + RX ID delays */
    lan_write8(0x6301, 0x18);

    /* Verify */
    uint8_t ctrl0 = lan_read8(0x6300);
    uint8_t ctrl1 = lan_read8(0x6301);
    LOG_I(TAG, "  XMII_CTRL0=0x%02X, XMII_CTRL1=0x%02X", ctrl0, ctrl1);
    LOG_I(TAG, "  Speed: %s, Duplex: %s",
          (ctrl1 & 0x40) ? "10/100M" : "1Gbps",
          (ctrl0 & 0x40) ? "Full" : "Half");
    LOG_I(TAG, "  TX_ID: %s, RX_ID: %s",
          (ctrl1 & 0x08) ? "ON" : "OFF",
          (ctrl1 & 0x10) ? "ON" : "OFF");

    /* Enable switch */
    lan_write8(0x0300, 0x01);

    /* Port membership - Port 6 can talk to Port 1-4 */
    lan_write32(0x6A04, 0x4F);
    lan_write32(0x1A04, 0x6E);
    lan_write32(0x2A04, 0x6D);
    lan_write32(0x3A04, 0x6B);
    lan_write32(0x4A04, 0x67);

    LOG_I(TAG, "LAN9646 configured OK");
    return lan9646OK;
}

/*===========================================================================*/
/*                          S32K388 RGMII CONFIG                              */
/*===========================================================================*/

static void configure_s32k388_rgmii(void) {
    LOG_I(TAG, "Configuring S32K388 RGMII...");

    /* DCMRWF1: RGMII mode */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    dcmrwf1 = (dcmrwf1 & ~0x03U) | 0x01U;  /* MAC_CONF_SEL = 1 (RGMII) */
    dcmrwf1 |= (1U << 6);                   /* MAC_TX_RMII_CLK_LPBCK_EN */
    IP_DCM_GPR->DCMRWF1 = dcmrwf1;
    LOG_I(TAG, "  DCMRWF1 = 0x%08lX (RGMII mode)", (unsigned long)IP_DCM_GPR->DCMRWF1);

    /* DCMRWF3: Clock bypass */
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    dcmrwf3 |= (1U << 13);  /* RX_CLK bypass */
    dcmrwf3 |= (1U << 11);  /* TX_CLK output enable */
    IP_DCM_GPR->DCMRWF3 = dcmrwf3;
    LOG_I(TAG, "  DCMRWF3 = 0x%08lX (RX bypass, TX out)", (unsigned long)IP_DCM_GPR->DCMRWF3);
}

static void configure_gmac_1gbps(void) {
    LOG_I(TAG, "Configuring GMAC for 1Gbps Full Duplex...");

    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    mac_cfg &= ~(1U << 15);  /* PS = 0 (1Gbps) */
    mac_cfg &= ~(1U << 14);  /* FES = 0 */
    mac_cfg |= (1U << 13);   /* DM = 1 (Full Duplex) */
    mac_cfg |= (1U << 0);    /* RE = 1 (RX Enable) */
    mac_cfg |= (1U << 1);    /* TE = 1 (TX Enable) */
    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;

    LOG_I(TAG, "  MAC_CFG = 0x%08lX", (unsigned long)IP_GMAC_0->MAC_CONFIGURATION);
}

/*===========================================================================*/
/*                          DIAGNOSTIC FUNCTIONS                              */
/*===========================================================================*/

static void print_separator(const char* title) {
    LOG_I(TAG, "");
    LOG_I(TAG, "============================================================");
    LOG_I(TAG, "  %s", title);
    LOG_I(TAG, "============================================================");
}

static void dump_gmac_rx_counters(void) {
    print_separator("GMAC RX COUNTERS");

    LOG_I(TAG, "  RX_PACKETS_GOOD_BAD  = %lu", (unsigned long)IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD);
    LOG_I(TAG, "  RX_OCTETS_GOOD       = %lu", (unsigned long)IP_GMAC_0->RX_OCTET_COUNT_GOOD);
    LOG_I(TAG, "  RX_BROADCAST         = %lu", (unsigned long)IP_GMAC_0->RX_BROADCAST_PACKETS_GOOD);
    LOG_I(TAG, "  RX_MULTICAST         = %lu", (unsigned long)IP_GMAC_0->RX_MULTICAST_PACKETS_GOOD);
    LOG_I(TAG, "  RX_UNICAST           = %lu", (unsigned long)IP_GMAC_0->RX_UNICAST_PACKETS_GOOD);
    LOG_I(TAG, "  ---");
    LOG_I(TAG, "  RX_CRC_ERROR         = %lu", (unsigned long)IP_GMAC_0->RX_CRC_ERROR_PACKETS);
    LOG_I(TAG, "  RX_ALIGN_ERROR       = %lu", (unsigned long)IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS);
    LOG_I(TAG, "  RX_RUNT_ERROR        = %lu", (unsigned long)IP_GMAC_0->RX_RUNT_ERROR_PACKETS);
    LOG_I(TAG, "  RX_FIFO_OVERFLOW     = %lu", (unsigned long)IP_GMAC_0->RX_FIFO_OVERFLOW_PACKETS);
}

static void dump_gmac_tx_counters(void) {
    print_separator("GMAC TX COUNTERS");

    LOG_I(TAG, "  TX_PACKETS_GOOD_BAD  = %lu", (unsigned long)IP_GMAC_0->TX_PACKET_COUNT_GOOD_BAD);
    LOG_I(TAG, "  TX_OCTETS_GOOD       = %lu", (unsigned long)IP_GMAC_0->TX_OCTET_COUNT_GOOD);
    LOG_I(TAG, "  TX_BROADCAST         = %lu", (unsigned long)IP_GMAC_0->TX_BROADCAST_PACKETS_GOOD);
    LOG_I(TAG, "  TX_MULTICAST         = %lu", (unsigned long)IP_GMAC_0->TX_MULTICAST_PACKETS_GOOD);
    LOG_I(TAG, "  TX_UNICAST           = %lu", (unsigned long)IP_GMAC_0->TX_UNICAST_PACKETS_GOOD_BAD);
}

static void dump_lan9646_port6_counters(void) {
    print_separator("LAN9646 PORT 6 COUNTERS");

    /* RX counters (from GMAC TX) */
    uint32_t rx_bcast = lan_read_mib(6, 0x0A);
    uint32_t rx_mcast = lan_read_mib(6, 0x0B);
    uint32_t rx_ucast = lan_read_mib(6, 0x0C);

    /* TX counters (to GMAC RX) */
    uint32_t tx_bcast = lan_read_mib(6, 0x63);
    uint32_t tx_mcast = lan_read_mib(6, 0x64);
    uint32_t tx_ucast = lan_read_mib(6, 0x65);

    LOG_I(TAG, "  RX (from GMAC): Bcast=%lu Mcast=%lu Ucast=%lu",
          (unsigned long)rx_bcast, (unsigned long)rx_mcast, (unsigned long)rx_ucast);
    LOG_I(TAG, "  TX (to GMAC):   Bcast=%lu Mcast=%lu Ucast=%lu",
          (unsigned long)tx_bcast, (unsigned long)tx_mcast, (unsigned long)tx_ucast);
}

static void dump_config_summary(void) {
    print_separator("CONFIGURATION SUMMARY");

    /* S32K388 */
    LOG_I(TAG, "[S32K388 GMAC]");
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    LOG_I(TAG, "  MAC_CFG    = 0x%08lX", (unsigned long)mac_cfg);
    LOG_I(TAG, "  Speed      = %s", (mac_cfg & (1<<15)) ? "10/100M" : "1Gbps");
    LOG_I(TAG, "  Duplex     = %s", (mac_cfg & (1<<13)) ? "Full" : "Half");
    LOG_I(TAG, "  RX Enable  = %s", (mac_cfg & (1<<0)) ? "YES" : "NO");
    LOG_I(TAG, "  TX Enable  = %s", (mac_cfg & (1<<1)) ? "YES" : "NO");

    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    LOG_I(TAG, "  RX_CLK Bypass = %s", (dcmrwf3 & (1<<13)) ? "YES" : "NO");
    LOG_I(TAG, "  TX_CLK Output = %s", (dcmrwf3 & (1<<11)) ? "YES" : "NO");

    /* LAN9646 */
    LOG_I(TAG, "");
    LOG_I(TAG, "[LAN9646 Port 6]");
    uint8_t ctrl0 = lan_read8(0x6300);
    uint8_t ctrl1 = lan_read8(0x6301);
    uint8_t status = lan_read8(0x6030);

    LOG_I(TAG, "  XMII_CTRL0 = 0x%02X", ctrl0);
    LOG_I(TAG, "  XMII_CTRL1 = 0x%02X", ctrl1);
    LOG_I(TAG, "  Speed      = %s", (ctrl1 & 0x40) ? "10/100M" : "1Gbps");
    LOG_I(TAG, "  Duplex     = %s", (ctrl0 & 0x40) ? "Full" : "Half");
    LOG_I(TAG, "  TX_ID Delay= %s", (ctrl1 & 0x08) ? "ON" : "OFF");
    LOG_I(TAG, "  RX_ID Delay= %s", (ctrl1 & 0x10) ? "ON" : "OFF");
    LOG_I(TAG, "  Link Status= 0x%02X", status);
}

/*===========================================================================*/
/*                          LOOPBACK TEST                                     */
/*===========================================================================*/

static uint8_t g_test_packet[64] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* Dest: Broadcast */
    0x10, 0x11, 0x22, 0x33, 0x44, 0x55,  /* Src: Test MAC */
    0x88, 0xB5,                          /* EtherType: Test */
    /* Payload */
    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
};

static uint32_t run_loopback_test(uint32_t count) {
    print_separator("LOOPBACK TEST");

    LOG_I(TAG, "Test: GMAC TX -> LAN9646 P6 -> Loopback -> P6 -> GMAC RX");
    LOG_I(TAG, "Sending %lu packets...", (unsigned long)count);

    /* Enable loopback on Port 6 */
    uint8_t op_ctrl = lan_read8(0x6020);
    lan_write8(0x6020, op_ctrl | 0x40);  /* Bit 6: Remote loopback */
    delay_ms(10);

    /* Record initial RX count */
    uint32_t rx_before = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;

    /* Send packets */
    Gmac_Ip_BufferType buf;
    buf.Data = g_test_packet;
    buf.Length = 64;

    for (uint32_t i = 0; i < count; i++) {
        Gmac_Ip_SendFrame(0, 0, &buf, NULL);
        delay_ms(2);
    }

    /* Wait for loopback */
    delay_ms(100);

    /* Check results */
    uint32_t rx_after = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
    uint32_t received = rx_after - rx_before;

    /* Disable loopback */
    lan_write8(0x6020, op_ctrl);

    /* Report */
    LOG_I(TAG, "");
    LOG_I(TAG, "Results:");
    LOG_I(TAG, "  TX sent     = %lu", (unsigned long)count);
    LOG_I(TAG, "  RX received = %lu", (unsigned long)received);

    if (received == count) {
        LOG_I(TAG, "  STATUS: OK - All packets received!");
    } else if (received > 0) {
        LOG_I(TAG, "  STATUS: PARTIAL - %lu/%lu received", (unsigned long)received, (unsigned long)count);
    } else {
        LOG_I(TAG, "  STATUS: FAIL - No packets received!");
    }

    return received;
}

/*===========================================================================*/
/*                          DELAY SWEEP TEST                                  */
/*===========================================================================*/

static void run_delay_sweep(void) {
    print_separator("TX DELAY SWEEP TEST");

    LOG_I(TAG, "Testing LAN9646 TX delay settings...");
    LOG_I(TAG, "(TX delay affects timing of signals TO S32K388)");
    LOG_I(TAG, "");

    const uint32_t test_count = 10;

    LOG_I(TAG, "TX_DLY | Sent | Recv | Status");
    LOG_I(TAG, "-------+------+------+--------");

    uint32_t best_rx = 0;
    bool best_delay = false;

    for (int tx_dly = 0; tx_dly <= 1; tx_dly++) {
        /* Set TX delay */
        uint8_t ctrl1 = lan_read8(0x6301);
        if (tx_dly) {
            ctrl1 |= 0x08;
        } else {
            ctrl1 &= ~0x08;
        }
        lan_write8(0x6301, ctrl1);
        delay_ms(10);

        /* Enable loopback */
        uint8_t op_ctrl = lan_read8(0x6020);
        lan_write8(0x6020, op_ctrl | 0x40);
        delay_ms(10);

        /* Test */
        uint32_t rx_before = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;

        Gmac_Ip_BufferType buf;
        buf.Data = g_test_packet;
        buf.Length = 64;

        for (uint32_t i = 0; i < test_count; i++) {
            Gmac_Ip_SendFrame(0, 0, &buf, NULL);
            delay_ms(2);
        }

        delay_ms(50);

        uint32_t received = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD - rx_before;

        /* Disable loopback */
        lan_write8(0x6020, op_ctrl);

        const char* status = (received == test_count) ? "OK" :
                             (received > 0) ? "PARTIAL" : "FAIL";

        LOG_I(TAG, "  %s  |  %2lu  |  %2lu  | %s",
              tx_dly ? "ON " : "OFF",
              (unsigned long)test_count,
              (unsigned long)received,
              status);

        if (received > best_rx) {
            best_rx = received;
            best_delay = tx_dly;
        }
    }

    LOG_I(TAG, "");
    if (best_rx > 0) {
        LOG_I(TAG, "Best: TX_DELAY=%s with %lu/%lu received",
              best_delay ? "ON" : "OFF",
              (unsigned long)best_rx,
              (unsigned long)test_count);

        /* Apply best setting */
        uint8_t ctrl1 = lan_read8(0x6301);
        if (best_delay) {
            ctrl1 |= 0x08;
        } else {
            ctrl1 &= ~0x08;
        }
        lan_write8(0x6301, ctrl1);
        LOG_I(TAG, "Applied TX_DELAY=%s", best_delay ? "ON" : "OFF");
    } else {
        LOG_I(TAG, "FAIL: No packets received with any setting!");
    }
}

/*===========================================================================*/
/*                          MAIN                                              */
/*===========================================================================*/

int main(void) {
    /* MCU Init */
    OsIf_Init(NULL_PTR);
    Port_Init(NULL_PTR);

    Mcu_Init(NULL_PTR);
    Mcu_InitClock(McuClockSettingConfig_0);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {}
    Mcu_DistributePllClock();
    Mcu_SetMode(McuModeSettingConf_0);

    Platform_Init(NULL_PTR);

    /* GPT for OsIf timing */
    Gpt_Init(NULL_PTR);
    Gpt_StartTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0, 40000U);
    Gpt_EnableNotification(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);

    /* UART & Log */
    Uart_Init(NULL_PTR);
    log_init();

    /* Banner */
    LOG_I(TAG, "");
    LOG_I(TAG, "############################################################");
    LOG_I(TAG, "#                                                          #");
    LOG_I(TAG, "#   RGMII 1Gbps TEST - S32K388 GMAC + LAN9646 Port 6       #");
    LOG_I(TAG, "#   Baremetal Version                                      #");
    LOG_I(TAG, "#                                                          #");
    LOG_I(TAG, "############################################################");
    LOG_I(TAG, "");

    /* LAN9646 Init */
    if (init_lan9646() != lan9646OK) {
        LOG_E(TAG, "FATAL: LAN9646 init failed!");
        while (1) { delay_ms(1000); }
    }

    /* GMAC Init */
    LOG_I(TAG, "Initializing GMAC...");
    Eth_43_GMAC_Init(&Eth_43_GMAC_xPredefinedConfig);
    configure_gmac_1gbps();
    Eth_43_GMAC_SetControllerMode(ETH_CTRL_IDX, ETH_MODE_ACTIVE);
    configure_s32k388_rgmii();
    LOG_I(TAG, "GMAC initialized OK");

    /* Wait for stabilization */
    delay_ms(100);

    /*=======================================================================*/
    /*                    DIAGNOSTIC SEQUENCE                                */
    /*=======================================================================*/

    /* Step 1: Configuration Summary */
    dump_config_summary();

    /* Step 2: Initial Counters */
    dump_gmac_rx_counters();
    dump_gmac_tx_counters();
    dump_lan9646_port6_counters();

    /* Step 3: Delay Sweep Test */
    run_delay_sweep();

    /* Step 4: Loopback Test */
    run_loopback_test(20);

    /* Step 5: Final Counters */
    dump_gmac_rx_counters();

    /*=======================================================================*/
    /*                    MONITORING LOOP                                    */
    /*=======================================================================*/

    print_separator("MONITORING MODE");
    LOG_I(TAG, "Printing RX/TX counts every 2 seconds...");
    LOG_I(TAG, "");

    uint32_t loop = 0;
    uint32_t prev_rx = 0;
    uint32_t prev_tx = 0;

    for (;;) {
        loop++;

        uint32_t rx = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
        uint32_t tx = IP_GMAC_0->TX_PACKET_COUNT_GOOD_BAD;
        uint32_t rx_err = IP_GMAC_0->RX_CRC_ERROR_PACKETS +
                          IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS;

        LOG_I(TAG, "[%04lu] RX=%lu (+%lu) TX=%lu (+%lu) ERR=%lu",
              (unsigned long)loop,
              (unsigned long)rx, (unsigned long)(rx - prev_rx),
              (unsigned long)tx, (unsigned long)(tx - prev_tx),
              (unsigned long)rx_err);

        prev_rx = rx;
        prev_tx = tx;

        /* Detailed dump every 30 seconds (15 iterations) */
        if (loop % 15 == 0) {
            dump_gmac_rx_counters();
            dump_lan9646_port6_counters();
        }

        delay_ms(2000);
    }

    return 0;
}
