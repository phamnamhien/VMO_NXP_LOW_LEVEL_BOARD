/**
 * @file    main.c
 * @brief   RGMII Hardware Diagnostic - S32K388 + LAN9646
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
#include "rgmii_diag.h"
#include "rgmii_config_debug.h"

/* External config symbols from generated PBcfg files */
extern const Eth_43_GMAC_ConfigType Eth_43_GMAC_xPredefinedConfig;

#define TAG "MAIN"

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
/*                          DELAY FUNCTION                                    */
/*===========================================================================*/

static void delay_ms(uint32_t ms) {
    /* Simple busy-wait delay using loop counter */
    /* Calibrated for ~160MHz core clock */
    volatile uint32_t count;
    while (ms > 0) {
        count = 40000U;  /* Approx 1ms at 160MHz */
        while (count > 0) {
            count--;
        }
        ms--;
    }
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
        LOG_E(TAG, "LAN9646 init failed!");
        return lan9646ERR;
    }

    /* Read chip ID */
    uint16_t chip_id;
    uint8_t revision;
    lan9646_get_chip_id(&g_lan9646, &chip_id, &revision);
    LOG_I(TAG, "Chip: 0x%04X Rev:%d", chip_id, revision);

    /* Configure Port 6 for RGMII 1Gbps with RX delay */
    LOG_I(TAG, "Configuring Port 6 for RGMII 1Gbps...");

    /*
     * XMII_CTRL0 [0x6300]:
     *   Bit 6: Duplex (1=Full)
     *   Bit 5: TX Flow Ctrl (1=Enable)
     *   Bit 4: Speed 100 (0 for 1G mode)
     *   Bit 3: RX Flow Ctrl (1=Enable)
     * Value: 0x68 = Full duplex, flow control, 1Gbps
     */
    lan9646_write_reg8(&g_lan9646, 0x6300, 0x68);

    /*
     * XMII_CTRL1 [0x6301]:
     *   Bit 6: Speed 1000 (0=1Gbps, 1=10/100Mbps)
     *   Bit 4: RX internal delay (1=ON) - adds delay to RX path (GMAC TX -> LAN9646 RX)
     *   Bit 3: TX internal delay (1=ON) - adds delay to TX path (LAN9646 TX -> GMAC RX)
     *
     * For RGMII, ~2ns delay is needed on each path.
     * Since GMAC RX is not receiving packets, enable TX delay to fix RX path.
     * Value: 0x18 = 1Gbps mode, RX delay ON, TX delay ON
     */
    lan9646_write_reg8(&g_lan9646, 0x6301, 0x18);

    /* Enable switch */
    lan9646_write_reg8(&g_lan9646, 0x0300, 0x01);

    /* Port membership */
    lan9646_write_reg32(&g_lan9646, 0x6A04, 0x4F);
    lan9646_write_reg32(&g_lan9646, 0x1A04, 0x6E);
    lan9646_write_reg32(&g_lan9646, 0x2A04, 0x6D);
    lan9646_write_reg32(&g_lan9646, 0x3A04, 0x6B);
    lan9646_write_reg32(&g_lan9646, 0x4A04, 0x67);

    LOG_I(TAG, "LAN9646 ready");
    return lan9646OK;
}

/*===========================================================================*/
/*                          S32K388 GMAC INIT                                 */
/*===========================================================================*/

/*===========================================================================*/
/*                      DEBUG FUNCTIONS                                        */
/*===========================================================================*/

