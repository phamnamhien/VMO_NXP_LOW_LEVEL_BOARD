/**
 * \file            main.c
 * \brief           LAN9646 + Eth + FreeRTOS + lwIP
 */

#include <string.h>

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

#define TAG "MAIN"

#define LAN9646_SCL_CHANNEL     DioConf_DioChannel_SCL_CH
#define LAN9646_SDA_CHANNEL     DioConf_DioChannel_SDA_CH
#define LAN9646_I2C_SPEED       5U

static lan9646_t g_lan9646;
static softi2c_t g_i2c;

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

static lan9646r_t configure_port6_rgmii_1g(void) {
    uint8_t ctrl0, ctrl1, port_ctrl;

    LOG_I(TAG, "Configuring Port 6 for RGMII 1G...");

    /* XMII Control */
    ctrl0 = 0x68;  /* RGMII, 1G speed, Full Duplex */
    ctrl1 = 0x00;  /* TX_DLY=OFF, RX_DLY=OFF */
    lan9646_write_reg8(&g_lan9646, 0x6300, ctrl0);
    lan9646_write_reg8(&g_lan9646, 0x6301, ctrl1);

    /* ===== CRITICAL: Disable VLAN Filtering ===== */
    uint8_t lue_ctrl0;
    lan9646_read_reg8(&g_lan9646, 0x0310, &lue_ctrl0);
    lue_ctrl0 &= ~0x10;  // Clear bit 4 (VLAN_EN)
    lan9646_write_reg8(&g_lan9646, 0x0310, lue_ctrl0);

    LOG_I(TAG, "LUE_CTRL0: 0x%02X (VLAN disabled)", lue_ctrl0);

    /* Enable Switch Operation */
    lan9646_write_reg8(&g_lan9646, 0x0300, 0x01);

    /* Port membership */
    lan9646_write_reg32(&g_lan9646, 0x6A04, 0x4F);  // Port 6 → 1,2,3,4,7
    lan9646_write_reg32(&g_lan9646, 0x1A04, 0x6E);  // Port 1 → others
    lan9646_write_reg32(&g_lan9646, 0x2A04, 0x6D);  // Port 2 → others
    lan9646_write_reg32(&g_lan9646, 0x3A04, 0x6B);  // Port 3 → others
    lan9646_write_reg32(&g_lan9646, 0x4A04, 0x67);  // Port 4 → others

    /* MSTP State - Enable TX/RX/Learning for all ports */
    for (int port = 1; port <= 4; port++) {
        uint16_t base = (uint16_t)port << 12;
        lan9646_write_reg8(&g_lan9646, base | 0x0B01, 0x00);  // MSTP Pointer
        lan9646_write_reg8(&g_lan9646, base | 0x0B04, 0x07);  // TX/RX/Learn
    }

    lan9646_write_reg8(&g_lan9646, 0x6B01, 0x00);
    lan9646_write_reg8(&g_lan9646, 0x6B04, 0x07);

    /* Verify */
    uint8_t lue_verify;
    lan9646_read_reg8(&g_lan9646, 0x0310, &lue_verify);
    LOG_I(TAG, "LUE_CTRL0 verified: 0x%02X", lue_verify);

    return lan9646OK;
}
static void debug_gmac_tx(void)
{
    Eth_BufIdxType bufIdx;
    uint8_t* bufPtr;
    Std_ReturnType ret;

    /* Try to get TX buffer */
    ret = Eth_43_GMAC_ProvideTxBuffer(0, 0, &bufIdx, &bufPtr, NULL);
    LOG_I(TAG, "GMAC ProvideTxBuffer: %s (ret=%d)",
          (ret == E_OK) ? "OK" : "FAIL", ret);

    if (ret == E_OK) {
        /* Build simple ARP request */
        uint8_t test_frame[64];
        memset(test_frame, 0, sizeof(test_frame));

        /* Dest MAC: broadcast */
        memset(&test_frame[0], 0xFF, 6);
        /* Src MAC */
        test_frame[6] = 0x10; test_frame[7] = 0x11;
        test_frame[8] = 0x22; test_frame[9] = 0x77;
        test_frame[10] = 0x77; test_frame[11] = 0x77;
        /* EtherType: ARP */
        test_frame[12] = 0x08; test_frame[13] = 0x06;

        memcpy(bufPtr, test_frame, 64);

        ret = Eth_43_GMAC_Transmit(0, bufIdx, 0x0806, FALSE, 64, NULL);
        LOG_I(TAG, "GMAC Transmit: %s (ret=%d)",
              (ret == E_OK) ? "OK" : "FAIL", ret);
    }

    /* Check controller mode */
    Eth_ModeType mode;
    Eth_43_GMAC_GetControllerMode(0, &mode);
    LOG_I(TAG, "GMAC Mode: %d (1=DOWN, 2=ACTIVE)", mode);
}

