/**
 * @file    rgmii_rx_debug.c
 * @brief   RGMII RX Path Debug Module Implementation
 * @note    Focus on debugging RX path: LAN9646 TX -> S32K388 GMAC RX
 */

#include "rgmii_rx_debug.h"
#include "log_debug.h"
#include "S32K388.h"
#include "Gmac_Ip.h"
#include <string.h>
#include <stdio.h>

#define TAG "RX_DBG"

/*===========================================================================*/
/*                          REGISTER ADDRESSES                                */
/*===========================================================================*/

/* SIUL2 Base addresses */
#define SIUL2_0_BASE            0x40290000UL
#define SIUL2_IMCR_BASE         (SIUL2_0_BASE + 0x0A40UL)
#define SIUL2_MSCR_BASE         (SIUL2_0_BASE + 0x0240UL)

/* GMAC0 RX IMCR indices */
#define IMCR_GMAC0_RX_CLK       300     /* PTB22 */
#define IMCR_GMAC0_RX_CTL       292     /* PTC16 */
#define IMCR_GMAC0_RXD0         294     /* PTC14 */
#define IMCR_GMAC0_RXD1         295     /* PTC15 */
#define IMCR_GMAC0_RXD2         301     /* PTB23 */
#define IMCR_GMAC0_RXD3         302     /* PTB24 */

/* GMAC0 RX MSCR indices (pins configured as GMAC inputs) */
#define MSCR_PTB22              54      /* RX_CLK */
#define MSCR_PTB23              55      /* RXD2 */
#define MSCR_PTB24              56      /* RXD3 */
#define MSCR_PTC14              78      /* RXD0 */
#define MSCR_PTC15              79      /* RXD1 */
#define MSCR_PTC16              80      /* RX_CTL */

/* LAN9646 Port 6 registers */
#define LAN_PORT6_BASE          0x6000
#define LAN_XMII_CTRL0          (LAN_PORT6_BASE | 0x0300)
#define LAN_XMII_CTRL1          (LAN_PORT6_BASE | 0x0301)
#define LAN_PORT_STATUS         (LAN_PORT6_BASE | 0x0030)
#define LAN_MSTP_STATE          (LAN_PORT6_BASE | 0x0B04)
#define LAN_OP_CTRL0            (LAN_PORT6_BASE | 0x0020)
#define LAN_MIB_CTRL            (LAN_PORT6_BASE | 0x0500)
#define LAN_MIB_DATA            (LAN_PORT6_BASE | 0x0504)

/*===========================================================================*/
/*                          PRIVATE DATA                                      */
/*===========================================================================*/

static lan9646_t* g_lan = NULL;
static void (*g_delay)(uint32_t) = NULL;

/*===========================================================================*/
/*                          HELPER MACROS                                     */
/*===========================================================================*/

#define SEPARATOR(title) do { \
    LOG_I(TAG, ""); \
    LOG_I(TAG, "================================================================"); \
    LOG_I(TAG, "  %s", title); \
    LOG_I(TAG, "================================================================"); \
} while(0)

#define SUBSECTION(title) do { \
    LOG_I(TAG, ""); \
    LOG_I(TAG, "--- %s ---", title); \
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

static void lan_write32(uint16_t addr, uint32_t val) {
    if (g_lan) lan9646_write_reg32(g_lan, addr, val);
}

/* Read MIB counter */
static uint32_t lan_read_mib(uint8_t port, uint8_t index) {
    if (!g_lan) return 0;

    uint16_t base = (uint16_t)port << 12;
    uint32_t ctrl, data = 0;
    uint32_t timeout = 1000;

    ctrl = ((uint32_t)index << 16) | 0x02000000UL;
    lan_write32(base | 0x0500, ctrl);

    do {
        lan_read32(base | 0x0500, &ctrl);
        if (--timeout == 0) break;
    } while (ctrl & 0x02000000UL);

    lan_read32(base | 0x0504, &data);
    return data;
}

/* Flush MIB counters */
static void lan_flush_mib(uint8_t port) {
    if (!g_lan) return;

    uint16_t base = (uint16_t)port << 12;
    lan_write32(base | 0x0500, 0x01000000UL);  /* Flush/Freeze enable */
    if (g_delay) g_delay(1);
    lan_write32(base | 0x0500, 0x00000000UL);  /* Clear */
}

/*===========================================================================*/
/*                          SIUL2 HELPER FUNCTIONS                            */
/*===========================================================================*/

static uint32_t read_imcr(uint16_t index) {
    volatile uint32_t* imcr = (volatile uint32_t*)SIUL2_IMCR_BASE;
    return imcr[index];
}

static uint32_t read_mscr(uint16_t index) {
    volatile uint32_t* mscr = (volatile uint32_t*)SIUL2_MSCR_BASE;
    return mscr[index];
}

/*===========================================================================*/
/*                          INITIALIZATION                                    */
/*===========================================================================*/

void rx_debug_init(lan9646_t* lan, void (*delay_ms)(uint32_t)) {
    g_lan = lan;
    g_delay = delay_ms;
}

/*===========================================================================*/
/*                          RX_CLK ANALYSIS                                   */
/*===========================================================================*/