static void debug_gmac_rx_input_mux(void) {
    /*
     * Debug SIUL2 Input Mux Configuration for GMAC0 RX signals
     * SIUL2_0 base: 0x40290000
     * IMCR base offset: 0x0A40 (from Siul2_Port_Ip.h)
     *
     * GMAC0 RGMII RX IMCR indexes (from Port_PBcfg.c and pin config):
     * - IMCR 292: RXCTL  (MSCR 80 / PTC16)
     * - IMCR 294: RXD0   (MSCR 78 / PTC14)
     * - IMCR 295: RXD1   (MSCR 79 / PTC15)
     * - IMCR 300: RX_CLK (MSCR 54 / PTB22) - CRITICAL!
     * - IMCR 301: RXD2   (MSCR 55 / PTB23)
     * - IMCR 302: RXD3   (MSCR 56 / PTB24)
     */
    LOG_I(TAG, "");
    LOG_I(TAG, "=== GMAC0 RX Input Mux Debug ===");

    /* SIUL2_0 IMCR base address: 0x40290000 + 0x0A40 */
    volatile uint32_t *siul2_imcr = (volatile uint32_t *)(0x40290000UL + 0x0A40UL);

    /* Print GMAC0 RGMII RX IMCRs with correct indexes */
    LOG_I(TAG, "SIUL2_0 IMCR registers (GMAC0 RGMII RX):");
    LOG_I(TAG, "  IMCR[292] (RXCTL/PTC16)  = 0x%02lX", (unsigned long)(siul2_imcr[292] & 0x0F));
    LOG_I(TAG, "  IMCR[294] (RXD0/PTC14)   = 0x%02lX", (unsigned long)(siul2_imcr[294] & 0x0F));
    LOG_I(TAG, "  IMCR[295] (RXD1/PTC15)   = 0x%02lX", (unsigned long)(siul2_imcr[295] & 0x0F));
    LOG_I(TAG, "  IMCR[300] (RX_CLK/PTB22) = 0x%02lX <-- CRITICAL", (unsigned long)(siul2_imcr[300] & 0x0F));
    LOG_I(TAG, "  IMCR[301] (RXD2/PTB23)   = 0x%02lX", (unsigned long)(siul2_imcr[301] & 0x0F));
    LOG_I(TAG, "  IMCR[302] (RXD3/PTB24)   = 0x%02lX", (unsigned long)(siul2_imcr[302] & 0x0F));

    /* Also check MC_CGM MUX_7 for RX_CLK (should be bypassed for RGMII) */
    LOG_I(TAG, "");
    LOG_I(TAG, "MC_CGM MUX_7 (GMAC0_RX_CLK - should be bypassed):");
    LOG_I(TAG, "  MUX_7_CSC = 0x%08lX", (unsigned long)IP_MC_CGM->MUX_7_CSC);
    LOG_I(TAG, "  MUX_7_CSS = 0x%08lX", (unsigned long)IP_MC_CGM->MUX_7_CSS);
    LOG_I(TAG, "  MUX_7_DC_0 = 0x%08lX", (unsigned long)IP_MC_CGM->MUX_7_DC_0);

    /* Check GMAC PHY interface status */
    LOG_I(TAG, "");
    LOG_I(TAG, "GMAC0 PHY Interface Status:");
    LOG_I(TAG, "  MAC_PHYIF_CONTROL_STATUS = 0x%08lX",
          (unsigned long)IP_GMAC_0->MAC_PHYIF_CONTROL_STATUS);

    uint32_t phyif = IP_GMAC_0->MAC_PHYIF_CONTROL_STATUS;
    LOG_I(TAG, "    TC [0]   = %lu (TX config)", (unsigned long)(phyif & 1));
    LOG_I(TAG, "    LUD [1]  = %lu (Link Up/Down)", (unsigned long)((phyif >> 1) & 1));
    LOG_I(TAG, "    LNKSTS [19] = %lu (Link Status)", (unsigned long)((phyif >> 19) & 1));
    LOG_I(TAG, "    LNKSPEED [18:17] = %lu", (unsigned long)((phyif >> 17) & 3));

    /* Check DMA RX status */
    LOG_I(TAG, "");
    LOG_I(TAG, "GMAC0 DMA RX Status:");
    LOG_I(TAG, "  DMA_CH0_STATUS = 0x%08lX", (unsigned long)IP_GMAC_0->DMA_CH0_STATUS);
    LOG_I(TAG, "  DMA_CH0_RX_CONTROL = 0x%08lX", (unsigned long)IP_GMAC_0->DMA_CH0_RX_CONTROL);
    LOG_I(TAG, "  DMA_DEBUG_STATUS0 = 0x%08lX", (unsigned long)IP_GMAC_0->DMA_DEBUG_STATUS0);

    /* Check RX packet counters */
    LOG_I(TAG, "");
    LOG_I(TAG, "GMAC0 RX Packet Counters:");
    LOG_I(TAG, "  RX_PACKETS_COUNT_GOOD_BAD = %lu", (unsigned long)IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD);
    LOG_I(TAG, "  RX_OCTET_COUNT_GOOD = %lu", (unsigned long)IP_GMAC_0->RX_OCTET_COUNT_GOOD);
    LOG_I(TAG, "  RX_CRC_ERROR_PACKETS = %lu", (unsigned long)IP_GMAC_0->RX_CRC_ERROR_PACKETS);
    LOG_I(TAG, "  RX_ALIGNMENT_ERROR_PACKETS = %lu", (unsigned long)IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS);
    LOG_I(TAG, "  RX_RUNT_ERROR_PACKETS = %lu", (unsigned long)IP_GMAC_0->RX_RUNT_ERROR_PACKETS);
    LOG_I(TAG, "  RX_JABBER_ERROR_PACKETS = %lu", (unsigned long)IP_GMAC_0->RX_JABBER_ERROR_PACKETS);

    /* Check MTL RX queue status */
    LOG_I(TAG, "");
    LOG_I(TAG, "GMAC0 MTL RX Queue Status:");
    LOG_I(TAG, "  MTL_RXQ0_DEBUG = 0x%08lX", (unsigned long)IP_GMAC_0->MTL_RXQ0_DEBUG);
    uint32_t mtl_rxq0 = IP_GMAC_0->MTL_RXQ0_DEBUG;
    LOG_I(TAG, "    RXQSTS [5:4] = %lu (Queue state)", (unsigned long)((mtl_rxq0 >> 4) & 0x03));
    LOG_I(TAG, "    PRXQ [15:8]  = %lu (Packets in queue)", (unsigned long)((mtl_rxq0 >> 8) & 0xFF));

    LOG_I(TAG, "=================================");
    LOG_I(TAG, "");
}

