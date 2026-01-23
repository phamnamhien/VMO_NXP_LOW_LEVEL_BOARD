/**
 * @file    rgmii_config_debug.c
 * @brief   RGMII Configuration Debug Module Implementation
 */

#include "rgmii_config_debug.h"
#include "log_debug.h"
#include "S32K388.h"
#include "Gmac_Ip.h"
#include <string.h>
#include <stdio.h>

/* Helper function to send a test packet */
static Gmac_Ip_StatusType send_test_packet_helper(uint8_t* data, uint16_t len) {
    Gmac_Ip_BufferType buf;
    buf.Data = data;
    buf.Length = len;
    return Gmac_Ip_SendFrame(0, 0, &buf, NULL);
}

#define TAG "RGMII_DBG"

/*===========================================================================*/
/*                          PRIVATE DATA                                      */
/*===========================================================================*/

static lan9646_t* g_lan = NULL;
static void (*g_delay)(uint32_t) = NULL;

/* Test packet for loopback */
static uint8_t g_test_packet[64] = {
    /* Dest MAC: Broadcast */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* Src MAC: Test */
    0x10, 0x11, 0x22, 0x33, 0x44, 0x55,
    /* EtherType: Custom test */
    0x88, 0xB5,
    /* Payload pattern */
    0x00, 0xFF, 0xAA, 0x55, 0x0F, 0xF0, 0x33, 0xCC,
    0x00, 0xFF, 0xAA, 0x55, 0x0F, 0xF0, 0x33, 0xCC,
    0x00, 0xFF, 0xAA, 0x55, 0x0F, 0xF0, 0x33, 0xCC,
    0x00, 0xFF, 0xAA, 0x55, 0x0F, 0xF0, 0x33, 0xCC,
    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
};

/*===========================================================================*/
/*                          HELPER MACROS                                     */
/*===========================================================================*/

#define SEPARATOR(title) do { \
    LOG_I(TAG, ""); \
    LOG_I(TAG, "================================================================"); \
    LOG_I(TAG, "  %s", title); \
    LOG_I(TAG, "================================================================"); \
} while(0)

#define PRINT_REG32(name, addr) do { \
    LOG_I(TAG, "  [0x%08lX] %-28s = 0x%08lX", \
          (unsigned long)(addr), name, (unsigned long)(*(volatile uint32_t*)(addr))); \
} while(0)

/*===========================================================================*/
/*                          LAN9646 HELPER FUNCTIONS                          */
/*===========================================================================*/

static uint8_t lan_read8(uint16_t addr) {
    uint8_t val = 0;
    if (g_lan) lan9646_read_reg8(g_lan, addr, &val);
    return val;
}

static void lan_write8(uint16_t addr, uint8_t val) {
    if (g_lan) lan9646_write_reg8(g_lan, addr, val);
}

static uint32_t lan_read32(uint16_t addr) {
    uint32_t val = 0;
    if (g_lan) lan9646_read_reg32(g_lan, addr, &val);
    return val;
}

/* Read MIB counter using indirect access */
static uint32_t lan_read_mib(uint8_t port, uint8_t index) {
    if (!g_lan) return 0;

    uint16_t base = (uint16_t)port << 12;
    uint32_t ctrl, data = 0;
    uint32_t timeout = 1000;

    /* Set MIB index and read enable */
    ctrl = ((uint32_t)index << 16) | 0x02000000UL;  /* bit25 = Read Enable */
    lan9646_write_reg32(g_lan, base | 0x0500, ctrl);

    /* Wait for read complete */
    do {
        lan9646_read_reg32(g_lan, base | 0x0500, &ctrl);
        if (--timeout == 0) break;
    } while (ctrl & 0x02000000UL);

    /* Read data */
    lan9646_read_reg32(g_lan, base | 0x0504, &data);
    return data;
}

/*===========================================================================*/
/*                          INITIALIZATION                                    */
/*===========================================================================*/

void rgmii_debug_init(lan9646_t* lan, void (*delay_ms)(uint32_t)) {
    g_lan = lan;
    g_delay = delay_ms;
}

/*===========================================================================*/
/*                          STRING CONVERSION                                 */
/*===========================================================================*/

const char* rgmii_debug_speed_str(rgmii_speed_t speed) {
    switch (speed) {
        case RGMII_SPEED_10M:   return "10 Mbps";
        case RGMII_SPEED_100M:  return "100 Mbps";
        case RGMII_SPEED_1000M: return "1000 Mbps";
        default:               return "Unknown";
    }
}

const char* rgmii_debug_duplex_str(rgmii_duplex_t duplex) {
    switch (duplex) {
        case RGMII_DUPLEX_HALF: return "Half";
        case RGMII_DUPLEX_FULL: return "Full";
        default:               return "Unknown";
    }
}

const char* rgmii_debug_interface_str(uint8_t mode) {
    /* S32K388 MAC_CONF_SEL values (different from other S32K3 variants!) */
    switch (mode) {
        case 0: return "MII";
        case 1: return "RGMII";   /* S32K388 specific: 1 = RGMII (not RMII!) */
        case 2: return "RMII";
        default: return "Unknown";
    }
}

const char* rgmii_debug_delay_str(rgmii_delay_mode_t mode) {
    switch (mode) {
        case RGMII_DELAY_NONE:    return "No Delay";
        case RGMII_DELAY_TX_ONLY: return "TX Delay Only";
        case RGMII_DELAY_RX_ONLY: return "RX Delay Only";
        case RGMII_DELAY_BOTH:    return "TX+RX Delay";
        default:                  return "Unknown";
    }
}

/*===========================================================================*/
/*                          S32K388 CONFIGURATION READ                        */
/*===========================================================================*/

void rgmii_debug_read_s32k388_config(s32k388_gmac_config_t* config) {
    if (!config) return;

    memset(config, 0, sizeof(s32k388_gmac_config_t));

    /* DCM_GPR Registers */
    config->dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    config->dcmrwf3 = IP_DCM_GPR->DCMRWF3;

    /* MC_CGM Clock Mux 8 - Use SDK register definitions */
    config->mux8_csc = IP_MC_CGM->MUX_8_CSC;
    config->mux8_css = IP_MC_CGM->MUX_8_CSS;
    config->mux8_dc0 = IP_MC_CGM->MUX_8_DC_0;

    /* GMAC MAC Registers */
    config->mac_configuration = IP_GMAC_0->MAC_CONFIGURATION;
    config->mac_version = IP_GMAC_0->MAC_VERSION;
    config->mac_hw_feature0 = IP_GMAC_0->MAC_HW_FEATURE0;
    config->mac_hw_feature1 = IP_GMAC_0->MAC_HW_FEATURE1;
    config->mac_hw_feature2 = IP_GMAC_0->MAC_HW_FEATURE2;
    config->mac_hw_feature3 = IP_GMAC_0->MAC_HW_FEATURE3;
    config->mac_addr_high = IP_GMAC_0->MAC_ADDRESS0_HIGH;
    config->mac_addr_low = IP_GMAC_0->MAC_ADDRESS0_LOW;
    config->mac_debug = IP_GMAC_0->MAC_DEBUG;

    /* DMA Registers */
    config->dma_mode = IP_GMAC_0->DMA_MODE;
    config->dma_sysbus_mode = IP_GMAC_0->DMA_SYSBUS_MODE;
    config->dma_ch0_control = IP_GMAC_0->DMA_CH0_CONTROL;
    config->dma_ch0_tx_control = IP_GMAC_0->DMA_CH0_TX_CONTROL;
    config->dma_ch0_rx_control = IP_GMAC_0->DMA_CH0_RX_CONTROL;

    /* MTL Registers */
    config->mtl_operation_mode = IP_GMAC_0->MTL_OPERATION_MODE;
    config->mtl_txq0_operation_mode = IP_GMAC_0->MTL_TXQ0_OPERATION_MODE;
    config->mtl_rxq0_operation_mode = IP_GMAC_0->MTL_RXQ0_OPERATION_MODE;

    /* Parse MAC Configuration */
    uint32_t mac_cfg = config->mac_configuration;

    /* Speed: PS (bit15) + FES (bit14) */
    /* PS=1, FES=1 -> 100M; PS=1, FES=0 -> 10M; PS=0 -> 1000M */
    bool ps = (mac_cfg >> 15) & 1;
    bool fes = (mac_cfg >> 14) & 1;
    if (!ps) {
        config->speed = RGMII_SPEED_1000M;
    } else if (fes) {
        config->speed = RGMII_SPEED_100M;
    } else {
        config->speed = RGMII_SPEED_10M;
    }

    /* Duplex: DM (bit 13) */
    config->duplex = ((mac_cfg >> 13) & 1) ? RGMII_DUPLEX_FULL : RGMII_DUPLEX_HALF;

    /* TX/RX Enable */
    config->rx_enable = (mac_cfg >> 0) & 1;  /* RE bit 0 */
    config->tx_enable = (mac_cfg >> 1) & 1;  /* TE bit 1 */

    /* Loopback */
    config->loopback = (mac_cfg >> 12) & 1;  /* LM bit 12 */

    /* Interface mode from DCM_GPR */
    config->interface_mode = config->dcmrwf1 & 0x03;

    /* Clock settings - correct bit positions for DCMRWF3:
     * Bit 13: MAC_RX_CLK_MUX_BYPASS
     * Bit 12: MAC_TX_CLK_MUX_BYPASS
     * Bit 11: MAC_TX_CLK_OUT_EN
     */
    config->rx_clk_bypass = (config->dcmrwf3 >> 13) & 1;
    config->tx_clk_out_enable = (config->dcmrwf3 >> 11) & 1;
}

/*===========================================================================*/
/*                          LAN9646 CONFIGURATION READ                        */
/*===========================================================================*/