void rx_debug_analyze_rx_clk(void) {
    SEPARATOR("RX_CLK SIGNAL ANALYSIS");

    LOG_I(TAG, "");
    LOG_I(TAG, "RX_CLK is THE MOST CRITICAL signal for RX path!");
    LOG_I(TAG, "It must come from LAN9646 Port 6 to S32K388.");
    LOG_I(TAG, "");

    /* DCM_GPR Configuration */
    SUBSECTION("DCM_GPR RX Clock Bypass");
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    bool rx_bypass = (dcmrwf3 >> 13) & 1;
    LOG_I(TAG, "DCMRWF3 = 0x%08lX", (unsigned long)dcmrwf3);
    LOG_I(TAG, "  RX_CLK_MUX_BYPASS [13] = %d", rx_bypass);

    if (rx_bypass) {
        LOG_I(TAG, "  -> RX_CLK comes from EXTERNAL PIN (correct for RGMII)");
    } else {
        LOG_I(TAG, "  -> RX_CLK comes from MUX_7 (WRONG for RGMII!)");
        LOG_I(TAG, "  [ACTION] Set DCMRWF3 |= 0x2000 to enable bypass");
    }

    /* MUX_7 Status (should be bypassed, but let's check) */
    SUBSECTION("MC_CGM MUX_7 (GMAC0_RX_CLK)");
    LOG_I(TAG, "MUX_7_CSC  = 0x%08lX", (unsigned long)IP_MC_CGM->MUX_7_CSC);
    LOG_I(TAG, "MUX_7_CSS  = 0x%08lX", (unsigned long)IP_MC_CGM->MUX_7_CSS);
    LOG_I(TAG, "MUX_7_DC_0 = 0x%08lX", (unsigned long)IP_MC_CGM->MUX_7_DC_0);

    uint8_t clk_src = (IP_MC_CGM->MUX_7_CSS >> 24) & 0x3F;
    LOG_I(TAG, "  Clock Source = %d", clk_src);

    if (rx_bypass) {
        LOG_I(TAG, "  (MUX_7 is bypassed - above values are not used)");
    }

    /* IMCR Configuration */
    SUBSECTION("SIUL2 IMCR[300] for RX_CLK (PTB22)");
    uint32_t imcr_rxclk = read_imcr(IMCR_GMAC0_RX_CLK);
    LOG_I(TAG, "IMCR[300] = 0x%02lX", (unsigned long)(imcr_rxclk & 0xFF));
    LOG_I(TAG, "  SSS [3:0] = %lu", (unsigned long)(imcr_rxclk & 0x0F));

    /*
     * For PTB22 -> GMAC0_RX_CLK, expected SSS depends on alternate function
     * Check S32K388 IO Signal Table for correct value
     */
    LOG_I(TAG, "");
    LOG_I(TAG, "Expected IMCR[300] values for RX_CLK:");
    LOG_I(TAG, "  0x07 = PTB22 -> GMAC0_RGMII_RX_CLK (check IO table)");

    /* MSCR Configuration */
    SUBSECTION("SIUL2 MSCR[54] for PTB22 (RX_CLK pin)");
    uint32_t mscr_rxclk = read_mscr(MSCR_PTB22);
    LOG_I(TAG, "MSCR[54] = 0x%08lX", (unsigned long)mscr_rxclk);
    LOG_I(TAG, "  SSS [3:0] = %lu (Should be 0 for input)", (unsigned long)(mscr_rxclk & 0x0F));
    LOG_I(TAG, "  IBE [19]  = %lu (Input Buffer Enable)", (unsigned long)((mscr_rxclk >> 19) & 1));

    bool ibe = (mscr_rxclk >> 19) & 1;
    if (!ibe) {
        LOG_I(TAG, "  [PROBLEM] Input Buffer DISABLED! RX_CLK cannot enter chip.");
        LOG_I(TAG, "  [ACTION] Set MSCR[54] |= (1 << 19)");
    }

    /* LAN9646 TX Delay */
    SUBSECTION("LAN9646 Port 6 TX Delay (affects RX_CLK timing)");
    uint8_t xmii_ctrl1 = lan_read8(LAN_XMII_CTRL1);
    bool tx_delay = (xmii_ctrl1 >> 3) & 1;
    LOG_I(TAG, "XMII_CTRL1 = 0x%02X", xmii_ctrl1);
    LOG_I(TAG, "  TX Delay [3] = %d -> %s", tx_delay, tx_delay ? "ON (+1.5ns)" : "OFF");
    LOG_I(TAG, "");
    LOG_I(TAG, "TX Delay adds ~1.5ns to signals FROM LAN9646 TO S32K388.");
    LOG_I(TAG, "This affects: RX_CLK, RXD0-3, RX_CTL timing into S32K388.");
}

bool rx_debug_verify_rx_clk(void) {
    bool ok = true;

    /* Check bypass */
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    if (!((dcmrwf3 >> 13) & 1)) {
        LOG_I(TAG, "[FAIL] RX_CLK bypass not enabled");
        ok = false;
    }

    /* Check IMCR */
    uint32_t imcr = read_imcr(IMCR_GMAC0_RX_CLK);
    if ((imcr & 0x0F) == 0) {
        LOG_I(TAG, "[WARN] IMCR[300] SSS=0, may need different value");
    }

    /* Check MSCR IBE */
    uint32_t mscr = read_mscr(MSCR_PTB22);
    if (!((mscr >> 19) & 1)) {
        LOG_I(TAG, "[FAIL] PTB22 Input Buffer disabled");
        ok = false;
    }

    return ok;
}

/*===========================================================================*/
/*                          IMCR/PIN CONFIGURATION                            */
/*===========================================================================*/

void rx_debug_dump_imcr(void) {
    SEPARATOR("SIUL2 IMCR CONFIGURATION FOR GMAC0 RX");

    LOG_I(TAG, "");
    LOG_I(TAG, "These registers select which pin routes to GMAC0 RX signals.");
    LOG_I(TAG, "SSS field [3:0] selects the input source.");
    LOG_I(TAG, "");

    struct {
        const char* name;
        const char* pin;
        uint16_t index;
    } imcr_list[] = {
        { "RX_CLK", "PTB22", IMCR_GMAC0_RX_CLK },
        { "RX_CTL", "PTC16", IMCR_GMAC0_RX_CTL },
        { "RXD0",   "PTC14", IMCR_GMAC0_RXD0 },
        { "RXD1",   "PTC15", IMCR_GMAC0_RXD1 },
        { "RXD2",   "PTB23", IMCR_GMAC0_RXD2 },
        { "RXD3",   "PTB24", IMCR_GMAC0_RXD3 },
    };

    LOG_I(TAG, "Signal   | Pin   | IMCR  | Value | SSS");
    LOG_I(TAG, "---------+-------+-------+-------+-----");

    for (int i = 0; i < 6; i++) {
        uint32_t val = read_imcr(imcr_list[i].index);
        LOG_I(TAG, "%-8s | %-5s | [%3d] |  0x%02lX | %lu",
              imcr_list[i].name,
              imcr_list[i].pin,
              imcr_list[i].index,
              (unsigned long)(val & 0xFF),
              (unsigned long)(val & 0x0F));
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "Note: Check S32K388 IO Signal Description Table for");
    LOG_I(TAG, "      correct SSS values for your pin assignment.");
}