static void configure_s32k388_rgmii(void) {
    /*
     * RGMII Clock Configuration:
     * - All clocks are configured via S32 Config Tool (MCU_Cfg.c)
     * - Only manual setting needed: GMAC RX_CLK MUX_7 bypass
     *
     * Per NXP engineer recommendation:
     * https://community.nxp.com/t5/S32K/S32K388-GMAC-with-RGMII/m-p/1999697
     */
    LOG_I(TAG, "Configuring S32K388 RGMII RX_CLK bypass (MUX_7)...");

    /* Bypass MUX_7 for GMAC0_RX_CLK - clock comes from external pin (LAN9646) */
    IP_DCM_GPR->DCMRWF3 |= DCM_GPR_DCMRWF3_MAC_RX_CLK_MUX_BYPASS(1u);

    LOG_I(TAG, "  DCMRWF3=0x%08lX (RX_CLK_BYPASS=%lu)",
          (unsigned long)IP_DCM_GPR->DCMRWF3,
          (unsigned long)(IP_DCM_GPR->DCMRWF3 & DCM_GPR_DCMRWF3_MAC_RX_CLK_MUX_BYPASS_MASK));

    /* Debug: Check RX input mux configuration */
    debug_gmac_rx_input_mux();
}

static void configure_gmac_mac(void) {
    LOG_I(TAG, "Configuring GMAC MAC for 1Gbps...");

    /*
     * MAC Configuration for 1Gbps Full Duplex:
     *   PS [15] = 0: 1000Mbps mode
     *   FES [14] = 0: Not used when PS=0 (1Gbps)
     *   DM [13] = 1: Full Duplex
     *   TE [1] = 1: Transmitter Enable
     *   RE [0] = 1: Receiver Enable
     */
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;

    /* Clear PS and FES for 1Gbps */
    mac_cfg &= ~(1U << 15);  /* PS = 0 for 1Gbps */
    mac_cfg &= ~(1U << 14);  /* FES = 0 for 1Gbps */

    /* Set other bits */
    mac_cfg |= (1U << 13);  /* DM: Full Duplex */
    mac_cfg |= (1U << 0);   /* RE: Receiver Enable */
    mac_cfg |= (1U << 1);   /* TE: Transmitter Enable */

    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;

    LOG_I(TAG, "  MAC_CFG=0x%08lX", (unsigned long)IP_GMAC_0->MAC_CONFIGURATION);
}

/*===========================================================================*/
/*                          DEBUG READBACK                                    */
/*===========================================================================*/