void rgmii_debug_read_lan9646_config(lan9646_port6_config_t* config) {
    if (!config || !g_lan) return;

    memset(config, 0, sizeof(lan9646_port6_config_t));

    /* Chip ID */
    lan9646_get_chip_id(g_lan, &config->chip_id, &config->revision);

    /* Port 6 XMII Control */
    config->xmii_ctrl0 = lan_read8(0x6300);
    config->xmii_ctrl1 = lan_read8(0x6301);

    /* Port 6 Status */
    config->port_status = lan_read8(0x6030);

    /* Port 6 Operation Control */
    config->op_ctrl0 = lan_read8(0x6020);
    config->op_ctrl1 = lan_read8(0x6021);

    /* Port 6 MSTP State */
    config->mstp_state = lan_read8(0x6B04);

    /* Port 6 Membership */
    config->membership = lan_read32(0x6A04);

    /* Port 6 MAC Control */
    config->mac_ctrl0 = lan_read8(0x6400);
    config->mac_ctrl1 = lan_read8(0x6401);

    /* Switch Operation */
    config->switch_op = lan_read8(0x0300);

    /* Parse XMII_CTRL0 */
    config->duplex = (config->xmii_ctrl0 & 0x40) ? RGMII_DUPLEX_FULL : RGMII_DUPLEX_HALF;
    config->tx_flow_ctrl = (config->xmii_ctrl0 & 0x20) != 0;
    config->rx_flow_ctrl = (config->xmii_ctrl0 & 0x08) != 0;

    /* Parse XMII_CTRL1 for speed */
    bool speed_1000 = !(config->xmii_ctrl1 & 0x40);  /* bit6=0 means 1000M */
    bool speed_100 = (config->xmii_ctrl0 & 0x10) != 0;

    if (speed_1000) {
        config->speed = RGMII_SPEED_1000M;
    } else if (speed_100) {
        config->speed = RGMII_SPEED_100M;
    } else {
        config->speed = RGMII_SPEED_10M;
    }

    /* Parse delays */
    config->tx_delay = (config->xmii_ctrl1 & 0x08) != 0;  /* bit3 */
    config->rx_delay = (config->xmii_ctrl1 & 0x10) != 0;  /* bit4 */

    /* Parse MSTP state */
    config->tx_enable = (config->mstp_state & 0x04) != 0;
    config->rx_enable = (config->mstp_state & 0x02) != 0;
    config->learning_enable = !(config->mstp_state & 0x01);
}

/*===========================================================================*/
/*                          CONFIGURATION SNAPSHOT                            */
/*===========================================================================*/

void rgmii_debug_read_snapshot(rgmii_config_snapshot_t* snapshot) {
    if (!snapshot) return;

    memset(snapshot, 0, sizeof(rgmii_config_snapshot_t));

    rgmii_debug_read_s32k388_config(&snapshot->s32k388);
    rgmii_debug_read_lan9646_config(&snapshot->lan9646);

    /* Validate configurations */
    rgmii_debug_validate(snapshot);
}

/*===========================================================================*/
/*                          VALIDATION                                        */
/*===========================================================================*/

bool rgmii_debug_validate(rgmii_config_snapshot_t* snapshot) {
    if (!snapshot) return false;

    char* rec = snapshot->recommendations;
    rec[0] = '\0';

    /* Check speed match */
    snapshot->speed_match = (snapshot->s32k388.speed == snapshot->lan9646.speed);
    if (!snapshot->speed_match) {
        snprintf(rec + strlen(rec), sizeof(snapshot->recommendations) - strlen(rec),
                 "- Speed mismatch: S32K388=%s, LAN9646=%s\n",
                 rgmii_debug_speed_str(snapshot->s32k388.speed),
                 rgmii_debug_speed_str(snapshot->lan9646.speed));
    }

    /* Check duplex match */
    snapshot->duplex_match = (snapshot->s32k388.duplex == snapshot->lan9646.duplex);
    if (!snapshot->duplex_match) {
        snprintf(rec + strlen(rec), sizeof(snapshot->recommendations) - strlen(rec),
                 "- Duplex mismatch: S32K388=%s, LAN9646=%s\n",
                 rgmii_debug_duplex_str(snapshot->s32k388.duplex),
                 rgmii_debug_duplex_str(snapshot->lan9646.duplex));
    }

    /* Check interface mode (S32K388: MAC_CONF_SEL = 1 for RGMII!) */
    snapshot->interface_valid = (snapshot->s32k388.interface_mode == 1);
    if (!snapshot->interface_valid) {
        snprintf(rec + strlen(rec), sizeof(snapshot->recommendations) - strlen(rec),
                 "- S32K388 not in RGMII mode! MAC_CONF_SEL=%d (expected 1). Current: %s\n",
                 snapshot->s32k388.interface_mode,
                 rgmii_debug_interface_str(snapshot->s32k388.interface_mode));
    }

    /* Check clock configuration */
    snapshot->clocks_valid = snapshot->s32k388.tx_clk_out_enable &&
                             snapshot->s32k388.rx_clk_bypass;
    if (!snapshot->clocks_valid) {
        if (!snapshot->s32k388.tx_clk_out_enable) {
            snprintf(rec + strlen(rec), sizeof(snapshot->recommendations) - strlen(rec),
                     "- TX_CLK output not enabled (DCMRWF3[11])\n");
        }
        if (!snapshot->s32k388.rx_clk_bypass) {
            snprintf(rec + strlen(rec), sizeof(snapshot->recommendations) - strlen(rec),
                     "- RX_CLK bypass not enabled (DCMRWF3[13])\n");
        }
    }

    /* Check delay configuration (complementary) */
    /* For RGMII without PCB delays, typically need delay on one side only */
    /* This is board-dependent, so we just note the current setting */
    snapshot->delay_valid = true;  /* No strict rule, board-dependent */

    /* Overall validation */
    snapshot->overall_valid = snapshot->speed_match &&
                              snapshot->duplex_match &&
                              snapshot->interface_valid &&
                              snapshot->clocks_valid;

    if (snapshot->overall_valid && rec[0] == '\0') {
        snprintf(rec, sizeof(snapshot->recommendations),
                 "Configuration looks correct! All checks passed.\n");
    }

    return snapshot->overall_valid;
}

/*===========================================================================*/
/*                          S32K388 DUMP                                      */
/*===========================================================================*/