void rx_debug_dump_mscr(void) {
    SEPARATOR("SIUL2 MSCR CONFIGURATION FOR RX PINS");

    LOG_I(TAG, "");
    LOG_I(TAG, "Key bits for input pins:");
    LOG_I(TAG, "  IBE [19] = Input Buffer Enable (MUST be 1 for RX pins)");
    LOG_I(TAG, "  SSS [3:0] = Should be 0 for pure inputs");
    LOG_I(TAG, "");

    struct {
        const char* name;
        const char* pin;
        uint16_t index;
    } mscr_list[] = {
        { "RX_CLK", "PTB22", MSCR_PTB22 },
        { "RXD2",   "PTB23", MSCR_PTB23 },
        { "RXD3",   "PTB24", MSCR_PTB24 },
        { "RXD0",   "PTC14", MSCR_PTC14 },
        { "RXD1",   "PTC15", MSCR_PTC15 },
        { "RX_CTL", "PTC16", MSCR_PTC16 },
    };

    LOG_I(TAG, "Signal   | Pin   | MSCR | Value      | IBE | SSS");
    LOG_I(TAG, "---------+-------+------+------------+-----+-----");

    for (int i = 0; i < 6; i++) {
        uint32_t val = read_mscr(mscr_list[i].index);
        bool ibe = (val >> 19) & 1;
        LOG_I(TAG, "%-8s | %-5s | [%2d] | 0x%08lX |  %d  |  %lu",
              mscr_list[i].name,
              mscr_list[i].pin,
              mscr_list[i].index,
              (unsigned long)val,
              ibe,
              (unsigned long)(val & 0x0F));

        if (!ibe) {
            LOG_I(TAG, "         [PROBLEM] IBE=0, input buffer disabled!");
        }
    }
}

bool rx_debug_verify_rx_pins(void) {
    bool ok = true;
    uint16_t mscr_indices[] = { MSCR_PTB22, MSCR_PTB23, MSCR_PTB24,
                                 MSCR_PTC14, MSCR_PTC15, MSCR_PTC16 };
    const char* names[] = { "RX_CLK", "RXD2", "RXD3", "RXD0", "RXD1", "RX_CTL" };

    for (int i = 0; i < 6; i++) {
        uint32_t mscr = read_mscr(mscr_indices[i]);
        if (!((mscr >> 19) & 1)) {
            LOG_I(TAG, "[FAIL] %s: Input Buffer disabled", names[i]);
            ok = false;
        }
    }

    return ok;
}

/*===========================================================================*/
/*                          GMAC RX STATUS                                    */
/*===========================================================================*/

void rx_debug_dump_dma_status(void) {
    SEPARATOR("GMAC DMA RX STATUS");

    uint32_t dma_status = IP_GMAC_0->DMA_CH0_STATUS;
    uint32_t dma_rx_ctrl = IP_GMAC_0->DMA_CH0_RX_CONTROL;
    uint32_t dma_debug = IP_GMAC_0->DMA_DEBUG_STATUS0;

    LOG_I(TAG, "DMA_CH0_STATUS     = 0x%08lX", (unsigned long)dma_status);
    LOG_I(TAG, "  TI [0]  = %lu (Transmit Interrupt)", (unsigned long)(dma_status & 1));
    LOG_I(TAG, "  RI [6]  = %lu (Receive Interrupt)", (unsigned long)((dma_status >> 6) & 1));
    LOG_I(TAG, "  RBU [7] = %lu (RX Buffer Unavailable)", (unsigned long)((dma_status >> 7) & 1));
    LOG_I(TAG, "  RPS [8] = %lu (RX Process Stopped)", (unsigned long)((dma_status >> 8) & 1));
    LOG_I(TAG, "  FBE [12]= %lu (Fatal Bus Error)", (unsigned long)((dma_status >> 12) & 1));

    LOG_I(TAG, "");
    LOG_I(TAG, "DMA_CH0_RX_CONTROL = 0x%08lX", (unsigned long)dma_rx_ctrl);
    LOG_I(TAG, "  SR [0]  = %lu (Start Receive)", (unsigned long)(dma_rx_ctrl & 1));
    LOG_I(TAG, "  RBSZ [14:1] = %lu (RX Buffer Size / 4)", (unsigned long)((dma_rx_ctrl >> 1) & 0x3FFF));

    if (!(dma_rx_ctrl & 1)) {
        LOG_I(TAG, "  [PROBLEM] RX DMA not started! SR=0");
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "DMA_DEBUG_STATUS0  = 0x%08lX", (unsigned long)dma_debug);

    uint8_t rps = (dma_debug >> 8) & 0x07;
    const char* rps_str[] = {
        "Stopped (reset or stop cmd)",
        "Running (fetching RX descriptor)",
        "Reserved",
        "Running (waiting for RX packet)",
        "Suspended (RX descriptor unavail)",
        "Running (closing RX descriptor)",
        "Reserved",
        "Running (transferring to memory)"
    };
    LOG_I(TAG, "  RPS [10:8] = %d -> %s", rps, rps_str[rps]);

    uint8_t tps = dma_debug & 0x0F;
    LOG_I(TAG, "  TPS [3:0]  = %d (TX Process State)", tps);
}