static void debug_readback_config(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "  CONFIGURATION READBACK VERIFICATION");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "");

    /* S32K388 GMAC0 */
    LOG_I(TAG, "--- S32K388 GMAC0 ---");

    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;

    LOG_I(TAG, "  DCM_GPR:");
    LOG_I(TAG, "    DCMRWF1 = 0x%08lX", (unsigned long)dcmrwf1);
    LOG_I(TAG, "      GMAC_INTF_MODE [1:0] = %lu -> %s",
          (unsigned long)(dcmrwf1 & 0x03U),
          ((dcmrwf1 & 0x03U) == 2) ? "RGMII" : "OTHER");

    LOG_I(TAG, "    DCMRWF3 = 0x%08lX", (unsigned long)dcmrwf3);
    LOG_I(TAG, "      TX_CLK_OUT_EN [3]     = %lu -> %s",
          (unsigned long)((dcmrwf3 >> 3) & 1),
          ((dcmrwf3 >> 3) & 1) ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "      RX_CLK_MUX_BYPASS [0] = %lu -> %s",
          (unsigned long)(dcmrwf3 & 1),
          (dcmrwf3 & 1) ? "BYPASS (from LAN9646)" : "MUX7");

    LOG_I(TAG, "  MAC_CONFIGURATION = 0x%08lX", (unsigned long)mac_cfg);
    LOG_I(TAG, "    PS  [15] = %lu", (unsigned long)((mac_cfg >> 15) & 1));
    LOG_I(TAG, "    FES [14] = %lu", (unsigned long)((mac_cfg >> 14) & 1));
    LOG_I(TAG, "    DM  [13] = %lu -> %s",
          (unsigned long)((mac_cfg >> 13) & 1),
          ((mac_cfg >> 13) & 1) ? "Full Duplex" : "Half Duplex");
    LOG_I(TAG, "    TE  [1]  = %lu -> %s",
          (unsigned long)((mac_cfg >> 1) & 1),
          ((mac_cfg >> 1) & 1) ? "TX ENABLED" : "TX DISABLED");
    LOG_I(TAG, "    RE  [0]  = %lu -> %s",
          (unsigned long)(mac_cfg & 1),
          (mac_cfg & 1) ? "RX ENABLED" : "RX DISABLED");

    /* Calculate effective speed */
    uint8_t ps = (mac_cfg >> 15) & 1;
    uint8_t fes = (mac_cfg >> 14) & 1;
    const char* speed;
    if (ps == 0) {
        speed = "1000 Mbps (1Gbps)";
    } else if (fes == 1) {
        speed = "100 Mbps";
    } else {
        speed = "10 Mbps";
    }
    LOG_I(TAG, "    -> Effective Speed: %s", speed);

    LOG_I(TAG, "");

    /* LAN9646 Port 6 */
    LOG_I(TAG, "--- LAN9646 Port 6 ---");

    uint8_t xmii_ctrl0, xmii_ctrl1, port_status;
    lan9646_read_reg8(&g_lan9646, 0x6300, &xmii_ctrl0);
    lan9646_read_reg8(&g_lan9646, 0x6301, &xmii_ctrl1);
    lan9646_read_reg8(&g_lan9646, 0x6030, &port_status);

    LOG_I(TAG, "  XMII_CTRL0 [0x6300] = 0x%02X", xmii_ctrl0);
    LOG_I(TAG, "    Duplex [6]       = %d -> %s",
          (xmii_ctrl0 >> 6) & 1,
          ((xmii_ctrl0 >> 6) & 1) ? "Full" : "Half");
    LOG_I(TAG, "    TX Flow Ctrl [5] = %d", (xmii_ctrl0 >> 5) & 1);
    LOG_I(TAG, "    Speed 100 [4]    = %d -> %s",
          (xmii_ctrl0 >> 4) & 1,
          ((xmii_ctrl0 >> 4) & 1) ? "100M mode" : "1G mode");
    LOG_I(TAG, "    RX Flow Ctrl [3] = %d", (xmii_ctrl0 >> 3) & 1);

    LOG_I(TAG, "  XMII_CTRL1 [0x6301] = 0x%02X", xmii_ctrl1);
    LOG_I(TAG, "    Speed 1000 [6]   = %d -> %s",
          (xmii_ctrl1 >> 6) & 1,
          ((xmii_ctrl1 >> 6) & 1) ? "10/100M mode" : "1Gbps mode");
    LOG_I(TAG, "    RX Delay [4]     = %d -> %s",
          (xmii_ctrl1 >> 4) & 1,
          ((xmii_ctrl1 >> 4) & 1) ? "ON" : "OFF");
    LOG_I(TAG, "    TX Delay [3]     = %d -> %s",
          (xmii_ctrl1 >> 3) & 1,
          ((xmii_ctrl1 >> 3) & 1) ? "ON" : "OFF");

    LOG_I(TAG, "  PORT_STATUS [0x6030] = 0x%02X", port_status);

    /* Calculate LAN9646 effective speed */
    uint8_t spd1000 = (xmii_ctrl1 >> 6) & 1;
    uint8_t spd100 = (xmii_ctrl0 >> 4) & 1;
    const char* lan_speed;
    if (spd1000 == 0) {
        lan_speed = "1000 Mbps (1Gbps)";
    } else if (spd100 == 1) {
        lan_speed = "100 Mbps";
    } else {
        lan_speed = "10 Mbps";
    }
    LOG_I(TAG, "    -> Effective Speed: %s", lan_speed);

    LOG_I(TAG, "");

    /* Verification summary */
    LOG_I(TAG, "--- VERIFICATION SUMMARY ---");
    uint8_t all_ok = 1;

    /* Check RGMII mode */
    if ((dcmrwf1 & 0x03U) != 2) {
        LOG_E(TAG, "  [FAIL] S32K388 not in RGMII mode!");
        all_ok = 0;
    } else {
        LOG_I(TAG, "  [OK] S32K388 RGMII mode");
    }

    /* Check RX_CLK bypass */
    if ((dcmrwf3 & 1) == 0) {
        LOG_E(TAG, "  [FAIL] RX_CLK bypass NOT enabled!");
        all_ok = 0;
    } else {
        LOG_I(TAG, "  [OK] RX_CLK bypass enabled");
    }

    /* Check TX_CLK output */
    if (((dcmrwf3 >> 3) & 1) == 0) {
        LOG_E(TAG, "  [FAIL] TX_CLK output NOT enabled!");
        all_ok = 0;
    } else {
        LOG_I(TAG, "  [OK] TX_CLK output enabled");
    }

    /* Check speed match */
    uint8_t gmac_1g = (ps == 0);
    uint8_t lan_1g = (spd1000 == 0);
    if (gmac_1g != lan_1g) {
        LOG_E(TAG, "  [FAIL] Speed mismatch! GMAC=%s, LAN9646=%s",
              gmac_1g ? "1G" : "100M",
              lan_1g ? "1G" : "100M");
        all_ok = 0;
    } else {
        LOG_I(TAG, "  [OK] Speed match: %s", gmac_1g ? "1Gbps" : "100Mbps");
    }

    /* Check LAN9646 RX delay */
    if (((xmii_ctrl1 >> 4) & 1) == 0) {
        LOG_W(TAG, "  [WARN] LAN9646 RX delay OFF - may need to be ON");
    } else {
        LOG_I(TAG, "  [OK] LAN9646 RX delay ON");
    }

    LOG_I(TAG, "");
    if (all_ok) {
        LOG_I(TAG, "==> ALL CONFIGURATIONS VERIFIED OK");
    } else {
        LOG_E(TAG, "==> CONFIGURATION ERRORS DETECTED!");
    }
    LOG_I(TAG, "");
}