void rgmii_debug_dump_s32k388(void) {
    s32k388_gmac_config_t cfg;
    rgmii_debug_read_s32k388_config(&cfg);

    SEPARATOR("S32K388 GMAC0 CONFIGURATION");

    /* DCM_GPR Section */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- DCM_GPR (RGMII Control) ---");
    LOG_I(TAG, "  DCMRWF1 = 0x%08lX", (unsigned long)cfg.dcmrwf1);
    LOG_I(TAG, "    GMAC_INTF_MODE [1:0] = %lu -> %s",
          (unsigned long)(cfg.dcmrwf1 & 0x03),
          rgmii_debug_interface_str(cfg.dcmrwf1 & 0x03));
    LOG_I(TAG, "");
    LOG_I(TAG, "  DCMRWF3 = 0x%08lX", (unsigned long)cfg.dcmrwf3);
    LOG_I(TAG, "    GMAC_RX_CLK_MUX_BYPASS [13] = %d -> RX clock: %s",
          (cfg.dcmrwf3 >> 13) & 1, cfg.rx_clk_bypass ? "BYPASS (from PHY)" : "MUX7");
    LOG_I(TAG, "    GMAC_TX_CLK_MUX_BYPASS [12] = %d -> TX clock: %s",
          (cfg.dcmrwf3 >> 12) & 1, ((cfg.dcmrwf3 >> 12) & 1) ? "BYPASS" : "MUX8");
    LOG_I(TAG, "    GMAC_TX_CLK_OUT_EN     [11] = %d -> TX clock output: %s",
          (cfg.dcmrwf3 >> 11) & 1, cfg.tx_clk_out_enable ? "ENABLED" : "DISABLED");

    /* MC_CGM Clock Section */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- MC_CGM MUX_8 (GMAC0_TX_CLK) ---");
    LOG_I(TAG, "  MUX_8_CSC = 0x%08lX", (unsigned long)cfg.mux8_csc);
    LOG_I(TAG, "  MUX_8_CSS = 0x%08lX", (unsigned long)cfg.mux8_css);
    LOG_I(TAG, "  MUX_8_DC_0 = 0x%08lX", (unsigned long)cfg.mux8_dc0);

    uint8_t clk_src = (cfg.mux8_css >> 24) & 0x3F;
    bool div_en = (cfg.mux8_dc0 >> 31) & 1;
    uint8_t div_val = (cfg.mux8_dc0 >> 16) & 0xFF;

    LOG_I(TAG, "    Clock Source Select = %d", clk_src);
    LOG_I(TAG, "    Divider Enable = %d", div_en);
    LOG_I(TAG, "    Divider Value = %d (divide by %d)", div_val, div_val + 1);

    /* Calculate expected TX clock */
    const char* clk_src_name;
    uint32_t base_freq = 0;
    /*
     * S32K388 MUX_8 clock source selector values (from Clock_Ip_apfFreqTableClkSrc):
     *   0  = FIRC (48MHz)
     *   1  = SIRC (32kHz)
     *   2  = FXOSC (16-40MHz, board dependent)
     *   8  = PLL_PHI0 (CORE_PLL)
     *   9  = PLL_PHI1 (CORE_PLL)
     *   12 = PLLAUX_PHI0 (typically 125MHz for RGMII 1Gbps)
     *   13 = PLLAUX_PHI1
     *   14 = PLLAUX_PHI2
     *   18 = GMAC0_RX_CLK_EXT
     *   19 = GMAC0_TX_CLK_EXT
     *   20 = GMAC0_REF_CLK
     */
    switch (clk_src) {
        case 0:  clk_src_name = "FIRC (48MHz)"; base_freq = 48000000; break;
        case 1:  clk_src_name = "SIRC (32kHz)"; base_freq = 32000; break;
        case 2:  clk_src_name = "FXOSC"; base_freq = 40000000; break;
        case 8:  clk_src_name = "PLL_PHI0 (CORE_PLL)"; base_freq = 160000000; break;
        case 9:  clk_src_name = "PLL_PHI1 (CORE_PLL)"; base_freq = 80000000; break;
        case 12: clk_src_name = "PLLAUX_PHI0"; base_freq = 125000000; break;  /* Typical for RGMII */
        case 13: clk_src_name = "PLLAUX_PHI1"; base_freq = 125000000; break;
        case 14: clk_src_name = "PLLAUX_PHI2"; base_freq = 250000000; break;
        case 18: clk_src_name = "GMAC0_RX_CLK_EXT"; base_freq = 0; break;
        case 19: clk_src_name = "GMAC0_TX_CLK_EXT"; base_freq = 0; break;
        case 20: clk_src_name = "GMAC0_REF_CLK"; base_freq = 50000000; break;
        default: clk_src_name = "Unknown"; base_freq = 0; break;
    }
    LOG_I(TAG, "    Source: %s", clk_src_name);

    if (base_freq > 0 && div_en) {
        uint32_t tx_clk = base_freq / (div_val + 1);
        LOG_I(TAG, "    Calculated TX_CLK: %lu Hz (%lu MHz)",
              (unsigned long)tx_clk, (unsigned long)(tx_clk / 1000000));
    }

    /* MAC Configuration Section */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- GMAC MAC Configuration ---");
    LOG_I(TAG, "  MAC_CONFIGURATION = 0x%08lX", (unsigned long)cfg.mac_configuration);
    LOG_I(TAG, "    RE  [0]  = %d -> Receiver: %s", cfg.rx_enable, cfg.rx_enable ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "    TE  [1]  = %d -> Transmitter: %s", cfg.tx_enable, cfg.tx_enable ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "    LM  [12] = %d -> Loopback: %s", cfg.loopback, cfg.loopback ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "    DM  [13] = %d -> Duplex: %s",
          cfg.duplex, rgmii_debug_duplex_str(cfg.duplex));
    LOG_I(TAG, "    FES [14] = %d -> Fast Ethernet Speed (100M when PS=1)",
          (cfg.mac_configuration >> 14) & 1);
    LOG_I(TAG, "    PS  [15] = %d -> Port Select (1=10/100M mode)",
          (cfg.mac_configuration >> 15) & 1);
    LOG_I(TAG, "    -> Effective Speed: %s", rgmii_debug_speed_str(cfg.speed));

    /* MAC Version */
    LOG_I(TAG, "");
    LOG_I(TAG, "  MAC_VERSION = 0x%08lX", (unsigned long)cfg.mac_version);
    LOG_I(TAG, "    SNPSVER = %lu.%lu",
          (unsigned long)((cfg.mac_version >> 8) & 0xFF),
          (unsigned long)(cfg.mac_version & 0xFF));

    /* MAC Address */
    LOG_I(TAG, "");
    LOG_I(TAG, "  MAC Address:");
    LOG_I(TAG, "    MAC_ADDRESS0_HIGH = 0x%08lX", (unsigned long)cfg.mac_addr_high);
    LOG_I(TAG, "    MAC_ADDRESS0_LOW  = 0x%08lX", (unsigned long)cfg.mac_addr_low);
    LOG_I(TAG, "    -> %02X:%02X:%02X:%02X:%02X:%02X",
          (uint8_t)(cfg.mac_addr_low & 0xFF),
          (uint8_t)((cfg.mac_addr_low >> 8) & 0xFF),
          (uint8_t)((cfg.mac_addr_low >> 16) & 0xFF),
          (uint8_t)((cfg.mac_addr_low >> 24) & 0xFF),
          (uint8_t)(cfg.mac_addr_high & 0xFF),
          (uint8_t)((cfg.mac_addr_high >> 8) & 0xFF));

    /* MAC Debug */
    LOG_I(TAG, "");
    LOG_I(TAG, "  MAC_DEBUG = 0x%08lX", (unsigned long)cfg.mac_debug);
    LOG_I(TAG, "    RFCFCSTS [1:0] = %lu (RX FIFO fill level)",
          (unsigned long)(cfg.mac_debug & 0x03));
    LOG_I(TAG, "    RPESTS [0]     = %lu (GMII/RGMII receive active)",
          (unsigned long)((cfg.mac_debug >> 0) & 0x01));
    LOG_I(TAG, "    TPESTS [16]    = %lu (GMII/RGMII transmit active)",
          (unsigned long)((cfg.mac_debug >> 16) & 0x01));

    /* DMA Configuration */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- DMA Configuration ---");
    LOG_I(TAG, "  DMA_MODE          = 0x%08lX", (unsigned long)cfg.dma_mode);
    LOG_I(TAG, "  DMA_SYSBUS_MODE   = 0x%08lX", (unsigned long)cfg.dma_sysbus_mode);
    LOG_I(TAG, "  DMA_CH0_CONTROL   = 0x%08lX", (unsigned long)cfg.dma_ch0_control);
    LOG_I(TAG, "  DMA_CH0_TX_CTRL   = 0x%08lX", (unsigned long)cfg.dma_ch0_tx_control);
    LOG_I(TAG, "  DMA_CH0_RX_CTRL   = 0x%08lX", (unsigned long)cfg.dma_ch0_rx_control);

    /* MTL Configuration */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- MTL Configuration ---");
    LOG_I(TAG, "  MTL_OPERATION_MODE      = 0x%08lX", (unsigned long)cfg.mtl_operation_mode);
    LOG_I(TAG, "  MTL_TXQ0_OPERATION_MODE = 0x%08lX", (unsigned long)cfg.mtl_txq0_operation_mode);
    LOG_I(TAG, "  MTL_RXQ0_OPERATION_MODE = 0x%08lX", (unsigned long)cfg.mtl_rxq0_operation_mode);

    /* Hardware Features */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Hardware Features ---");
    LOG_I(TAG, "  MAC_HW_FEATURE0 = 0x%08lX", (unsigned long)cfg.mac_hw_feature0);
    LOG_I(TAG, "  MAC_HW_FEATURE1 = 0x%08lX", (unsigned long)cfg.mac_hw_feature1);
    LOG_I(TAG, "  MAC_HW_FEATURE2 = 0x%08lX", (unsigned long)cfg.mac_hw_feature2);
    LOG_I(TAG, "  MAC_HW_FEATURE3 = 0x%08lX", (unsigned long)cfg.mac_hw_feature3);

    /* Summary */
    LOG_I(TAG, "");
    LOG_I(TAG, "=== S32K388 GMAC0 SUMMARY ===");
    LOG_I(TAG, "  Interface: %s", rgmii_debug_interface_str(cfg.interface_mode));
    LOG_I(TAG, "  Speed:     %s", rgmii_debug_speed_str(cfg.speed));
    LOG_I(TAG, "  Duplex:    %s", rgmii_debug_duplex_str(cfg.duplex));
    LOG_I(TAG, "  TX:        %s", cfg.tx_enable ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "  RX:        %s", cfg.rx_enable ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "  TX_CLK:    %s", cfg.tx_clk_out_enable ? "OUTPUT ENABLED" : "OUTPUT DISABLED");
    LOG_I(TAG, "  RX_CLK:    %s", cfg.rx_clk_bypass ? "BYPASS (MUX7)" : "MUX");
    LOG_I(TAG, "  Loopback:  %s", cfg.loopback ? "YES" : "NO");
}

/*===========================================================================*/
/*                          LAN9646 DUMP                                      */
/*===========================================================================*/