void rx_debug_dump_mtl_status(void) {
    SEPARATOR("GMAC MTL RX QUEUE STATUS");

    uint32_t mtl_op = IP_GMAC_0->MTL_OPERATION_MODE;
    uint32_t mtl_rxq_op = IP_GMAC_0->MTL_RXQ0_OPERATION_MODE;
    uint32_t mtl_rxq_dbg = IP_GMAC_0->MTL_RXQ0_DEBUG;

    LOG_I(TAG, "MTL_OPERATION_MODE      = 0x%08lX", (unsigned long)mtl_op);
    LOG_I(TAG, "MTL_RXQ0_OPERATION_MODE = 0x%08lX", (unsigned long)mtl_rxq_op);
    LOG_I(TAG, "  RQS [24:20] = %lu (RX Queue Size)", (unsigned long)((mtl_rxq_op >> 20) & 0x1F));

    LOG_I(TAG, "");
    LOG_I(TAG, "MTL_RXQ0_DEBUG = 0x%08lX", (unsigned long)mtl_rxq_dbg);

    uint8_t rxqsts = (mtl_rxq_dbg >> 4) & 0x03;
    const char* rxqsts_str[] = { "Empty", "Below threshold", "Above threshold", "Full" };
    LOG_I(TAG, "  RXQSTS [5:4] = %d -> %s", rxqsts, rxqsts_str[rxqsts]);

    uint8_t prxq = (mtl_rxq_dbg >> 16) & 0x3F;
    LOG_I(TAG, "  PRXQ [21:16] = %d (Packets in RX Queue)", prxq);

    uint8_t rwcsts = mtl_rxq_dbg & 1;
    LOG_I(TAG, "  RWCSTS [0]   = %d (RX Write Controller Status)", rwcsts);
}

bool rx_debug_check_dma_ready(void) {
    uint32_t rx_ctrl = IP_GMAC_0->DMA_CH0_RX_CONTROL;
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;

    bool dma_started = (rx_ctrl & 1) != 0;
    bool rx_enabled = (mac_cfg & 1) != 0;

    if (!dma_started) {
        LOG_I(TAG, "[FAIL] DMA RX not started");
        return false;
    }

    if (!rx_enabled) {
        LOG_I(TAG, "[FAIL] MAC RX not enabled");
        return false;
    }

    return true;
}

/*===========================================================================*/
/*                          RX COUNTERS                                       */
/*===========================================================================*/

void rx_debug_dump_gmac_counters(void) {
    SEPARATOR("GMAC RX COUNTERS (MMC)");

    LOG_I(TAG, "");
    LOG_I(TAG, "Counter                      | Value");
    LOG_I(TAG, "-----------------------------+------------");
    LOG_I(TAG, "RX_PACKETS_COUNT_GOOD_BAD    | %10lu", (unsigned long)IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD);
    LOG_I(TAG, "RX_OCTET_COUNT_GOOD          | %10lu", (unsigned long)IP_GMAC_0->RX_OCTET_COUNT_GOOD);
    LOG_I(TAG, "RX_BROADCAST_PACKETS_GOOD    | %10lu", (unsigned long)IP_GMAC_0->RX_BROADCAST_PACKETS_GOOD);
    LOG_I(TAG, "RX_MULTICAST_PACKETS_GOOD    | %10lu", (unsigned long)IP_GMAC_0->RX_MULTICAST_PACKETS_GOOD);
    LOG_I(TAG, "RX_UNICAST_PACKETS_GOOD      | %10lu", (unsigned long)IP_GMAC_0->RX_UNICAST_PACKETS_GOOD);
    LOG_I(TAG, "-----------------------------+------------");
    LOG_I(TAG, "RX_CRC_ERROR_PACKETS         | %10lu", (unsigned long)IP_GMAC_0->RX_CRC_ERROR_PACKETS);
    LOG_I(TAG, "RX_ALIGNMENT_ERROR_PACKETS   | %10lu", (unsigned long)IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS);
    LOG_I(TAG, "RX_RUNT_ERROR_PACKETS        | %10lu", (unsigned long)IP_GMAC_0->RX_RUNT_ERROR_PACKETS);
    LOG_I(TAG, "RX_JABBER_ERROR_PACKETS      | %10lu", (unsigned long)IP_GMAC_0->RX_JABBER_ERROR_PACKETS);
    LOG_I(TAG, "RX_LENGTH_ERROR_PACKETS      | %10lu", (unsigned long)IP_GMAC_0->RX_LENGTH_ERROR_PACKETS);
    LOG_I(TAG, "RX_FIFO_OVERFLOW_PACKETS     | %10lu", (unsigned long)IP_GMAC_0->RX_FIFO_OVERFLOW_PACKETS);

    uint32_t total_err = IP_GMAC_0->RX_CRC_ERROR_PACKETS +
                         IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS +
                         IP_GMAC_0->RX_RUNT_ERROR_PACKETS +
                         IP_GMAC_0->RX_JABBER_ERROR_PACKETS;

    LOG_I(TAG, "-----------------------------+------------");
    LOG_I(TAG, "Total Error Packets          | %10lu", (unsigned long)total_err);

    if (IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD == 0) {
        LOG_I(TAG, "");
        LOG_I(TAG, "[INFO] No packets received! Check:");
        LOG_I(TAG, "  1. RX_CLK signal from LAN9646");
        LOG_I(TAG, "  2. RXD0-3, RX_CTL connections");
        LOG_I(TAG, "  3. IMCR input mux settings");
        LOG_I(TAG, "  4. LAN9646 Port 6 TX enable");
    }
}