static void debug_gmac_clock(void)
{
    LOG_I(TAG, "=== GMAC Clock & Config Debug ===");

    /* DCM_GPR registers */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    LOG_I(TAG, "DCMRWF1: 0x%08lX [MAC_CONF_SEL=%lu]",
          (unsigned long)dcmrwf1, (unsigned long)(dcmrwf1 & 0x7));
    LOG_I(TAG, "DCMRWF3: 0x%08lX [RX_CLK_MUX_BYPASS=%lu]",
          (unsigned long)dcmrwf3, (unsigned long)(dcmrwf3 & 0x1));

    /* MC_CGM MUX */
    LOG_I(TAG, "MC_CGM MUX_7_CSS: 0x%08lX", (unsigned long)IP_MC_CGM->MUX_7_CSS);
    LOG_I(TAG, "MC_CGM MUX_9_CSS: 0x%08lX", (unsigned long)IP_MC_CGM->MUX_9_CSS);

    /* MAC Configuration */
    uint32_t mac_config = IP_GMAC_0->MAC_CONFIGURATION;
    uint32_t mac_ext_config = IP_GMAC_0->MAC_EXT_CONFIGURATION;
    LOG_I(TAG, "MAC_CONFIG: 0x%08lX", (unsigned long)mac_config);
    LOG_I(TAG, "  RE=%d TE=%d FES(100M)=%d PS(PortSel)=%d DM(Duplex)=%d",
          (int)(mac_config & 1),
          (int)((mac_config >> 1) & 1),
          (int)((mac_config >> 13) & 1),
          (int)((mac_config >> 15) & 1),
          (int)((mac_config >> 11) & 1));
    LOG_I(TAG, "MAC_EXT_CONFIG: 0x%08lX [PortSel=%d]",
          (unsigned long)mac_ext_config,
          (int)((mac_ext_config >> 12) & 0x7));

    /* RX Error counters */
    LOG_I(TAG, "--- RX Error Counters ---");
    LOG_I(TAG, "RX_CRC_ERROR: %lu", (unsigned long)IP_GMAC_0->RX_CRC_ERROR_PACKETS);
    LOG_I(TAG, "RX_ALIGN_ERROR: %lu", (unsigned long)IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS);
    LOG_I(TAG, "RX_RUNT_ERROR: %lu", (unsigned long)IP_GMAC_0->RX_RUNT_ERROR_PACKETS);
    LOG_I(TAG, "RX_JABBER_ERROR: %lu", (unsigned long)IP_GMAC_0->RX_JABBER_ERROR_PACKETS);
    LOG_I(TAG, "RX_LENGTH_ERROR: %lu", (unsigned long)IP_GMAC_0->RX_LENGTH_ERROR_PACKETS);
    LOG_I(TAG, "RX_OUT_OF_RANGE: %lu", (unsigned long)IP_GMAC_0->RX_OUT_OF_RANGE_TYPE_PACKETS);

    /* RX Good counters */
    LOG_I(TAG, "--- RX Good Counters ---");
    LOG_I(TAG, "RX_PACKETS_GOOD_BAD: %lu", (unsigned long)IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD);
    LOG_I(TAG, "RX_UNICAST_GOOD: %lu", (unsigned long)IP_GMAC_0->RX_UNICAST_PACKETS_GOOD);
    LOG_I(TAG, "RX_BROADCAST_GOOD: %lu", (unsigned long)IP_GMAC_0->RX_BROADCAST_PACKETS_GOOD);
    LOG_I(TAG, "RX_MULTICAST_GOOD: %lu", (unsigned long)IP_GMAC_0->RX_MULTICAST_PACKETS_GOOD);

    /* TX counters */
    LOG_I(TAG, "--- TX Counters ---");
    LOG_I(TAG, "TX_PACKETS_GOOD_BAD: %lu", (unsigned long)IP_GMAC_0->TX_PACKET_COUNT_GOOD_BAD);
    LOG_I(TAG, "TX_UNDERFLOW_ERROR: %lu", (unsigned long)IP_GMAC_0->TX_UNDERFLOW_ERROR_PACKETS);

    /* DMA Status */
    uint32_t dma_status = IP_GMAC_0->DMA_CH0_STATUS;
    uint32_t dma_rx_ctrl = IP_GMAC_0->DMA_CH0_RX_CONTROL;
    LOG_I(TAG, "--- DMA Status ---");
    LOG_I(TAG, "DMA_CH0_STATUS: 0x%08lX [TPS=%lu RPS=%lu]",
          (unsigned long)dma_status,
          (unsigned long)((dma_status >> 12) & 0xF),
          (unsigned long)((dma_status >> 8) & 0xF));
    LOG_I(TAG, "DMA_CH0_RX_CTRL: 0x%08lX [SR=%d]",
          (unsigned long)dma_rx_ctrl,
          (int)(dma_rx_ctrl & 1));

    /* MTL Debug */
    LOG_I(TAG, "--- MTL Debug ---");
    LOG_I(TAG, "MTL_RXQ0_DEBUG: 0x%08lX", (unsigned long)IP_GMAC_0->MTL_RXQ0_DEBUG);
    LOG_I(TAG, "MTL_TXQ0_DEBUG: 0x%08lX", (unsigned long)IP_GMAC_0->MTL_TXQ0_DEBUG);

    LOG_I(TAG, "=================================");
}