void rgmii_debug_dump_lan9646(void) {
    if (!g_lan) {
        LOG_E(TAG, "LAN9646 handle not initialized!");
        return;
    }

    lan9646_port6_config_t cfg;
    rgmii_debug_read_lan9646_config(&cfg);

    SEPARATOR("LAN9646 PORT 6 CONFIGURATION");

    /* Chip ID */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Chip Identification ---");
    LOG_I(TAG, "  Chip ID:  0x%04X (expected 0x9477)", cfg.chip_id);
    LOG_I(TAG, "  Revision: %d", cfg.revision);

    /* Switch Operation */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Switch Operation ---");
    LOG_I(TAG, "  SWITCH_OP [0x0300] = 0x%02X", cfg.switch_op);
    LOG_I(TAG, "    Switch Start [0] = %d -> %s",
          cfg.switch_op & 1, (cfg.switch_op & 1) ? "RUNNING" : "STOPPED");

    /* XMII Control */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port 6 XMII Control (RGMII) ---");
    LOG_I(TAG, "  XMII_CTRL0 [0x6300] = 0x%02X", cfg.xmii_ctrl0);
    LOG_I(TAG, "    Duplex [6]        = %d -> %s",
          (cfg.xmii_ctrl0 >> 6) & 1, rgmii_debug_duplex_str(cfg.duplex));
    LOG_I(TAG, "    TX Flow Ctrl [5]  = %d -> %s",
          cfg.tx_flow_ctrl, cfg.tx_flow_ctrl ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "    Speed 100 [4]     = %d -> %s",
          (cfg.xmii_ctrl0 >> 4) & 1, ((cfg.xmii_ctrl0 >> 4) & 1) ? "100M" : "10M (if not 1000M)");
    LOG_I(TAG, "    RX Flow Ctrl [3]  = %d -> %s",
          cfg.rx_flow_ctrl, cfg.rx_flow_ctrl ? "ENABLED" : "DISABLED");

    LOG_I(TAG, "");
    LOG_I(TAG, "  XMII_CTRL1 [0x6301] = 0x%02X", cfg.xmii_ctrl1);
    LOG_I(TAG, "    Speed 1000 [6]    = %d -> %s",
          (cfg.xmii_ctrl1 >> 6) & 1,
          ((cfg.xmii_ctrl1 >> 6) & 1) ? "10/100M mode" : "1000M mode");
    LOG_I(TAG, "    RX Delay [4]      = %d -> RX internal delay: %s",
          cfg.rx_delay, cfg.rx_delay ? "ON (~1.5ns)" : "OFF");
    LOG_I(TAG, "    TX Delay [3]      = %d -> TX internal delay: %s",
          cfg.tx_delay, cfg.tx_delay ? "ON (~1.5ns)" : "OFF");
    LOG_I(TAG, "    -> Effective Speed: %s", rgmii_debug_speed_str(cfg.speed));

    /* Port Status */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port 6 Status ---");
    LOG_I(TAG, "  PORT_STATUS [0x6030] = 0x%02X", cfg.port_status);

    uint8_t op_speed = (cfg.port_status >> 3) & 0x03;
    const char* speed_status[] = {"10M", "100M", "1000M", "Reserved"};
    LOG_I(TAG, "    Speed Status [4:3]  = %d -> %s", op_speed, speed_status[op_speed]);
    LOG_I(TAG, "    Duplex Status [2]   = %d -> %s",
          (cfg.port_status >> 2) & 1,
          ((cfg.port_status >> 2) & 1) ? "Full" : "Half");
    LOG_I(TAG, "    TX Flow Status [1]  = %d", (cfg.port_status >> 1) & 1);
    LOG_I(TAG, "    RX Flow Status [0]  = %d", cfg.port_status & 1);

    /* Operation Control */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port 6 Operation Control ---");
    LOG_I(TAG, "  OP_CTRL0 [0x6020] = 0x%02X", cfg.op_ctrl0);
    LOG_I(TAG, "    Remote Loopback [6] = %d -> %s",
          (cfg.op_ctrl0 >> 6) & 1,
          ((cfg.op_ctrl0 >> 6) & 1) ? "ENABLED (MAC loopback)" : "DISABLED");

    /* MSTP State */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port 6 MSTP State ---");
    LOG_I(TAG, "  MSTP_STATE [0x6B04] = 0x%02X", cfg.mstp_state);
    LOG_I(TAG, "    TX Enable [2]    = %d -> %s",
          cfg.tx_enable, cfg.tx_enable ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "    RX Enable [1]    = %d -> %s",
          cfg.rx_enable, cfg.rx_enable ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "    Learning Dis [0] = %d -> Learning: %s",
          !cfg.learning_enable, cfg.learning_enable ? "ENABLED" : "DISABLED");

    /* MAC Control */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port 6 MAC Control ---");
    LOG_I(TAG, "  MAC_CTRL0 [0x6400] = 0x%02X", cfg.mac_ctrl0);
    LOG_I(TAG, "  MAC_CTRL1 [0x6401] = 0x%02X", cfg.mac_ctrl1);

    /* Membership */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port 6 VLAN Membership ---");
    LOG_I(TAG, "  MEMBERSHIP [0x6A04] = 0x%08lX", (unsigned long)cfg.membership);
    LOG_I(TAG, "    Can forward to:");
    LOG_I(TAG, "      Port 1: %s", (cfg.membership & 0x01) ? "YES" : "NO");
    LOG_I(TAG, "      Port 2: %s", (cfg.membership & 0x02) ? "YES" : "NO");
    LOG_I(TAG, "      Port 3: %s", (cfg.membership & 0x04) ? "YES" : "NO");
    LOG_I(TAG, "      Port 4: %s", (cfg.membership & 0x08) ? "YES" : "NO");
    LOG_I(TAG, "      Port 7: %s", (cfg.membership & 0x40) ? "YES" : "NO");

    /* Summary */
    LOG_I(TAG, "");
    LOG_I(TAG, "=== LAN9646 PORT 6 SUMMARY ===");
    LOG_I(TAG, "  Interface: RGMII");
    LOG_I(TAG, "  Speed:     %s", rgmii_debug_speed_str(cfg.speed));
    LOG_I(TAG, "  Duplex:    %s", rgmii_debug_duplex_str(cfg.duplex));
    LOG_I(TAG, "  TX:        %s", cfg.tx_enable ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "  RX:        %s", cfg.rx_enable ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "  TX Delay:  %s", cfg.tx_delay ? "ON (~1.5ns)" : "OFF");
    LOG_I(TAG, "  RX Delay:  %s", cfg.rx_delay ? "ON (~1.5ns)" : "OFF");
}

/*===========================================================================*/
/*                          DUMP ALL                                          */
/*===========================================================================*/

void rgmii_debug_dump_all(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "##                                                            ##");
    LOG_I(TAG, "##       RGMII CONFIGURATION DEBUG - S32K388 + LAN9646        ##");
    LOG_I(TAG, "##                                                            ##");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "################################################################");

    rgmii_debug_dump_s32k388();
    rgmii_debug_dump_lan9646();

    /* Configuration comparison */
    rgmii_config_snapshot_t snapshot;
    rgmii_debug_read_snapshot(&snapshot);
    rgmii_debug_print_validation(&snapshot);
}

/*===========================================================================*/
/*                          QUICK SUMMARY                                     */
/*===========================================================================*/

void rgmii_debug_quick_summary(void) {
    rgmii_config_snapshot_t snapshot;
    rgmii_debug_read_snapshot(&snapshot);

    SEPARATOR("RGMII QUICK SUMMARY");

    LOG_I(TAG, "");
    LOG_I(TAG, "              | S32K388 GMAC0 | LAN9646 Port6 | Match");
    LOG_I(TAG, "--------------+---------------+---------------+-------");
    LOG_I(TAG, "  Speed       | %-13s | %-13s | %s",
          rgmii_debug_speed_str(snapshot.s32k388.speed),
          rgmii_debug_speed_str(snapshot.lan9646.speed),
          snapshot.speed_match ? "OK" : "MISMATCH!");
    LOG_I(TAG, "  Duplex      | %-13s | %-13s | %s",
          rgmii_debug_duplex_str(snapshot.s32k388.duplex),
          rgmii_debug_duplex_str(snapshot.lan9646.duplex),
          snapshot.duplex_match ? "OK" : "MISMATCH!");
    LOG_I(TAG, "  TX Enable   | %-13s | %-13s | -",
          snapshot.s32k388.tx_enable ? "YES" : "NO",
          snapshot.lan9646.tx_enable ? "YES" : "NO");
    LOG_I(TAG, "  RX Enable   | %-13s | %-13s | -",
          snapshot.s32k388.rx_enable ? "YES" : "NO",
          snapshot.lan9646.rx_enable ? "YES" : "NO");
    LOG_I(TAG, "  Interface   | %-13s | RGMII         | %s",
          rgmii_debug_interface_str(snapshot.s32k388.interface_mode),
          snapshot.interface_valid ? "OK" : "ERROR!");
    LOG_I(TAG, "  TX_CLK Out  | %-13s | N/A           | %s",
          snapshot.s32k388.tx_clk_out_enable ? "ENABLED" : "DISABLED",
          snapshot.s32k388.tx_clk_out_enable ? "OK" : "ERROR!");
    LOG_I(TAG, "  RX_CLK Byp  | %-13s | N/A           | %s",
          snapshot.s32k388.rx_clk_bypass ? "ENABLED" : "DISABLED",
          snapshot.s32k388.rx_clk_bypass ? "OK" : "CHECK!");
    LOG_I(TAG, "  TX Delay    | N/A (PCB/SW)  | %-13s | -",
          snapshot.lan9646.tx_delay ? "ON (~1.5ns)" : "OFF");
    LOG_I(TAG, "  RX Delay    | N/A (PCB/SW)  | %-13s | -",
          snapshot.lan9646.rx_delay ? "ON (~1.5ns)" : "OFF");

    LOG_I(TAG, "");
    LOG_I(TAG, "Overall Status: %s", snapshot.overall_valid ? "CONFIGURATION OK" : "ISSUES DETECTED");

    if (!snapshot.overall_valid) {
        LOG_I(TAG, "");
        LOG_I(TAG, "Issues:");
        LOG_I(TAG, "%s", snapshot.recommendations);
    }
}

/*===========================================================================*/
/*                          VALIDATION PRINT                                  */
/*===========================================================================*/