void rx_debug_dump_lan9646_tx_counters(void) {
    SEPARATOR("LAN9646 PORT 6 TX COUNTERS");

    LOG_I(TAG, "");
    LOG_I(TAG, "These counters show what LAN9646 is SENDING to S32K388.");
    LOG_I(TAG, "(LAN9646 TX = S32K388 RX)");
    LOG_I(TAG, "");

    uint32_t tx_bcast = lan_read_mib(6, 0x63);      /* TX broadcast */
    uint32_t tx_mcast = lan_read_mib(6, 0x64);      /* TX multicast */
    uint32_t tx_ucast = lan_read_mib(6, 0x65);      /* TX unicast */
    uint32_t tx_total = tx_bcast + tx_mcast + tx_ucast;
    uint32_t tx_bytes = lan_read_mib(6, 0x81);      /* TX total bytes */
    uint32_t tx_late = lan_read_mib(6, 0x61);       /* TX late collision */
    uint32_t tx_excess = lan_read_mib(6, 0x68);     /* TX excess collision */
    uint32_t tx_drop = lan_read_mib(6, 0x83);       /* TX dropped */

    LOG_I(TAG, "Counter                | Value");
    LOG_I(TAG, "-----------------------+------------");
    LOG_I(TAG, "TX Broadcast (0x63)    | %10lu", (unsigned long)tx_bcast);
    LOG_I(TAG, "TX Multicast (0x64)    | %10lu", (unsigned long)tx_mcast);
    LOG_I(TAG, "TX Unicast (0x65)      | %10lu", (unsigned long)tx_ucast);
    LOG_I(TAG, "TX Total Packets       | %10lu", (unsigned long)tx_total);
    LOG_I(TAG, "TX Total Bytes (0x81)  | %10lu", (unsigned long)tx_bytes);
    LOG_I(TAG, "-----------------------+------------");
    LOG_I(TAG, "TX Late Collision      | %10lu", (unsigned long)tx_late);
    LOG_I(TAG, "TX Excess Collision    | %10lu", (unsigned long)tx_excess);
    LOG_I(TAG, "TX Dropped (0x83)      | %10lu", (unsigned long)tx_drop);

    if (tx_total == 0) {
        LOG_I(TAG, "");
        LOG_I(TAG, "[INFO] LAN9646 not transmitting to GMAC!");
        LOG_I(TAG, "  Check Port 6 TX enable (MSTP_STATE)");
    }
}

void rx_debug_monitor_counters(uint32_t duration_ms) {
    SEPARATOR("COUNTER MONITORING");

    LOG_I(TAG, "Monitoring for %lu ms...", (unsigned long)duration_ms);
    LOG_I(TAG, "");

    /* Record initial values */
    uint32_t gmac_rx_start = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
    uint32_t lan_tx_start = lan_read_mib(6, 0x63) +
                            lan_read_mib(6, 0x64) +
                            lan_read_mib(6, 0x65);

    /* Wait */
    if (g_delay) g_delay(duration_ms);

    /* Record final values */
    uint32_t gmac_rx_end = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
    uint32_t lan_tx_end = lan_read_mib(6, 0x63) +
                          lan_read_mib(6, 0x64) +
                          lan_read_mib(6, 0x65);

    uint32_t gmac_delta = gmac_rx_end - gmac_rx_start;
    uint32_t lan_delta = lan_tx_end - lan_tx_start;

    LOG_I(TAG, "Results:");
    LOG_I(TAG, "  GMAC RX packets: %lu -> %lu (delta: %lu)",
          (unsigned long)gmac_rx_start, (unsigned long)gmac_rx_end,
          (unsigned long)gmac_delta);
    LOG_I(TAG, "  LAN9646 TX packets: %lu -> %lu (delta: %lu)",
          (unsigned long)lan_tx_start, (unsigned long)lan_tx_end,
          (unsigned long)lan_delta);

    if (lan_delta > 0 && gmac_delta == 0) {
        LOG_I(TAG, "");
        LOG_I(TAG, "[PROBLEM] LAN9646 is transmitting but GMAC not receiving!");
        LOG_I(TAG, "  -> RX path has issues (clock/timing/pins)");
    }
}

/*===========================================================================*/
/*                          LAN9646 TX STATUS                                 */
/*===========================================================================*/

void rx_debug_dump_lan9646_tx_config(void) {
    SEPARATOR("LAN9646 PORT 6 TX CONFIGURATION");

    LOG_I(TAG, "");
    LOG_I(TAG, "Port 6 TX -> S32K388 GMAC RX");
    LOG_I(TAG, "");

    /* XMII Control */
    SUBSECTION("XMII Control");
    uint8_t ctrl0 = lan_read8(LAN_XMII_CTRL0);
    uint8_t ctrl1 = lan_read8(LAN_XMII_CTRL1);

    LOG_I(TAG, "XMII_CTRL0 [0x6300] = 0x%02X", ctrl0);
    LOG_I(TAG, "XMII_CTRL1 [0x6301] = 0x%02X", ctrl1);
    LOG_I(TAG, "  TX Delay [3] = %d -> %s",
          (ctrl1 >> 3) & 1,
          ((ctrl1 >> 3) & 1) ? "ON (+1.5ns to S32K388)" : "OFF");

    /* Port Status */
    SUBSECTION("Port Status");
    uint8_t status = lan_read8(LAN_PORT_STATUS);
    uint8_t speed = (status >> 3) & 0x03;
    const char* speed_str[] = { "10M", "100M", "1000M", "Reserved" };

    LOG_I(TAG, "PORT_STATUS [0x6030] = 0x%02X", status);
    LOG_I(TAG, "  Speed [4:3]  = %d -> %s", speed, speed_str[speed]);
    LOG_I(TAG, "  Duplex [2]   = %d -> %s",
          (status >> 2) & 1,
          ((status >> 2) & 1) ? "Full" : "Half");

    /* MSTP State - TX Enable */
    SUBSECTION("MSTP State (TX/RX Enable)");
    uint8_t mstp = lan_read8(LAN_MSTP_STATE);
    bool tx_en = (mstp >> 2) & 1;
    bool rx_en = (mstp >> 1) & 1;

    LOG_I(TAG, "MSTP_STATE [0x6B04] = 0x%02X", mstp);
    LOG_I(TAG, "  TX Enable [2] = %d -> %s", tx_en, tx_en ? "ENABLED" : "DISABLED");
    LOG_I(TAG, "  RX Enable [1] = %d -> %s", rx_en, rx_en ? "ENABLED" : "DISABLED");

    if (!tx_en) {
        LOG_I(TAG, "  [PROBLEM] Port 6 TX is DISABLED!");
        LOG_I(TAG, "  [ACTION] Write 0x06 to MSTP_STATE to enable TX/RX");
    }

    /* Operation Control (Loopback) */
    SUBSECTION("Operation Control (Loopback)");
    uint8_t op_ctrl = lan_read8(LAN_OP_CTRL0);
    bool loopback = (op_ctrl >> 6) & 1;

    LOG_I(TAG, "OP_CTRL0 [0x6020] = 0x%02X", op_ctrl);
    LOG_I(TAG, "  Remote Loopback [6] = %d -> %s", loopback, loopback ? "ON" : "OFF");
}