static void device_init(void) {
    OsIf_Init(NULL_PTR);

    Mcu_Init(NULL_PTR);
    Mcu_InitClock(McuClockSettingConfig_0);

#if (MCU_NO_PLL == STD_OFF)
    while (MCU_PLL_LOCKED != Mcu_GetPllStatus()) {}
    Mcu_DistributePllClock();
#endif

    Mcu_SetMode(McuModeSettingConf_0);

    Port_Init(NULL_PTR);
    Platform_Init(NULL_PTR);
    Uart_Init(NULL_PTR);
    log_init();

    LOG_I("INIT", "Setting RGMII mode...");

    /* Log BEFORE Eth_Init */
    LOG_I("INIT", "DCMRWF1 before Eth_Init: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF1);
    LOG_I("INIT", "DCMRWF3 before Eth_Init: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF3);

    Eth_Init(NULL_PTR);

    /* Log AFTER Eth_Init */
    LOG_I("INIT", "DCMRWF1 after Eth_Init: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF1);
    LOG_I("INIT", "DCMRWF3 after Eth_Init: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF3);

    /* Thử ghi và verify */
    IP_DCM_GPR->DCMRWF3 |= (1U << 0);  /* RX_CLK_MUX_BYPASS */
    LOG_I("INIT", "DCMRWF3 after set bypass: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF3);

    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    dcmrwf1 = (dcmrwf1 & ~0x7U) | 2U;  /* MAC_CONF_SEL = 2 */
    IP_DCM_GPR->DCMRWF1 = dcmrwf1;
    LOG_I("INIT", "DCMRWF1 after set RGMII: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF1);

    /* Force RGMII trong GMAC core */
    uint32_t ext_cfg = IP_GMAC_0->MAC_EXT_CONFIGURATION;
    ext_cfg |= (1U << 12);  /* Port Select = 1 (RGMII) */
    IP_GMAC_0->MAC_EXT_CONFIGURATION = ext_cfg;
    LOG_I("INIT", "MAC_EXT_CONFIG: 0x%08lX", (unsigned long)IP_GMAC_0->MAC_EXT_CONFIGURATION);

    /* Fix speed to 1Gbps - clear FES bit (bit 13) */
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    mac_cfg &= ~(1U << 13);  /* Clear FES = 1Gbps */
    mac_cfg |= (1U << 11);   /* Set DM = Full Duplex */
    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;
    LOG_I("INIT", "MAC_CONFIG after fix: 0x%08lX", (unsigned long)IP_GMAC_0->MAC_CONFIGURATION);
    uint8_t mac[6];
    Eth_43_GMAC_GetPhysAddr(0, mac);
    LOG_I(TAG, "GMAC MAC: %02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
void test_raw_tx(void)
{
    Eth_BufIdxType bufIdx;
    uint8_t* bufPtr;
    uint16_t bufLen = 64;

    LOG_I(TAG, "=== RAW TX Test ===");

    /* Get TX buffer */
    Std_ReturnType ret = Eth_43_GMAC_ProvideTxBuffer(0, 0, &bufIdx, &bufPtr, &bufLen);
    if (ret != E_OK) {
        LOG_E(TAG, "ProvideTxBuffer failed: %d", ret);
        return;
    }

    LOG_I(TAG, "Got buffer idx=%d, ptr=%p, len=%d", bufIdx, bufPtr, bufLen);

    /* Build broadcast ARP frame */
    uint8_t frame[64];
    memset(frame, 0, sizeof(frame));

    /* Ethernet header */
    memset(&frame[0], 0xFF, 6);           /* Dst: broadcast */
    frame[6] = 0x10; frame[7] = 0x11;     /* Src MAC */
    frame[8] = 0x22; frame[9] = 0x77;
    frame[10] = 0x77; frame[11] = 0x77;
    frame[12] = 0x08; frame[13] = 0x06;   /* EtherType: ARP */

    /* ARP payload */
    frame[14] = 0x00; frame[15] = 0x01;   /* Hardware: Ethernet */
    frame[16] = 0x08; frame[17] = 0x00;   /* Protocol: IPv4 */
    frame[18] = 0x06;                      /* HW size: 6 */
    frame[19] = 0x04;                      /* Proto size: 4 */
    frame[20] = 0x00; frame[21] = 0x01;   /* Opcode: Request */

    /* Sender MAC */
    frame[22] = 0x10; frame[23] = 0x11;
    frame[24] = 0x22; frame[25] = 0x77;
    frame[26] = 0x77; frame[27] = 0x77;

    /* Sender IP: 192.168.1.200 */
    frame[28] = 192; frame[29] = 168;
    frame[30] = 1;   frame[31] = 200;

    /* Target MAC: 00:00:00:00:00:00 */
    memset(&frame[32], 0, 6);

    /* Target IP: 192.168.1.1 */
    frame[38] = 192; frame[39] = 168;
    frame[40] = 1;   frame[41] = 1;

    memcpy(bufPtr, frame, 64);

    /* Send */
    ret = Eth_43_GMAC_Transmit(0, bufIdx, 0x0806, FALSE, 64, NULL);
    LOG_I(TAG, "Transmit result: %d", ret);

    /* Check TX counter */
    delay_ms(100);
    LOG_I(TAG, "TX Packets after: %lu", (unsigned long)IP_GMAC_0->TX_PACKET_COUNT_GOOD_BAD);
}

/* Thêm sau test_raw_tx loop trong test.c */
void debug_lan9646_mib(void)
{
    LOG_I(TAG, "=== LAN9646 MIB Counters ===");

    /* Port 6 TX counters */
    uint32_t p6_tx_uni, p6_tx_bcast, p6_rx_uni, p6_rx_bcast;

    /* Read Port 6 TX */
    lan9646_read_reg32(&g_lan9646, 0x6500, &p6_tx_uni);   /* MIB ctrl */

    /* Đọc MIB theo cách đúng - set index rồi đọc data */
    uint32_t ctrl, data;

    /* P6 TX Broadcast (index 0x63) */
    ctrl = (0x63 << 16) | 0x02000000;  /* Index + Read Enable */
    lan9646_write_reg32(&g_lan9646, 0x6500, ctrl);
    delay_ms(1);
    lan9646_read_reg32(&g_lan9646, 0x6504, &p6_tx_bcast);

    /* P6 TX Unicast (index 0x65) */
    ctrl = (0x65 << 16) | 0x02000000;
    lan9646_write_reg32(&g_lan9646, 0x6500, ctrl);
    delay_ms(1);
    lan9646_read_reg32(&g_lan9646, 0x6504, &p6_tx_uni);

    /* P6 RX Broadcast (index 0x0A) */
    ctrl = (0x0A << 16) | 0x02000000;
    lan9646_write_reg32(&g_lan9646, 0x6500, ctrl);
    delay_ms(1);
    lan9646_read_reg32(&g_lan9646, 0x6504, &p6_rx_bcast);

    /* P6 RX Unicast (index 0x0C) */
    ctrl = (0x0C << 16) | 0x02000000;
    lan9646_write_reg32(&g_lan9646, 0x6500, ctrl);
    delay_ms(1);
    lan9646_read_reg32(&g_lan9646, 0x6504, &p6_rx_uni);

    LOG_I(TAG, "Port 6: TX_Uni=%lu TX_Bcast=%lu RX_Uni=%lu RX_Bcast=%lu",
          (unsigned long)p6_tx_uni, (unsigned long)p6_tx_bcast,
          (unsigned long)p6_rx_uni, (unsigned long)p6_rx_bcast);

    /* Port 1 counters */
    uint32_t p1_tx_bcast, p1_rx_bcast;

    ctrl = (0x63 << 16) | 0x02000000;
    lan9646_write_reg32(&g_lan9646, 0x1500, ctrl);
    delay_ms(1);
    lan9646_read_reg32(&g_lan9646, 0x1504, &p1_tx_bcast);

    ctrl = (0x0A << 16) | 0x02000000;
    lan9646_write_reg32(&g_lan9646, 0x1500, ctrl);
    delay_ms(1);
    lan9646_read_reg32(&g_lan9646, 0x1504, &p1_rx_bcast);

    LOG_I(TAG, "Port 1: TX_Bcast=%lu RX_Bcast=%lu",
          (unsigned long)p1_tx_bcast, (unsigned long)p1_rx_bcast);
}


static void lan9646_init_device(void) {
    uint16_t chip_id;
    uint8_t revision;

    /* Init LAN9646 FIRST to provide RX_CLK */
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

    LOG_I(TAG, "LAN9646 ready, GMAC will be activated by lwIP");
}

/* lwIP test entry */
extern void start_example(void);

int main(void) {
    device_init();

    LOG_I(TAG, "");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "  LAN9646 + GMAC + lwIP");
    LOG_I(TAG, "========================================");

    debug_gmac_clock();

    lan9646_init_device();

    start_example();

    return 0;
}