/*===========================================================================*/
/*                          DEVICE INIT                                       */
/*===========================================================================*/

static void device_init(void) {
    /* Initialize in correct order (matching working pattern) */
    OsIf_Init(NULL_PTR);
    Port_Init(NULL_PTR);

    Mcu_Init(NULL_PTR);
    Mcu_InitClock(McuClockSettingConfig_0);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {}
    Mcu_DistributePllClock();
    Mcu_SetMode(McuModeSettingConf_0);

    Platform_Init(NULL_PTR);

#if (OSIF_USE_SYSTEM_TIMER == STD_ON)
    /* Initialize GPT for OsIf system timer (optional - requires proper S32 Config Tool setup) */
    Gpt_Init(NULL_PTR);
    Gpt_StartTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0, 40000000U);
    Gpt_EnableNotification(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);
    OsIf_SetTimerFrequency(160000000U, OSIF_COUNTER_SYSTEM);
#endif

    Uart_Init(NULL_PTR);
    log_init();

    /* Print banner */
    LOG_I(TAG, "");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "      RGMII 1Gbps HARDWARE DIAGNOSTIC - S32K388 + LAN9646");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "");
    LOG_I(TAG, "[Step 1] MCU Init... Done");
    LOG_I(TAG, "[Step 2] Platform & Port Init... Done");

    /* Step 3: Init LAN9646 */
    LOG_I(TAG, "[Step 3] Init LAN9646...");
    if (init_lan9646() != lan9646OK) {
        LOG_E(TAG, "FATAL: LAN9646 init failed!");
        while (1) {}
    }

    /* Step 4: Init Ethernet (AUTOSAR)
     * NOTE: Clock configuration is handled by S32 Config Tool (Mcu_InitClock)
     */
    LOG_I(TAG, "[Step 4] Init Ethernet...");
    Eth_43_GMAC_Init(&Eth_43_GMAC_xPredefinedConfig);

    /* Step 5: Configure GMAC MAC */
    LOG_I(TAG, "[Step 5] Configure GMAC MAC...");
    configure_gmac_mac();

    /* Step 6: Set controller active */
    LOG_I(TAG, "[Step 6] Activate Ethernet controller...");
    Eth_43_GMAC_SetControllerMode(ETH_CTRL_IDX, ETH_MODE_ACTIVE);

    /* Step 7: Configure S32K388 RGMII (AFTER Eth init to avoid being overwritten) */
    LOG_I(TAG, "[Step 7] Configure S32K388 RGMII bypass...");
    configure_s32k388_rgmii();

    /* Step 8: Debug readback - verify all configurations */
    LOG_I(TAG, "[Step 8] Verifying configurations...");
    debug_readback_config();

    LOG_I(TAG, "");
    LOG_I(TAG, "Device initialization complete!");
    LOG_I(TAG, "");
}