/*===========================================================================*/
/*                          RX PATH TESTS                                     */
/*===========================================================================*/

static uint8_t g_test_packet[64] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* Dest: Broadcast */
    0x10, 0x11, 0x22, 0x33, 0x44, 0x55,  /* Src: Test */
    0x88, 0xB5,                          /* EtherType: Test */
    /* Payload */
    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
};

uint32_t rx_debug_test_loopback(uint32_t count) {
    SEPARATOR("RX LOOPBACK TEST");

    LOG_I(TAG, "");
    LOG_I(TAG, "Test: GMAC TX -> LAN9646 P6 -> Loopback -> LAN9646 P6 -> GMAC RX");
    LOG_I(TAG, "Sending %lu packets...", (unsigned long)count);
    LOG_I(TAG, "");

    /* Enable loopback on Port 6 */
    uint8_t op_ctrl = lan_read8(LAN_OP_CTRL0);
    lan_write8(LAN_OP_CTRL0, op_ctrl | 0x40);  /* Bit 6: Remote loopback */

    if (g_delay) g_delay(10);

    /* Clear counters */
    lan_flush_mib(6);

    /* Record initial GMAC RX count */
    uint32_t rx_before = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;

    /* Send packets */
    Gmac_Ip_BufferType buf;
    buf.Data = g_test_packet;
    buf.Length = 64;

    for (uint32_t i = 0; i < count; i++) {
        Gmac_Ip_SendFrame(0, 0, &buf, NULL);
        if (g_delay) g_delay(1);
    }

    /* Wait for loopback */
    if (g_delay) g_delay(100);

    /* Check results */
    uint32_t rx_after = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
    uint32_t received = rx_after - rx_before;

    /* Disable loopback */
    lan_write8(LAN_OP_CTRL0, op_ctrl);

    /* Report */
    LOG_I(TAG, "Results:");
    LOG_I(TAG, "  GMAC TX: %lu packets sent", (unsigned long)count);
    LOG_I(TAG, "  GMAC RX: %lu packets received", (unsigned long)received);

    uint32_t lan_rx = lan_read_mib(6, 0x0A) + lan_read_mib(6, 0x0B) + lan_read_mib(6, 0x0C);
    uint32_t lan_tx = lan_read_mib(6, 0x63) + lan_read_mib(6, 0x64) + lan_read_mib(6, 0x65);

    LOG_I(TAG, "  LAN9646 P6 RX (from GMAC): %lu", (unsigned long)lan_rx);
    LOG_I(TAG, "  LAN9646 P6 TX (to GMAC):   %lu", (unsigned long)lan_tx);

    if (received == 0) {
        LOG_I(TAG, "");
        LOG_I(TAG, "[FAIL] No packets received by GMAC!");

        if (lan_rx > 0 && lan_tx > 0) {
            LOG_I(TAG, "  LAN9646 loopback is working.");
            LOG_I(TAG, "  Problem is on RX path: LAN9646 TX -> S32K388 GMAC RX");
            LOG_I(TAG, "  Check: RX_CLK, RXD0-3, RX_CTL, IMCR, timing");
        } else if (lan_rx > 0) {
            LOG_I(TAG, "  LAN9646 received from GMAC but not looping back.");
            LOG_I(TAG, "  Check loopback configuration.");
        } else {
            LOG_I(TAG, "  LAN9646 not receiving from GMAC either.");
            LOG_I(TAG, "  Check TX path first.");
        }
    } else if (received < count) {
        LOG_I(TAG, "");
        LOG_I(TAG, "[PARTIAL] Only %lu/%lu received.", (unsigned long)received, (unsigned long)count);
        LOG_I(TAG, "  Possible timing issues. Try different delay settings.");
    } else {
        LOG_I(TAG, "");
        LOG_I(TAG, "[OK] All packets received!");
    }

    return received;
}