void rgmii_debug_print_validation(const rgmii_config_snapshot_t* snapshot) {
    if (!snapshot) return;

    SEPARATOR("CONFIGURATION VALIDATION");

    LOG_I(TAG, "");
    LOG_I(TAG, "Check                         | Status");
    LOG_I(TAG, "------------------------------+--------");
    LOG_I(TAG, "  Speed Match                 | %s", snapshot->speed_match ? "PASS" : "FAIL");
    LOG_I(TAG, "  Duplex Match                | %s", snapshot->duplex_match ? "PASS" : "FAIL");
    LOG_I(TAG, "  S32K388 in RGMII Mode       | %s", snapshot->interface_valid ? "PASS" : "FAIL");
    LOG_I(TAG, "  Clock Configuration         | %s", snapshot->clocks_valid ? "PASS" : "FAIL");
    LOG_I(TAG, "  RGMII Delay (info only)     | TX=%s, RX=%s",
          snapshot->lan9646.tx_delay ? "ON" : "OFF",
          snapshot->lan9646.rx_delay ? "ON" : "OFF");

    LOG_I(TAG, "");
    LOG_I(TAG, "Overall: %s", snapshot->overall_valid ? "ALL CHECKS PASSED" : "ISSUES FOUND");

    if (!snapshot->overall_valid) {
        LOG_I(TAG, "");
        LOG_I(TAG, "Recommendations:");
        LOG_I(TAG, "%s", snapshot->recommendations);
    }

    /* RGMII delay recommendations */
    LOG_I(TAG, "");
    LOG_I(TAG, "=== RGMII DELAY RECOMMENDATIONS ===");
    LOG_I(TAG, "");
    LOG_I(TAG, "For RGMII to work, total delay on each path should be ~2ns:");
    LOG_I(TAG, "");
    LOG_I(TAG, "TX Path (S32K388 -> LAN9646):");
    LOG_I(TAG, "  - If PCB trace adds delay: LAN9646 RX_DELAY = OFF");
    LOG_I(TAG, "  - If PCB trace short:      LAN9646 RX_DELAY = ON");
    LOG_I(TAG, "");
    LOG_I(TAG, "RX Path (LAN9646 -> S32K388):");
    LOG_I(TAG, "  - If PCB trace adds delay: LAN9646 TX_DELAY = OFF");
    LOG_I(TAG, "  - If PCB trace short:      LAN9646 TX_DELAY = ON");
    LOG_I(TAG, "");
    LOG_I(TAG, "Current LAN9646 delay: TX=%s, RX=%s",
          snapshot->lan9646.tx_delay ? "ON" : "OFF",
          snapshot->lan9646.rx_delay ? "ON" : "OFF");
    LOG_I(TAG, "");
    LOG_I(TAG, "If communication fails, try 'rgmii_debug_delay_sweep()' to find");
    LOG_I(TAG, "the correct delay combination for your PCB.");
}

/*===========================================================================*/
/*                          SPEED CONFIGURATION                               */
/*===========================================================================*/

void rgmii_debug_set_s32k388_speed(rgmii_speed_t speed, rgmii_duplex_t duplex) {
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;

    /* Clear speed/duplex bits */
    mac_cfg &= ~((1U << 15) | (1U << 14) | (1U << 13));

    /* Set duplex */
    if (duplex == RGMII_DUPLEX_FULL) {
        mac_cfg |= (1U << 13);  /* DM = 1 */
    }

    /* Set speed */
    switch (speed) {
        case RGMII_SPEED_1000M:
            /* PS=0, FES=0 -> 1000M */
            break;
        case RGMII_SPEED_100M:
            /* PS=1, FES=1 -> 100M */
            mac_cfg |= (1U << 15) | (1U << 14);
            break;
        case RGMII_SPEED_10M:
            /* PS=1, FES=0 -> 10M */
            mac_cfg |= (1U << 15);
            break;
    }

    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;

    LOG_I(TAG, "S32K388 GMAC: Set %s %s Duplex",
          rgmii_debug_speed_str(speed), rgmii_debug_duplex_str(duplex));
}

void rgmii_debug_set_lan9646_speed(rgmii_speed_t speed, rgmii_duplex_t duplex) {
    if (!g_lan) return;

    uint8_t ctrl0 = lan_read8(0x6300);
    uint8_t ctrl1 = lan_read8(0x6301);

    /* Clear speed/duplex bits */
    ctrl0 &= ~(0x40 | 0x10);  /* Clear duplex and speed100 */
    ctrl1 &= ~0x40;           /* Clear speed1000 */

    /* Set duplex */
    if (duplex == RGMII_DUPLEX_FULL) {
        ctrl0 |= 0x40;
    }

    /* Set speed */
    switch (speed) {
        case RGMII_SPEED_1000M:
            /* ctrl1[6] = 0 means 1000M */
            break;
        case RGMII_SPEED_100M:
            ctrl1 |= 0x40;   /* Not 1000M */
            ctrl0 |= 0x10;   /* 100M */
            break;
        case RGMII_SPEED_10M:
            ctrl1 |= 0x40;   /* Not 1000M */
            /* ctrl0[4] = 0 means 10M */
            break;
    }

    lan_write8(0x6300, ctrl0);
    lan_write8(0x6301, ctrl1);

    LOG_I(TAG, "LAN9646 Port 6: Set %s %s Duplex",
          rgmii_debug_speed_str(speed), rgmii_debug_duplex_str(duplex));
}

bool rgmii_debug_set_speed(rgmii_speed_t speed, rgmii_duplex_t duplex) {
    LOG_I(TAG, "Setting RGMII speed to %s %s Duplex on both chips...",
          rgmii_debug_speed_str(speed), rgmii_debug_duplex_str(duplex));

    rgmii_debug_set_s32k388_speed(speed, duplex);
    rgmii_debug_set_lan9646_speed(speed, duplex);

    /* Verify */
    if (g_delay) g_delay(10);

    rgmii_config_snapshot_t snapshot;
    rgmii_debug_read_snapshot(&snapshot);

    bool success = snapshot.speed_match && snapshot.duplex_match;

    LOG_I(TAG, "Speed change: %s", success ? "SUCCESS" : "FAILED (mismatch)");

    return success;
}

/*===========================================================================*/
/*                          DELAY CONFIGURATION                               */
/*===========================================================================*/

void rgmii_debug_set_lan9646_delay(bool tx_delay, bool rx_delay) {
    if (!g_lan) return;

    uint8_t ctrl1 = lan_read8(0x6301);

    /* Clear delay bits */
    ctrl1 &= ~(0x08 | 0x10);

    /* Set delays */
    if (tx_delay) ctrl1 |= 0x08;  /* bit3 */
    if (rx_delay) ctrl1 |= 0x10;  /* bit4 */

    lan_write8(0x6301, ctrl1);

    LOG_I(TAG, "LAN9646 Port 6: TX_DELAY=%s, RX_DELAY=%s",
          tx_delay ? "ON" : "OFF", rx_delay ? "ON" : "OFF");
}

void rgmii_debug_delay_sweep(void) {
    SEPARATOR("RGMII DELAY SWEEP TEST");

    LOG_I(TAG, "");
    LOG_I(TAG, "Testing all 4 delay combinations with loopback...");
    LOG_I(TAG, "");

    const char* delay_names[] = {
        "No delay",
        "TX delay only",
        "RX delay only",
        "Both TX+RX"
    };
    const bool tx_delays[] = {false, true, false, true};
    const bool rx_delays[] = {false, false, true, true};

    /* Save current delay */
    uint8_t orig_ctrl1 = lan_read8(0x6301);

    /* Clear and read before counters */
    rgmii_debug_clear_lan9646_mib();
    rgmii_debug_clear_s32k388_mmc();

    LOG_I(TAG, "Opt | Delay Config      | LAN RX | LAN CRC | GMAC RX | GMAC CRC | Result");
    LOG_I(TAG, "----+-------------------+--------+---------+---------+----------+--------");

    for (int i = 0; i < 4; i++) {
        /* Set delay */
        rgmii_debug_set_lan9646_delay(tx_delays[i], rx_delays[i]);
        if (g_delay) g_delay(10);

        /* Clear counters */
        rgmii_debug_clear_lan9646_mib();

        /* Enable loopback */
        uint8_t ctrl = lan_read8(0x6020);
        ctrl |= 0x40;
        lan_write8(0x6020, ctrl);

        /* Read GMAC counters before */
        uint32_t gmac_rx_before = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
        uint32_t gmac_crc_before = IP_GMAC_0->RX_CRC_ERROR_PACKETS;

        /* Send test packets */
        for (int j = 0; j < 10; j++) {
            g_test_packet[50] = (uint8_t)j;
            (void)send_test_packet_helper(g_test_packet, sizeof(g_test_packet));
            if (g_delay) g_delay(5);
        }
        if (g_delay) g_delay(50);

        /* Disable loopback */
        ctrl &= ~0x40;
        lan_write8(0x6020, ctrl);

        /* Read counters */
        uint32_t lan_rx = lan_read_mib(6, 0x80);    /* RX Total */
        uint32_t lan_crc = lan_read_mib(6, 0x06);   /* RX CRC */
        uint32_t gmac_rx = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD - gmac_rx_before;
        uint32_t gmac_crc = IP_GMAC_0->RX_CRC_ERROR_PACKETS - gmac_crc_before;

        const char* result;
        if (lan_rx > 0 && lan_crc == 0 && gmac_rx > 0 && gmac_crc == 0) {
            result = "<<< BEST";
        } else if (lan_rx > 0 && gmac_rx > 0) {
            result = "Partial";
        } else if (lan_rx > 0) {
            result = "TX OK";
        } else {
            result = "FAIL";
        }

        LOG_I(TAG, " %d  | %-17s |   %2lu   |   %2lu    |   %2lu    |    %2lu    | %s",
              i, delay_names[i],
              (unsigned long)lan_rx, (unsigned long)lan_crc,
              (unsigned long)gmac_rx, (unsigned long)gmac_crc,
              result);
    }

    /* Restore original delay */
    lan_write8(0x6301, orig_ctrl1);

    LOG_I(TAG, "");
    LOG_I(TAG, "Use the option marked '<<< BEST' for your board.");
    LOG_I(TAG, "Apply with: rgmii_debug_set_lan9646_delay(tx, rx);");
}

/*===========================================================================*/
/*                          MMC/MIB COUNTERS                                  */
/*===========================================================================*/