/*===========================================================================*/
/*                          MAIN                                              */
/*===========================================================================*/

int main(void) {
    /* Initialize hardware */
    device_init();

    /* Small delay for hardware to stabilize */
    delay_ms(100);

    /* Initialize diagnostic modules */
    rgmii_diag_init(&g_lan9646, delay_ms);
    rgmii_debug_init(&g_lan9646, delay_ms);

    LOG_I(TAG, "");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "  SELECT DEBUG MODE:");
    LOG_I(TAG, "  1. Quick Summary (rgmii_debug_quick_summary)");
    LOG_I(TAG, "  2. Full Configuration Dump (rgmii_debug_dump_all)");
    LOG_I(TAG, "  3. Full Diagnostic with Tests (rgmii_debug_full_diagnostic)");
    LOG_I(TAG, "  4. Original RGMII Diagnostic (rgmii_diag_run_all)");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "");

    /* --- Run Quick Summary first for overview --- */
    LOG_I(TAG, "Running Quick Summary...");
    rgmii_debug_quick_summary();

    /* --- Run Full Configuration Dump --- */
    LOG_I(TAG, "");
    LOG_I(TAG, "Running Full Configuration Dump...");
    delay_ms(100);
    rgmii_debug_dump_all();

    /* --- Run Full Diagnostic with Tests --- */
    LOG_I(TAG, "");
    LOG_I(TAG, "Running Full Diagnostic with Tests...");
    delay_ms(100);
    rgmii_debug_full_diagnostic();

    /* --- Also run original diagnostic for comparison --- */
    LOG_I(TAG, "");
    LOG_I(TAG, "Running Original RGMII Diagnostic...");
    delay_ms(100);
    rgmii_test_result_t result = rgmii_diag_run_all();

    LOG_I(TAG, "");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "                    FINAL SUMMARY");
    LOG_I(TAG, "================================================================");
    if (result == RGMII_TEST_PASS) {
        LOG_I(TAG, "  RGMII DIAGNOSTIC: ALL TESTS PASSED!");
        LOG_I(TAG, "  Hardware is working correctly.");
    } else {
        LOG_E(TAG, "  RGMII DIAGNOSTIC: ISSUES DETECTED");
        LOG_E(TAG, "  Result: %s", rgmii_diag_result_str(result));
        LOG_E(TAG, "");
        LOG_E(TAG, "  Recommended Actions:");
        LOG_E(TAG, "  1. Check the configuration dump above");
        LOG_E(TAG, "  2. Review the timing sweep results");
        LOG_E(TAG, "  3. See troubleshooting guide above");
    }
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "");

    /* Infinite loop with periodic status */
    uint32_t loop_count = 0;
    while (1) {
        delay_ms(5000);
        loop_count++;

        LOG_I(TAG, "[%lu] System running...", (unsigned long)loop_count);

        /* Every 60 seconds, print quick summary */
        if (loop_count % 12 == 0) {
            LOG_I(TAG, "--- Periodic Status Check ---");
            rgmii_debug_quick_summary();
        }
    }

    return 0;
}