void rx_debug_delay_sweep(void) {
    SEPARATOR("RX PATH DELAY SWEEP");

    LOG_I(TAG, "");
    LOG_I(TAG, "Testing all LAN9646 TX delay combinations for RX path...");
    LOG_I(TAG, "LAN9646 TX delay affects timing into S32K388 GMAC RX.");
    LOG_I(TAG, "");

    LOG_I(TAG, "TX_DLY | Sent | Received | Status");
    LOG_I(TAG, "-------+------+----------+--------");

    const uint32_t test_count = 10;
    uint32_t best_rx = 0;
    bool best_delay = false;

    for (int tx_delay = 0; tx_delay <= 1; tx_delay++) {
        /* Set TX delay */
        uint8_t ctrl1 = lan_read8(LAN_XMII_CTRL1);
        if (tx_delay) {
            ctrl1 |= 0x08;
        } else {
            ctrl1 &= ~0x08;
        }
        lan_write8(LAN_XMII_CTRL1, ctrl1);

        if (g_delay) g_delay(10);

        /* Enable loopback */
        uint8_t op_ctrl = lan_read8(LAN_OP_CTRL0);
        lan_write8(LAN_OP_CTRL0, op_ctrl | 0x40);
        if (g_delay) g_delay(10);

        /* Clear and test */
        lan_flush_mib(6);
        uint32_t rx_before = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;

        Gmac_Ip_BufferType buf;
        buf.Data = g_test_packet;
        buf.Length = 64;

        for (uint32_t i = 0; i < test_count; i++) {
            Gmac_Ip_SendFrame(0, 0, &buf, NULL);
            if (g_delay) g_delay(1);
        }

        if (g_delay) g_delay(50);

        uint32_t received = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD - rx_before;

        /* Disable loopback */
        lan_write8(LAN_OP_CTRL0, op_ctrl);

        const char* status;
        if (received == test_count) {
            status = "OK";
        } else if (received > 0) {
            status = "Partial";
        } else {
            status = "FAIL";
        }

        LOG_I(TAG, "  %s  |  %2lu  |    %2lu    | %s",
              tx_delay ? "ON " : "OFF",
              (unsigned long)test_count,
              (unsigned long)received,
              status);

        if (received > best_rx) {
            best_rx = received;
            best_delay = tx_delay;
        }
    }

    LOG_I(TAG, "");
    if (best_rx > 0) {
        LOG_I(TAG, "Best result: TX_DELAY=%s with %lu/%lu received",
              best_delay ? "ON" : "OFF",
              (unsigned long)best_rx,
              (unsigned long)test_count);

        /* Apply best setting */
        uint8_t ctrl1 = lan_read8(LAN_XMII_CTRL1);
        if (best_delay) {
            ctrl1 |= 0x08;
        } else {
            ctrl1 &= ~0x08;
        }
        lan_write8(LAN_XMII_CTRL1, ctrl1);
        LOG_I(TAG, "Applied TX_DELAY=%s", best_delay ? "ON" : "OFF");
    } else {
        LOG_I(TAG, "[FAIL] No packets received with any delay setting!");
        LOG_I(TAG, "Check RX_CLK and pin configurations.");
    }
}

void rx_debug_set_lan9646_tx_delay(bool tx_delay) {
    uint8_t ctrl1 = lan_read8(LAN_XMII_CTRL1);
    if (tx_delay) {
        ctrl1 |= 0x08;
    } else {
        ctrl1 &= ~0x08;
    }
    lan_write8(LAN_XMII_CTRL1, ctrl1);
    LOG_I(TAG, "Set LAN9646 TX_DELAY = %s", tx_delay ? "ON" : "OFF");
}

/*===========================================================================*/
/*                          FULL ANALYSIS                                     */
/*===========================================================================*/

void rx_debug_full_analysis(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "##                                                            ##");
    LOG_I(TAG, "##        RGMII RX PATH FULL ANALYSIS                         ##");
    LOG_I(TAG, "##        LAN9646 Port 6 TX -> S32K388 GMAC0 RX               ##");
    LOG_I(TAG, "##                                                            ##");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "################################################################");

    /* Step 1: RX_CLK Analysis */
    rx_debug_analyze_rx_clk();

    /* Step 2: All RX IMCR */
    rx_debug_dump_imcr();

    /* Step 3: All RX MSCR */
    rx_debug_dump_mscr();

    /* Step 4: GMAC DMA Status */
    rx_debug_dump_dma_status();

    /* Step 5: GMAC MTL Status */
    rx_debug_dump_mtl_status();

    /* Step 6: LAN9646 TX Config */
    rx_debug_dump_lan9646_tx_config();

    /* Step 7: Counters */
    rx_debug_dump_gmac_counters();
    rx_debug_dump_lan9646_tx_counters();

    /* Step 8: Diagnosis */
    SEPARATOR("DIAGNOSIS");

    bool clk_ok = rx_debug_verify_rx_clk();
    bool pins_ok = rx_debug_verify_rx_pins();
    bool dma_ok = rx_debug_check_dma_ready();

    LOG_I(TAG, "");
    LOG_I(TAG, "Check                  | Status");
    LOG_I(TAG, "-----------------------+--------");
    LOG_I(TAG, "RX_CLK Configuration   | %s", clk_ok ? "OK" : "FAIL");
    LOG_I(TAG, "RX Pin Configuration   | %s", pins_ok ? "OK" : "FAIL");
    LOG_I(TAG, "GMAC DMA Ready         | %s", dma_ok ? "OK" : "FAIL");

    if (!clk_ok || !pins_ok || !dma_ok) {
        LOG_I(TAG, "");
        LOG_I(TAG, "Fix the above issues before testing loopback.");
    } else {
        LOG_I(TAG, "");
        LOG_I(TAG, "Configuration looks OK. Running loopback test...");
        rx_debug_test_loopback(10);
    }
}