void rgmii_debug_read_s32k388_mmc(s32k388_mmc_counters_t* counters) {
    if (!counters) return;

    memset(counters, 0, sizeof(s32k388_mmc_counters_t));

    /* TX Counters */
    counters->tx_octet_count_good_bad = IP_GMAC_0->TX_OCTET_COUNT_GOOD_BAD;
    counters->tx_packet_count_good_bad = IP_GMAC_0->TX_PACKET_COUNT_GOOD_BAD;
    counters->tx_broadcast_packets_good = IP_GMAC_0->TX_BROADCAST_PACKETS_GOOD;
    counters->tx_multicast_packets_good = IP_GMAC_0->TX_MULTICAST_PACKETS_GOOD;
    counters->tx_unicast_packets_good_bad = IP_GMAC_0->TX_UNICAST_PACKETS_GOOD_BAD;
    counters->tx_underflow_error_packets = IP_GMAC_0->TX_UNDERFLOW_ERROR_PACKETS;
    counters->tx_single_collision_good_packets = IP_GMAC_0->TX_SINGLE_COLLISION_GOOD_PACKETS;
    counters->tx_multiple_collision_good_packets = IP_GMAC_0->TX_MULTIPLE_COLLISION_GOOD_PACKETS;
    counters->tx_deferred_packets = IP_GMAC_0->TX_DEFERRED_PACKETS;
    counters->tx_late_collision_packets = IP_GMAC_0->TX_LATE_COLLISION_PACKETS;
    counters->tx_excessive_collision_packets = IP_GMAC_0->TX_EXCESSIVE_COLLISION_PACKETS;
    counters->tx_carrier_error_packets = IP_GMAC_0->TX_CARRIER_ERROR_PACKETS;
    counters->tx_packet_count_good = IP_GMAC_0->TX_PACKET_COUNT_GOOD;
    counters->tx_pause_packets = IP_GMAC_0->TX_PAUSE_PACKETS;

    /* RX Counters */
    counters->rx_packets_count_good_bad = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
    counters->rx_octet_count_good_bad = IP_GMAC_0->RX_OCTET_COUNT_GOOD_BAD;
    counters->rx_octet_count_good = IP_GMAC_0->RX_OCTET_COUNT_GOOD;
    counters->rx_broadcast_packets_good = IP_GMAC_0->RX_BROADCAST_PACKETS_GOOD;
    counters->rx_multicast_packets_good = IP_GMAC_0->RX_MULTICAST_PACKETS_GOOD;
    counters->rx_crc_error_packets = IP_GMAC_0->RX_CRC_ERROR_PACKETS;
    counters->rx_alignment_error_packets = IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS;
    counters->rx_runt_error_packets = IP_GMAC_0->RX_RUNT_ERROR_PACKETS;
    counters->rx_jabber_error_packets = IP_GMAC_0->RX_JABBER_ERROR_PACKETS;
    counters->rx_undersize_packets_good = IP_GMAC_0->RX_UNDERSIZE_PACKETS_GOOD;
    counters->rx_oversize_packets_good = IP_GMAC_0->RX_OVERSIZE_PACKETS_GOOD;
    counters->rx_unicast_packets_good = IP_GMAC_0->RX_UNICAST_PACKETS_GOOD;
    counters->rx_length_error_packets = IP_GMAC_0->RX_LENGTH_ERROR_PACKETS;
    counters->rx_out_of_range_type_packets = IP_GMAC_0->RX_OUT_OF_RANGE_TYPE_PACKETS;
    counters->rx_pause_packets = IP_GMAC_0->RX_PAUSE_PACKETS;
    counters->rx_fifo_overflow_packets = IP_GMAC_0->RX_FIFO_OVERFLOW_PACKETS;
    counters->rx_watchdog_error_packets = IP_GMAC_0->RX_WATCHDOG_ERROR_PACKETS;
}

void rgmii_debug_read_lan9646_mib(lan9646_mib_counters_t* counters) {
    if (!counters || !g_lan) return;

    memset(counters, 0, sizeof(lan9646_mib_counters_t));

    /* RX Counters */
    counters->rx_hi_priority_bytes = lan_read_mib(6, 0x00);
    counters->rx_undersize = lan_read_mib(6, 0x01);
    counters->rx_fragments = lan_read_mib(6, 0x02);
    counters->rx_oversize = lan_read_mib(6, 0x03);
    counters->rx_jabbers = lan_read_mib(6, 0x04);
    counters->rx_symbol_err = lan_read_mib(6, 0x05);
    counters->rx_crc_err = lan_read_mib(6, 0x06);
    counters->rx_align_err = lan_read_mib(6, 0x07);
    counters->rx_control = lan_read_mib(6, 0x08);
    counters->rx_pause = lan_read_mib(6, 0x09);
    counters->rx_broadcast = lan_read_mib(6, 0x0A);
    counters->rx_multicast = lan_read_mib(6, 0x0B);
    counters->rx_unicast = lan_read_mib(6, 0x0C);
    counters->rx_64 = lan_read_mib(6, 0x0D);
    counters->rx_65_127 = lan_read_mib(6, 0x0E);
    counters->rx_128_255 = lan_read_mib(6, 0x0F);
    counters->rx_256_511 = lan_read_mib(6, 0x10);
    counters->rx_512_1023 = lan_read_mib(6, 0x11);
    counters->rx_1024_1522 = lan_read_mib(6, 0x12);
    counters->rx_total = lan_read_mib(6, 0x80);
    counters->rx_dropped = lan_read_mib(6, 0x82);

    /* TX Counters */
    counters->tx_hi_priority_bytes = lan_read_mib(6, 0x60);
    counters->tx_late_collision = lan_read_mib(6, 0x61);
    counters->tx_pause = lan_read_mib(6, 0x62);
    counters->tx_broadcast = lan_read_mib(6, 0x63);
    counters->tx_multicast = lan_read_mib(6, 0x64);
    counters->tx_unicast = lan_read_mib(6, 0x65);
    counters->tx_deferred = lan_read_mib(6, 0x66);
    counters->tx_total_collision = lan_read_mib(6, 0x67);
    counters->tx_excess_collision = lan_read_mib(6, 0x68);
    counters->tx_single_collision = lan_read_mib(6, 0x69);
    counters->tx_multi_collision = lan_read_mib(6, 0x6A);
    counters->tx_total = lan_read_mib(6, 0x81);
    counters->tx_dropped = lan_read_mib(6, 0x83);
}

void rgmii_debug_dump_s32k388_mmc(void) {
    s32k388_mmc_counters_t c;
    rgmii_debug_read_s32k388_mmc(&c);

    SEPARATOR("S32K388 GMAC0 MMC COUNTERS");

    LOG_I(TAG, "");
    LOG_I(TAG, "--- TX Counters ---");
    LOG_I(TAG, "  TX Packets (Good+Bad):  %lu", (unsigned long)c.tx_packet_count_good_bad);
    LOG_I(TAG, "  TX Packets (Good):      %lu", (unsigned long)c.tx_packet_count_good);
    LOG_I(TAG, "  TX Octets (Good+Bad):   %lu", (unsigned long)c.tx_octet_count_good_bad);
    LOG_I(TAG, "  TX Broadcast:           %lu", (unsigned long)c.tx_broadcast_packets_good);
    LOG_I(TAG, "  TX Multicast:           %lu", (unsigned long)c.tx_multicast_packets_good);
    LOG_I(TAG, "  TX Unicast:             %lu", (unsigned long)c.tx_unicast_packets_good_bad);
    LOG_I(TAG, "  TX Underflow Errors:    %lu", (unsigned long)c.tx_underflow_error_packets);
    LOG_I(TAG, "  TX Deferred:            %lu", (unsigned long)c.tx_deferred_packets);
    LOG_I(TAG, "  TX Late Collision:      %lu", (unsigned long)c.tx_late_collision_packets);
    LOG_I(TAG, "  TX Excess Collision:    %lu", (unsigned long)c.tx_excessive_collision_packets);
    LOG_I(TAG, "  TX Carrier Error:       %lu", (unsigned long)c.tx_carrier_error_packets);
    LOG_I(TAG, "  TX Pause:               %lu", (unsigned long)c.tx_pause_packets);

    LOG_I(TAG, "");
    LOG_I(TAG, "--- RX Counters ---");
    LOG_I(TAG, "  RX Packets (Good+Bad):  %lu", (unsigned long)c.rx_packets_count_good_bad);
    LOG_I(TAG, "  RX Octets (Good+Bad):   %lu", (unsigned long)c.rx_octet_count_good_bad);
    LOG_I(TAG, "  RX Octets (Good):       %lu", (unsigned long)c.rx_octet_count_good);
    LOG_I(TAG, "  RX Broadcast:           %lu", (unsigned long)c.rx_broadcast_packets_good);
    LOG_I(TAG, "  RX Multicast:           %lu", (unsigned long)c.rx_multicast_packets_good);
    LOG_I(TAG, "  RX Unicast:             %lu", (unsigned long)c.rx_unicast_packets_good);
    LOG_I(TAG, "  RX CRC Errors:          %lu", (unsigned long)c.rx_crc_error_packets);
    LOG_I(TAG, "  RX Alignment Errors:    %lu", (unsigned long)c.rx_alignment_error_packets);
    LOG_I(TAG, "  RX Runt Errors:         %lu", (unsigned long)c.rx_runt_error_packets);
    LOG_I(TAG, "  RX Jabber Errors:       %lu", (unsigned long)c.rx_jabber_error_packets);
    LOG_I(TAG, "  RX Undersize:           %lu", (unsigned long)c.rx_undersize_packets_good);
    LOG_I(TAG, "  RX Oversize:            %lu", (unsigned long)c.rx_oversize_packets_good);
    LOG_I(TAG, "  RX Length Error:        %lu", (unsigned long)c.rx_length_error_packets);
    LOG_I(TAG, "  RX FIFO Overflow:       %lu", (unsigned long)c.rx_fifo_overflow_packets);
    LOG_I(TAG, "  RX Pause:               %lu", (unsigned long)c.rx_pause_packets);
}