void rx_debug_dump_gmac_rx_config(void) {
    SEPARATOR("GMAC RX CONFIGURATION");

    /* MAC Configuration */
    SUBSECTION("MAC Configuration");
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    LOG_I(TAG, "MAC_CONFIGURATION = 0x%08lX", (unsigned long)mac_cfg);
    LOG_I(TAG, "  RE [0] = %lu -> RX %s", (unsigned long)(mac_cfg & 1), (mac_cfg & 1) ? "ENABLED" : "DISABLED");

    /* MAC Debug */
    SUBSECTION("MAC Debug Status");
    uint32_t mac_dbg = IP_GMAC_0->MAC_DEBUG;
    LOG_I(TAG, "MAC_DEBUG = 0x%08lX", (unsigned long)mac_dbg);
    LOG_I(TAG, "  RPESTS [0]   = %lu (RGMII RX active)", (unsigned long)(mac_dbg & 1));
    LOG_I(TAG, "  RFCFCSTS [1] = %lu (RX FIFO fill)", (unsigned long)((mac_dbg >> 1) & 1));

    /* PHY Interface Status */
    SUBSECTION("PHY Interface Status");
    uint32_t phyif = IP_GMAC_0->MAC_PHYIF_CONTROL_STATUS;
    LOG_I(TAG, "MAC_PHYIF_CONTROL_STATUS = 0x%08lX", (unsigned long)phyif);
    LOG_I(TAG, "  LNKSTS [19]     = %lu (Link Status)", (unsigned long)((phyif >> 19) & 1));
    LOG_I(TAG, "  LNKSPEED [18:17]= %lu", (unsigned long)((phyif >> 17) & 0x03));
    LOG_I(TAG, "  LNKMOD [16]     = %lu (Link Mode)", (unsigned long)((phyif >> 16) & 1));

    if (!((phyif >> 19) & 1)) {
        LOG_I(TAG, "  [INFO] Link Status = 0. This may be normal for RGMII");
        LOG_I(TAG, "         with forced speed/duplex configuration.");
    }

    /* DCM_GPR */
    SUBSECTION("DCM_GPR (RX Clock Bypass)");
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    LOG_I(TAG, "DCMRWF3 = 0x%08lX", (unsigned long)dcmrwf3);
    LOG_I(TAG, "  RX_CLK_MUX_BYPASS [13] = %lu", (unsigned long)((dcmrwf3 >> 13) & 1));

    /* DMA RX Control */
    rx_debug_dump_dma_status();
}

void rx_debug_print_troubleshooting(void) {
    SEPARATOR("RX PATH TROUBLESHOOTING GUIDE");

    LOG_I(TAG, "");
    LOG_I(TAG, "If GMAC is not receiving packets, check in this order:");
    LOG_I(TAG, "");
    LOG_I(TAG, "1. RX_CLK Signal (MOST CRITICAL)");
    LOG_I(TAG, "   - DCMRWF3[13] must be 1 (bypass mode)");
    LOG_I(TAG, "   - IMCR[300] must route PTB22 to GMAC0_RX_CLK");
    LOG_I(TAG, "   - PTB22 MSCR must have IBE=1 (input buffer enabled)");
    LOG_I(TAG, "   - LAN9646 TX_DELAY affects RX_CLK timing");
    LOG_I(TAG, "");
    LOG_I(TAG, "2. RX Data Pins (RXD0-3, RX_CTL)");
    LOG_I(TAG, "   - All MSCR must have IBE=1");
    LOG_I(TAG, "   - All IMCR must route correct pins");
    LOG_I(TAG, "   - Check physical connections on PCB");
    LOG_I(TAG, "");
    LOG_I(TAG, "3. LAN9646 Port 6 TX Enable");
    LOG_I(TAG, "   - MSTP_STATE[2] = 1 for TX enable");
    LOG_I(TAG, "   - Check Port 6 TX counters are incrementing");
    LOG_I(TAG, "");
    LOG_I(TAG, "4. GMAC RX DMA");
    LOG_I(TAG, "   - DMA_CH0_RX_CONTROL SR=1 (started)");
    LOG_I(TAG, "   - Check RX descriptor ring is setup");
    LOG_I(TAG, "");
    LOG_I(TAG, "5. RGMII Timing");
    LOG_I(TAG, "   - Try LAN9646 TX_DELAY ON and OFF");
    LOG_I(TAG, "   - Run rx_debug_delay_sweep()");
    LOG_I(TAG, "");
    LOG_I(TAG, "Commands to run:");
    LOG_I(TAG, "   rx_debug_full_analysis()  - Complete RX path analysis");
    LOG_I(TAG, "   rx_debug_analyze_rx_clk() - Focus on RX_CLK");
    LOG_I(TAG, "   rx_debug_delay_sweep()    - Find working delay");
    LOG_I(TAG, "   rx_debug_test_loopback(10)- Test with loopback");
}

void rx_debug_auto_diagnose(void) {
    SEPARATOR("AUTO DIAGNOSIS");

    LOG_I(TAG, "Running automated RX path diagnosis...");
    LOG_I(TAG, "");

    /* Step 1: Check configuration */
    LOG_I(TAG, "[Step 1] Checking configuration...");

    bool clk_ok = rx_debug_verify_rx_clk();
    bool pins_ok = rx_debug_verify_rx_pins();
    bool dma_ok = rx_debug_check_dma_ready();

    if (!clk_ok) {
        LOG_I(TAG, "");
        LOG_I(TAG, "[AUTO-FIX] Enabling RX_CLK bypass...");
        uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
        IP_DCM_GPR->DCMRWF3 = dcmrwf3 | 0x2000;  /* Set bit 13 */
        if (g_delay) g_delay(1);
        LOG_I(TAG, "  DCMRWF3: 0x%08lX -> 0x%08lX",
              (unsigned long)dcmrwf3,
              (unsigned long)IP_DCM_GPR->DCMRWF3);
    }

    if (!dma_ok) {
        LOG_I(TAG, "");
        LOG_I(TAG, "[INFO] DMA may need reinitialization.");
    }

    /* Step 2: Test loopback */
    LOG_I(TAG, "");
    LOG_I(TAG, "[Step 2] Testing loopback...");
    uint32_t rx = rx_debug_test_loopback(10);

    if (rx == 0) {
        /* Step 3: Try delay sweep */
        LOG_I(TAG, "");
        LOG_I(TAG, "[Step 3] Trying delay sweep...");
        rx_debug_delay_sweep();
    }

    /* Step 4: Final status */
    LOG_I(TAG, "");
    LOG_I(TAG, "[Step 4] Final RX counter check...");
    rx_debug_dump_gmac_counters();

    if (IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD > 0) {
        LOG_I(TAG, "");
        LOG_I(TAG, "==> RX PATH APPEARS TO BE WORKING!");
    } else {
        LOG_I(TAG, "");
        LOG_I(TAG, "==> RX PATH STILL NOT WORKING");
        LOG_I(TAG, "    Manual investigation needed.");
        LOG_I(TAG, "    Check physical connections and signal integrity.");
    }
}