void rgmii_debug_dump_lan9646_mib(void) {
    lan9646_mib_counters_t c;
    rgmii_debug_read_lan9646_mib(&c);

    SEPARATOR("LAN9646 PORT 6 MIB COUNTERS");

    LOG_I(TAG, "");
    LOG_I(TAG, "--- RX Counters (from GMAC TX) ---");
    LOG_I(TAG, "  RX Total:               %lu", (unsigned long)c.rx_total);
    LOG_I(TAG, "  RX Broadcast:           %lu", (unsigned long)c.rx_broadcast);
    LOG_I(TAG, "  RX Multicast:           %lu", (unsigned long)c.rx_multicast);
    LOG_I(TAG, "  RX Unicast:             %lu", (unsigned long)c.rx_unicast);
    LOG_I(TAG, "  RX CRC Errors:          %lu", (unsigned long)c.rx_crc_err);
    LOG_I(TAG, "  RX Symbol Errors:       %lu", (unsigned long)c.rx_symbol_err);
    LOG_I(TAG, "  RX Alignment Errors:    %lu", (unsigned long)c.rx_align_err);
    LOG_I(TAG, "  RX Undersize:           %lu", (unsigned long)c.rx_undersize);
    LOG_I(TAG, "  RX Oversize:            %lu", (unsigned long)c.rx_oversize);
    LOG_I(TAG, "  RX Fragments:           %lu", (unsigned long)c.rx_fragments);
    LOG_I(TAG, "  RX Jabbers:             %lu", (unsigned long)c.rx_jabbers);
    LOG_I(TAG, "  RX Dropped:             %lu", (unsigned long)c.rx_dropped);

    LOG_I(TAG, "");
    LOG_I(TAG, "--- TX Counters (to GMAC RX) ---");
    LOG_I(TAG, "  TX Total:               %lu", (unsigned long)c.tx_total);
    LOG_I(TAG, "  TX Broadcast:           %lu", (unsigned long)c.tx_broadcast);
    LOG_I(TAG, "  TX Multicast:           %lu", (unsigned long)c.tx_multicast);
    LOG_I(TAG, "  TX Unicast:             %lu", (unsigned long)c.tx_unicast);
    LOG_I(TAG, "  TX Late Collision:      %lu", (unsigned long)c.tx_late_collision);
    LOG_I(TAG, "  TX Excess Collision:    %lu", (unsigned long)c.tx_excess_collision);
    LOG_I(TAG, "  TX Deferred:            %lu", (unsigned long)c.tx_deferred);
    LOG_I(TAG, "  TX Dropped:             %lu", (unsigned long)c.tx_dropped);
}

void rgmii_debug_dump_all_counters(void) {
    rgmii_debug_dump_s32k388_mmc();
    rgmii_debug_dump_lan9646_mib();

    SEPARATOR("COUNTER COMPARISON");

    s32k388_mmc_counters_t s32k;
    lan9646_mib_counters_t lan;
    rgmii_debug_read_s32k388_mmc(&s32k);
    rgmii_debug_read_lan9646_mib(&lan);

    LOG_I(TAG, "");
    LOG_I(TAG, "Path             | S32K388 GMAC | LAN9646 P6 | Match");
    LOG_I(TAG, "-----------------+--------------+------------+-------");
    LOG_I(TAG, "TX Path (->LAN): | TX=%9lu | RX=%8lu | %s",
          (unsigned long)s32k.tx_packet_count_good_bad,
          (unsigned long)lan.rx_total,
          (s32k.tx_packet_count_good_bad == lan.rx_total) ? "OK" : "DIFF");
    LOG_I(TAG, "  CRC Errors:    | -            | %10lu | %s",
          (unsigned long)lan.rx_crc_err,
          (lan.rx_crc_err == 0) ? "OK" : "ERROR!");
    LOG_I(TAG, "RX Path (<-LAN): | RX=%9lu | TX=%8lu | %s",
          (unsigned long)s32k.rx_packets_count_good_bad,
          (unsigned long)lan.tx_total,
          (s32k.rx_packets_count_good_bad == lan.tx_total) ? "OK" : "DIFF");
    LOG_I(TAG, "  CRC Errors:    | %12lu | -          | %s",
          (unsigned long)s32k.rx_crc_error_packets,
          (s32k.rx_crc_error_packets == 0) ? "OK" : "ERROR!");
}

void rgmii_debug_clear_s32k388_mmc(void) {
    /* MMC counters are cleared by reading them or by writing to MMC_CONTROL */
    IP_GMAC_0->MMC_CONTROL |= (1U << 0);  /* Counter Reset */
    LOG_I(TAG, "S32K388 MMC counters cleared");
}

void rgmii_debug_clear_lan9646_mib(void) {
    if (!g_lan) return;

    /* Read all MIB counters to clear them (read-to-clear) */
    for (uint8_t i = 0; i < 0x90; i++) {
        (void)lan_read_mib(6, i);
    }
    LOG_I(TAG, "LAN9646 Port 6 MIB counters cleared");
}

/*===========================================================================*/
/*                          CLOCK DIAGNOSTICS                                 */
/*===========================================================================*/

void rgmii_debug_dump_clocks(void) {
    SEPARATOR("GMAC CLOCK CONFIGURATION");

    s32k388_gmac_config_t cfg;
    rgmii_debug_read_s32k388_config(&cfg);

    LOG_I(TAG, "");
    LOG_I(TAG, "--- MUX_8 (GMAC0_TX_CLK source) ---");
    LOG_I(TAG, "  CSC  = 0x%08lX", (unsigned long)cfg.mux8_csc);
    LOG_I(TAG, "  CSS  = 0x%08lX", (unsigned long)cfg.mux8_css);
    LOG_I(TAG, "  DC_0 = 0x%08lX", (unsigned long)cfg.mux8_dc0);

    /* Decode source */
    uint8_t src = (cfg.mux8_css >> 24) & 0x3F;
    LOG_I(TAG, "  Source Select = %d", src);

    /* Expected clocks for RGMII */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Expected TX Clock for RGMII ---");
    LOG_I(TAG, "  1000 Mbps: 125 MHz");
    LOG_I(TAG, "  100 Mbps:  25 MHz");
    LOG_I(TAG, "  10 Mbps:   2.5 MHz");
    LOG_I(TAG, "");
    LOG_I(TAG, "  Current speed setting: %s", rgmii_debug_speed_str(cfg.speed));

    /* DCM_GPR */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- DCM_GPR Clock Control ---");
    LOG_I(TAG, "  RX_CLK_MUX_BYPASS (DCMRWF3[13]) = %d", cfg.rx_clk_bypass);
    LOG_I(TAG, "    -> RX_CLK bypasses MUX7 (from LAN9646): %s",
          cfg.rx_clk_bypass ? "YES (REQUIRED!)" : "NO (CHECK!)");
    LOG_I(TAG, "  TX_CLK_OUT_EN (DCMRWF3[11]) = %d", cfg.tx_clk_out_enable);
    LOG_I(TAG, "    -> S32K388 outputs TX_CLK to LAN9646: %s",
          cfg.tx_clk_out_enable ? "YES" : "NO");
}

bool rgmii_debug_verify_tx_clock(void) {
    s32k388_gmac_config_t cfg;
    rgmii_debug_read_s32k388_config(&cfg);

    bool ok = cfg.tx_clk_out_enable;

    if (!ok) {
        LOG_E(TAG, "TX_CLK output NOT enabled! Set DCMRWF3[11]=1 (TX_CLK_OUT_EN)");
    } else {
        LOG_I(TAG, "TX_CLK output enabled (DCMRWF3[11]=1): OK");
    }

    return ok;
}

bool rgmii_debug_verify_rx_clock_bypass(void) {
    s32k388_gmac_config_t cfg;
    rgmii_debug_read_s32k388_config(&cfg);

    bool ok = cfg.rx_clk_bypass;

    if (!ok) {
        LOG_W(TAG, "RX_CLK bypass NOT enabled. This is REQUIRED for RGMII!");
        LOG_W(TAG, "Fix: IP_DCM_GPR->DCMRWF3 |= (1U << 13);  /* bit 13 = RX_CLK_MUX_BYPASS */");
    } else {
        LOG_I(TAG, "RX_CLK bypass enabled (DCMRWF3[13]=1, MUX7 bypassed): OK");
    }

    return ok;
}

/*===========================================================================*/
/*                          SIGNAL TESTS                                      */
/*===========================================================================*/

uint32_t rgmii_debug_test_tx_path(uint32_t count) {
    if (!g_lan) return 0;

    LOG_I(TAG, "Testing TX path: Sending %lu packets...", (unsigned long)count);

    /* Clear counters */
    rgmii_debug_clear_lan9646_mib();

    /* Send packets */
    uint32_t sent = 0;
    for (uint32_t i = 0; i < count; i++) {
        g_test_packet[50] = (uint8_t)(i & 0xFF);
        if (send_test_packet_helper(g_test_packet, sizeof(g_test_packet)) == GMAC_STATUS_SUCCESS) {
            sent++;
        }
        if (g_delay) g_delay(2);
    }
    if (g_delay) g_delay(50);

    /* Check LAN9646 RX */
    uint32_t lan_rx = lan_read_mib(6, 0x80);
    uint32_t lan_crc = lan_read_mib(6, 0x06);

    LOG_I(TAG, "TX Path Test Results:");
    LOG_I(TAG, "  GMAC TX:     %lu packets sent", (unsigned long)sent);
    LOG_I(TAG, "  LAN9646 RX:  %lu good, %lu CRC errors",
          (unsigned long)lan_rx, (unsigned long)lan_crc);

    if (lan_rx == sent && lan_crc == 0) {
        LOG_I(TAG, "  Result: TX PATH OK!");
    } else if (lan_crc > 0) {
        LOG_E(TAG, "  Result: TX PATH HAS CRC ERRORS - Check timing/delay");
    } else {
        LOG_E(TAG, "  Result: TX PATH FAIL - Packets not received");
    }

    return lan_rx;
}

uint32_t rgmii_debug_test_loopback(uint32_t count) {
    if (!g_lan) return 0;

    LOG_I(TAG, "Testing loopback: Sending %lu packets...", (unsigned long)count);

    /* Clear counters */
    rgmii_debug_clear_lan9646_mib();
    uint32_t gmac_rx_before = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
    uint32_t gmac_crc_before = IP_GMAC_0->RX_CRC_ERROR_PACKETS;

    /* Enable loopback */
    uint8_t ctrl = lan_read8(0x6020);
    ctrl |= 0x40;
    lan_write8(0x6020, ctrl);

    /* Send packets */
    uint32_t sent = 0;
    for (uint32_t i = 0; i < count; i++) {
        g_test_packet[50] = (uint8_t)(i & 0xFF);
        if (send_test_packet_helper(g_test_packet, sizeof(g_test_packet)) == GMAC_STATUS_SUCCESS) {
            sent++;
        }
        if (g_delay) g_delay(5);
    }
    if (g_delay) g_delay(100);

    /* Disable loopback */
    ctrl &= ~0x40;
    lan_write8(0x6020, ctrl);

    /* Read counters */
    uint32_t lan_rx = lan_read_mib(6, 0x80);
    uint32_t lan_crc = lan_read_mib(6, 0x06);
    uint32_t gmac_rx = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD - gmac_rx_before;
    uint32_t gmac_crc = IP_GMAC_0->RX_CRC_ERROR_PACKETS - gmac_crc_before;

    LOG_I(TAG, "Loopback Test Results:");
    LOG_I(TAG, "  GMAC TX:     %lu packets sent", (unsigned long)sent);
    LOG_I(TAG, "  LAN9646 RX:  %lu good, %lu CRC errors", (unsigned long)lan_rx, (unsigned long)lan_crc);
    LOG_I(TAG, "  GMAC RX:     %lu good, %lu CRC errors", (unsigned long)gmac_rx, (unsigned long)gmac_crc);

    if (gmac_rx == sent && gmac_crc == 0 && lan_crc == 0) {
        LOG_I(TAG, "  Result: FULL LOOPBACK OK!");
    } else if (lan_rx > 0 && gmac_rx == 0) {
        LOG_E(TAG, "  Result: TX OK but RX FAIL - Check RX path/delay");
    } else if (lan_crc > 0 || gmac_crc > 0) {
        LOG_E(TAG, "  Result: TIMING ISSUE - Try different delay config");
    } else {
        LOG_E(TAG, "  Result: LOOPBACK FAIL");
    }

    return gmac_rx;
}

/*===========================================================================*/
/*                          TROUBLESHOOTING                                   */
/*===========================================================================*/

void rgmii_debug_print_troubleshooting(void) {
    rgmii_config_snapshot_t snapshot;
    rgmii_debug_read_snapshot(&snapshot);

    SEPARATOR("TROUBLESHOOTING GUIDE");

    LOG_I(TAG, "");
    LOG_I(TAG, "Based on current configuration:");
    LOG_I(TAG, "");

    /* Check interface mode (S32K388: MAC_CONF_SEL = 1 for RGMII!) */
    if (snapshot.s32k388.interface_mode != 1) {
        LOG_E(TAG, "1. S32K388 is NOT in RGMII mode! (MAC_CONF_SEL=%d, expected 1)",
              snapshot.s32k388.interface_mode);
        LOG_E(TAG, "   Note: S32K388 uses MAC_CONF_SEL=1 for RGMII (different from other S32K3!)");
        LOG_E(TAG, "   Fix: IP_DCM_GPR->DCMRWF1 = (IP_DCM_GPR->DCMRWF1 & ~0x03) | 0x01;");
        LOG_I(TAG, "");
    }

    /* Check TX clock output */
    if (!snapshot.s32k388.tx_clk_out_enable) {
        LOG_E(TAG, "2. TX_CLK output is DISABLED!");
        LOG_E(TAG, "   The S32K388 must output TX_CLK to drive the LAN9646.");
        LOG_E(TAG, "   Fix: IP_DCM_GPR->DCMRWF3 |= (1U << 11);  /* bit 11 = TX_CLK_OUT_EN */");
        LOG_I(TAG, "");
    }

    /* Check RX clock bypass */
    if (!snapshot.s32k388.rx_clk_bypass) {
        LOG_W(TAG, "3. RX_CLK bypass is DISABLED");
        LOG_W(TAG, "   LAN9646 provides RX_CLK which needs to bypass MUX7.");
        LOG_W(TAG, "   Fix: IP_DCM_GPR->DCMRWF3 |= (1U << 13);  /* bit 13 = RX_CLK_MUX_BYPASS */");
        LOG_I(TAG, "");
    }

    /* Check speed match */
    if (!snapshot.speed_match) {
        LOG_E(TAG, "4. Speed MISMATCH detected!");
        LOG_E(TAG, "   S32K388: %s, LAN9646: %s",
              rgmii_debug_speed_str(snapshot.s32k388.speed),
              rgmii_debug_speed_str(snapshot.lan9646.speed));
        LOG_E(TAG, "   Fix: Call rgmii_debug_set_speed(RGMII_SPEED_100M, RGMII_DUPLEX_FULL);");
        LOG_I(TAG, "");
    }

    /* Check duplex match */
    if (!snapshot.duplex_match) {
        LOG_E(TAG, "5. Duplex MISMATCH detected!");
        LOG_E(TAG, "   S32K388: %s, LAN9646: %s",
              rgmii_debug_duplex_str(snapshot.s32k388.duplex),
              rgmii_debug_duplex_str(snapshot.lan9646.duplex));
        LOG_I(TAG, "");
    }

    /* RGMII delay info */
    LOG_I(TAG, "6. RGMII DELAY Configuration:");
    LOG_I(TAG, "   Current LAN9646: TX_DELAY=%s, RX_DELAY=%s",
          snapshot.lan9646.tx_delay ? "ON" : "OFF",
          snapshot.lan9646.rx_delay ? "ON" : "OFF");
    LOG_I(TAG, "");
    LOG_I(TAG, "   If you see CRC errors, the timing is wrong.");
    LOG_I(TAG, "   Run: rgmii_debug_delay_sweep() to find the correct setting.");
    LOG_I(TAG, "");

    /* Common issues */
    LOG_I(TAG, "=== COMMON ISSUES ===");
    LOG_I(TAG, "");
    LOG_I(TAG, "NO PACKETS RECEIVED by LAN9646:");
    LOG_I(TAG, "  - Check TX_CLK output enabled");
    LOG_I(TAG, "  - Verify PCB connections (TXD0-3, TX_CTL, TX_CLK)");
    LOG_I(TAG, "  - Check if GMAC is transmitting (TX_PACKET_COUNT_GOOD)");
    LOG_I(TAG, "");
    LOG_I(TAG, "ALL PACKETS HAVE CRC ERRORS:");
    LOG_I(TAG, "  - Timing issue - try different delay combinations");
    LOG_I(TAG, "  - Run rgmii_debug_delay_sweep()");
    LOG_I(TAG, "");
    LOG_I(TAG, "TX OK but RX FAILS:");
    LOG_I(TAG, "  - Enable RX_CLK bypass: IP_DCM_GPR->DCMRWF3 |= 0x01;");
    LOG_I(TAG, "  - Check RXD0-3, RX_CTL, RX_CLK signals");
    LOG_I(TAG, "  - Adjust LAN9646 TX delay setting");
    LOG_I(TAG, "");
    LOG_I(TAG, "INTERMITTENT ERRORS:");
    LOG_I(TAG, "  - Signal integrity issue");
    LOG_I(TAG, "  - Check PCB trace length matching");
    LOG_I(TAG, "  - Try different delay combinations");
}

void rgmii_debug_full_diagnostic(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "##                                                            ##");
    LOG_I(TAG, "##         RGMII FULL DIAGNOSTIC - S32K388 + LAN9646          ##");
    LOG_I(TAG, "##                                                            ##");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "################################################################");

    /* Configuration dump */
    rgmii_debug_dump_all();

    /* Clock verification */
    rgmii_debug_dump_clocks();

    /* TX path test */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- TX Path Test ---");
    uint32_t tx_result = rgmii_debug_test_tx_path(10);

    /* Loopback test */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Loopback Test ---");
    uint32_t loop_result = rgmii_debug_test_loopback(10);

    /* Counter comparison */
    rgmii_debug_dump_all_counters();

    /* If tests failed, run delay sweep */
    if (tx_result == 0 || loop_result == 0) {
        LOG_I(TAG, "");
        LOG_I(TAG, "Tests failed - running delay sweep...");
        rgmii_debug_delay_sweep();
    }

    /* Troubleshooting */
    rgmii_debug_print_troubleshooting();

    /* Final summary */
    SEPARATOR("DIAGNOSTIC SUMMARY");

    rgmii_config_snapshot_t snapshot;
    rgmii_debug_read_snapshot(&snapshot);

    LOG_I(TAG, "");
    if (snapshot.overall_valid && tx_result > 0 && loop_result > 0) {
        LOG_I(TAG, "RESULT: RGMII INTERFACE WORKING CORRECTLY!");
        LOG_I(TAG, "");
        LOG_I(TAG, "  Configuration: VALID");
        LOG_I(TAG, "  TX Path:       %lu/10 packets OK", (unsigned long)tx_result);
        LOG_I(TAG, "  Loopback:      %lu/10 packets OK", (unsigned long)loop_result);
    } else {
        LOG_E(TAG, "RESULT: ISSUES DETECTED");
        LOG_E(TAG, "");
        LOG_E(TAG, "  Configuration: %s", snapshot.overall_valid ? "VALID" : "INVALID");
        LOG_E(TAG, "  TX Path:       %lu/10 packets", (unsigned long)tx_result);
        LOG_E(TAG, "  Loopback:      %lu/10 packets", (unsigned long)loop_result);
        LOG_E(TAG, "");
        LOG_E(TAG, "See troubleshooting guide above for fixes.");
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
}
