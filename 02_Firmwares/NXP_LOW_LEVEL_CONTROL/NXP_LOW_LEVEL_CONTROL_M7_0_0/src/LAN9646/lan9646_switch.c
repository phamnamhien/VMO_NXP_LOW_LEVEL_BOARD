/**
 * \file            lan9646_switch.c
 * \brief           LAN9646 High Level Switch API (Fixed per datasheet)
 */

#include "lan9646_switch.h"
#include <string.h>

/*===========================================================================*/
/*                              PRIVATE DATA                                  */
/*===========================================================================*/

static lan9646_link_cb_t g_link_callback = NULL;
static lan9646_port_status_t g_last_status[8];

/*===========================================================================*/
/*                          PRIVATE FUNCTIONS                                 */
/*===========================================================================*/

static bool prv_is_valid_port(uint8_t port) {
    return (port >= 1 && port <= 4) || (port == 6) || (port == 7);
}

static bool prv_is_phy_port(uint8_t port) {
    return (port >= 1 && port <= 4);
}

/**
 * \brief           Read MIB counter - CORRECTED per datasheet
 * \note            MIB Index goes in bits [23:16], Read Enable is bit 25
 */
static uint32_t prv_read_mib_counter(lan9646_t* h, uint8_t port, uint8_t index) {
    uint16_t base = (uint16_t)port << 12;
    uint32_t ctrl;
    uint32_t data = 0;
    uint32_t timeout = 1000;

    /* Set MIB Index [23:16] and Read Enable [25] */
    ctrl = ((uint32_t)index << 16) | LAN9646_MIB_READ_EN;
    lan9646_write_reg32(h, base | 0x0500, ctrl);

    /* Poll until bit 25 (Read Enable) auto-clears */
    do {
        lan9646_read_reg32(h, base | 0x0500, &ctrl);
        if (--timeout == 0) break;
    } while (ctrl & LAN9646_MIB_READ_EN);

    /* Read 32-bit counter data */
    lan9646_read_reg32(h, base | 0x0504, &data);

    return data;
}

/*===========================================================================*/
/*                         INITIALIZATION                                     */
/*===========================================================================*/

lan9646r_t lan9646_switch_init(lan9646_t* h) {
    uint16_t chip_id;

    if (!h) return lan9646INVPARAM;

    if (lan9646_switch_get_chip_id(h, &chip_id, NULL) != lan9646OK) {
        return lan9646BUSERR;
    }

    if (chip_id != LAN9646_CHIP_ID) {
        return lan9646ERR;
    }

    memset(g_last_status, 0, sizeof(g_last_status));
    return lan9646OK;
}

lan9646r_t lan9646_switch_get_chip_id(lan9646_t* h, uint16_t* chip_id, uint8_t* revision) {
    uint8_t id1, id2, id3;
    lan9646r_t res;

    if (!h || !chip_id) return lan9646INVPARAM;

    /* Read Chip ID MSB (0x0001) and LSB (0x0002) */
    res = lan9646_read_reg8(h, 0x0001, &id1);
    if (res != lan9646OK) return res;

    res = lan9646_read_reg8(h, 0x0002, &id2);
    if (res != lan9646OK) return res;

    /* Full 16-bit chip ID = MSB:LSB */
    *chip_id = ((uint16_t)id1 << 8) | id2;

    if (revision) {
        res = lan9646_read_reg8(h, 0x0003, &id3);
        if (res == lan9646OK) {
            *revision = (id3 >> 4) & 0x0F;
        }
    }

    return lan9646OK;
}

/*===========================================================================*/
/*                          PORT STATUS                                       */
/*===========================================================================*/

lan9646r_t lan9646_switch_get_port_status(lan9646_t* h, uint8_t port,
                                          lan9646_port_status_t* status) {
    uint8_t port_stat;
    uint16_t phy_stat;
    lan9646r_t res;

    if (!h || !status || !prv_is_valid_port(port)) {
        return lan9646INVPARAM;
    }

    status->port = port;
    status->auto_mdix = false;

    /*
     * Port Status Register (0xN030) is 8-bit for ALL ports
     * Bits 4:3 = Speed (00=10M, 01=100M, 10=1000M)
     * Bit 2 = Duplex (1=Full)
     * Bit 1 = TX Flow Control
     * Bit 0 = RX Flow Control
     */
    res = lan9646_read_reg8(h, LAN9646_REG_PORT_STATUS(port), &port_stat);
    if (res != lan9646OK) return res;

    /* Decode speed from bits [4:3] */
    uint8_t speed = (port_stat & LAN9646_PORT_STATUS_OP_SPEED_MASK)
                    >> LAN9646_PORT_STATUS_OP_SPEED_SHIFT;
    switch (speed) {
        case LAN9646_SPEED_10:   status->speed = LAN9646_SPEED_10M;   break;
        case LAN9646_SPEED_100:  status->speed = LAN9646_SPEED_100M;  break;
        case LAN9646_SPEED_1000: status->speed = LAN9646_SPEED_1000M; break;
        default:                 status->speed = LAN9646_SPEED_DOWN;  break;
    }

    /* Decode duplex from bit 2 */
    status->duplex = (port_stat & LAN9646_PORT_STATUS_OP_DUPLEX)
                     ? LAN9646_DUPLEX_FULL : LAN9646_DUPLEX_HALF;

    if (prv_is_phy_port(port)) {
        /*
         * PHY ports (1-4): Get link from PHY Basic Status Register (0xN102)
         * Bit 2 is "Latch Low" - must read TWICE per IEEE 802.3!
         * First read clears latch, second read returns current status
         */
        res = lan9646_read_reg16(h, LAN9646_REG_PHY_BASIC_STATUS(port), &phy_stat);
        if (res != lan9646OK) return res;
        res = lan9646_read_reg16(h, LAN9646_REG_PHY_BASIC_STATUS(port), &phy_stat);
        if (res != lan9646OK) return res;

        status->link_up = (phy_stat & LAN9646_PHY_LINK_STATUS) != 0;
    } else {
        /*
         * RGMII ports (6-7): No auto link detection
         * For Port 6 connected to GMAC: assume link up if speed is configured
         * In real systems, link status comes from external PHY or GMAC driver
         */
        status->link_up = (speed != 0);  /* Assume up if speed is set */
    }

    if (!status->link_up) {
        status->speed = LAN9646_SPEED_DOWN;
    }

    return lan9646OK;
}

bool lan9646_switch_get_gmac_link(lan9646_t* h, lan9646_speed_t* speed,
                                  lan9646_duplex_t* duplex) {
    lan9646_port_status_t status;

    if (lan9646_switch_get_port_status(h, LAN9646_PORT6, &status) != lan9646OK) {
        return false;
    }

    if (speed)  *speed = status.speed;
    if (duplex) *duplex = status.duplex;

    return status.link_up;
}

uint8_t lan9646_switch_get_all_phy_status(lan9646_t* h, lan9646_port_status_t* status) {
    uint8_t link_mask = 0;

    for (uint8_t port = 1; port <= 4; port++) {
        if (lan9646_switch_get_port_status(h, port, &status[port - 1]) == lan9646OK) {
            if (status[port - 1].link_up) {
                link_mask |= (1 << (port - 1));
            }
        }
    }

    return link_mask;
}

/*===========================================================================*/
/*                          PORT CONTROL                                      */
/*===========================================================================*/

lan9646r_t lan9646_switch_set_port_enable(lan9646_t* h, uint8_t port,
                                          bool tx_en, bool rx_en) {
    uint8_t state = 0;

    if (!h || !prv_is_valid_port(port)) {
        return lan9646INVPARAM;
    }

    /* Select MSTP instance 0 */
    lan9646_write_reg8(h, LAN9646_REG_PORT_MSTP_PTR(port), 0x00);

    /* Set TX/RX enable bits */
    if (tx_en) state |= LAN9646_MSTP_TX_EN;
    if (rx_en) state |= LAN9646_MSTP_RX_EN;

    return lan9646_write_reg8(h, LAN9646_REG_PORT_MSTP_STATE(port), state);
}

lan9646r_t lan9646_switch_set_rgmii_delay(lan9646_t* h, const lan9646_rgmii_delay_t* delay) {
    uint8_t val = 0;

    if (!h || !delay) return lan9646INVPARAM;

    /* Per datasheet XMII_CTRL1:
     * Bit 4 = RX Ingress Delay (1.5ns)
     * Bit 3 = TX Egress Delay (1.5ns)
     */
    if (delay->rx_delay) val |= LAN9646_XMII_RGMII_RX_DLY_EN;  /* bit 4 = 0x10 */
    if (delay->tx_delay) val |= LAN9646_XMII_RGMII_TX_DLY_EN;  /* bit 3 = 0x08 */

    return lan9646_write_reg8(h, LAN9646_REG_PORT_XMII_CTRL1(6), val);
}

lan9646r_t lan9646_switch_get_rgmii_delay(lan9646_t* h, lan9646_rgmii_delay_t* delay) {
    uint8_t val;
    lan9646r_t res;

    if (!h || !delay) return lan9646INVPARAM;

    res = lan9646_read_reg8(h, LAN9646_REG_PORT_XMII_CTRL1(6), &val);
    if (res != lan9646OK) return res;

    delay->rx_delay = (val & LAN9646_XMII_RGMII_RX_DLY_EN) != 0;  /* bit 4 */
    delay->tx_delay = (val & LAN9646_XMII_RGMII_TX_DLY_EN) != 0;  /* bit 3 */

    return lan9646OK;
}

/*===========================================================================*/
/*                          MIB COUNTERS                                      */
/*===========================================================================*/

lan9646r_t lan9646_switch_read_mib(lan9646_t* h, uint8_t port, lan9646_mib_t* mib) {
    if (!h || !mib || !prv_is_valid_port(port)) {
        return lan9646INVPARAM;
    }

    memset(mib, 0, sizeof(lan9646_mib_t));

    /* RX counters */
    mib->rx_unicast   = prv_read_mib_counter(h, port, LAN9646_MIB_RX_UNICAST);
    mib->rx_broadcast = prv_read_mib_counter(h, port, LAN9646_MIB_RX_BROADCAST);
    mib->rx_multicast = prv_read_mib_counter(h, port, LAN9646_MIB_RX_MULTICAST);
    mib->rx_bytes     = prv_read_mib_counter(h, port, LAN9646_MIB_RX_HI_PRIO_BYTE);
    mib->rx_crc_err   = prv_read_mib_counter(h, port, LAN9646_MIB_RX_CRC_ERR);
    mib->rx_undersize = prv_read_mib_counter(h, port, LAN9646_MIB_RX_UNDERSIZE);
    mib->rx_oversize  = prv_read_mib_counter(h, port, LAN9646_MIB_RX_OVERSIZE);
    mib->rx_discard   = prv_read_mib_counter(h, port, LAN9646_MIB_RX_DROP);

    /* TX counters */
    mib->tx_unicast   = prv_read_mib_counter(h, port, LAN9646_MIB_TX_UNICAST);
    mib->tx_broadcast = prv_read_mib_counter(h, port, LAN9646_MIB_TX_BROADCAST);
    mib->tx_multicast = prv_read_mib_counter(h, port, LAN9646_MIB_TX_MULTICAST);
    mib->tx_bytes     = prv_read_mib_counter(h, port, LAN9646_MIB_TX_TOTAL);
    mib->tx_collisions= prv_read_mib_counter(h, port, LAN9646_MIB_TX_TOTAL_COL);
    mib->tx_discard   = prv_read_mib_counter(h, port, LAN9646_MIB_TX_DROP);

    return lan9646OK;
}

lan9646r_t lan9646_switch_read_mib_simple(lan9646_t* h, uint8_t port,
                                          lan9646_mib_simple_t* mib) {
    if (!h || !mib || !prv_is_valid_port(port)) {
        return lan9646INVPARAM;
    }

    memset(mib, 0, sizeof(lan9646_mib_simple_t));

    mib->rx_packets = prv_read_mib_counter(h, port, LAN9646_MIB_RX_TOTAL);
    mib->tx_packets = prv_read_mib_counter(h, port, LAN9646_MIB_TX_UNICAST) +
                      prv_read_mib_counter(h, port, LAN9646_MIB_TX_BROADCAST) +
                      prv_read_mib_counter(h, port, LAN9646_MIB_TX_MULTICAST);
    mib->rx_bytes = prv_read_mib_counter(h, port, LAN9646_MIB_RX_HI_PRIO_BYTE);
    mib->tx_bytes = prv_read_mib_counter(h, port, LAN9646_MIB_TX_HI_PRIO_BYTE);

    return lan9646OK;
}

lan9646r_t lan9646_switch_flush_mib(lan9646_t* h, uint8_t port) {
    uint32_t ctrl;

    if (!h) return lan9646INVPARAM;

    if (port == 0) {
        /* Flush all ports via global control */
        lan9646_write_reg8(h, LAN9646_REG_SWITCH_MIB_CTRL,
                           LAN9646_SW_MIB_FLUSH | LAN9646_SW_MIB_FREEZE);
        lan9646_write_reg8(h, LAN9646_REG_SWITCH_MIB_CTRL, 0);
    } else if (prv_is_valid_port(port)) {
        /* Flush single port */
        uint16_t base = (uint16_t)port << 12;

        /* Enable flush for this port and trigger */
        ctrl = LAN9646_MIB_FLUSH_FREEZE_EN;
        lan9646_write_reg32(h, base | 0x0500, ctrl);
    } else {
        return lan9646INVPARAM;
    }

    return lan9646OK;
}

/*===========================================================================*/
/*                         PORT MIRRORING                                     */
/*===========================================================================*/

lan9646r_t lan9646_switch_set_port_mirror(lan9646_t* h, uint8_t sniffer_port,
                                          uint8_t source_mask, bool mirror_rx, bool mirror_tx) {
    uint8_t val;

    if (!h || !prv_is_valid_port(sniffer_port)) {
        return lan9646INVPARAM;
    }

    /* Set sniffer port */
    lan9646_read_reg8(h, LAN9646_REG_PORT_MIRROR_CTRL(sniffer_port), &val);
    val |= LAN9646_MIRROR_SNIFFER_PORT;
    lan9646_write_reg8(h, LAN9646_REG_PORT_MIRROR_CTRL(sniffer_port), val);

    /* Configure source ports */
    for (uint8_t port = 1; port <= 7; port++) {
        if (port == 5 || port == sniffer_port) continue;

        if (source_mask & (1 << (port - 1))) {
            val = 0;
            if (mirror_rx) val |= LAN9646_MIRROR_RX_SNIFF;
            if (mirror_tx) val |= LAN9646_MIRROR_TX_SNIFF;
            lan9646_write_reg8(h, LAN9646_REG_PORT_MIRROR_CTRL(port), val);
        }
    }

    return lan9646OK;
}

lan9646r_t lan9646_switch_disable_port_mirror(lan9646_t* h) {
    if (!h) return lan9646INVPARAM;

    uint8_t ports[] = {1, 2, 3, 4, 6, 7};
    for (int i = 0; i < 6; i++) {
        lan9646_write_reg8(h, LAN9646_REG_PORT_MIRROR_CTRL(ports[i]), 0x00);
    }

    return lan9646OK;
}

/*===========================================================================*/
/*                            VLAN                                            */
/*===========================================================================*/

lan9646r_t lan9646_switch_set_port_membership(lan9646_t* h, uint8_t port,
                                              uint8_t membership) {
    uint32_t val;

    if (!h || !prv_is_valid_port(port)) {
        return lan9646INVPARAM;
    }

    lan9646_read_reg32(h, LAN9646_REG_PORT_MEMBERSHIP(port), &val);
    val = (val & ~LAN9646_VLAN_MEMBERSHIP_MASK) | (membership & LAN9646_VLAN_MEMBERSHIP_MASK);
    return lan9646_write_reg32(h, LAN9646_REG_PORT_MEMBERSHIP(port), val);
}

lan9646r_t lan9646_switch_get_port_membership(lan9646_t* h, uint8_t port,
                                              uint8_t* membership) {
    uint32_t val;
    lan9646r_t res;

    if (!h || !membership || !prv_is_valid_port(port)) {
        return lan9646INVPARAM;
    }

    res = lan9646_read_reg32(h, LAN9646_REG_PORT_MEMBERSHIP(port), &val);
    if (res != lan9646OK) return res;

    *membership = val & LAN9646_VLAN_MEMBERSHIP_MASK;
    return lan9646OK;
}

/*===========================================================================*/
/*                         LINK CALLBACK                                      */
/*===========================================================================*/

void lan9646_switch_set_link_callback(lan9646_t* h, lan9646_link_cb_t callback) {
    (void)h;
    g_link_callback = callback;
}

void lan9646_switch_poll_link(lan9646_t* h) {
    if (!h) return;

    lan9646_port_status_t status;
    uint8_t ports[] = {1, 2, 3, 4, 6, 7};

    for (int i = 0; i < 6; i++) {
        uint8_t port = ports[i];

        if (lan9646_switch_get_port_status(h, port, &status) == lan9646OK) {
            if (status.link_up != g_last_status[port].link_up ||
                status.speed != g_last_status[port].speed ||
                status.duplex != g_last_status[port].duplex) {

                g_last_status[port] = status;

                if (g_link_callback) {
                    g_link_callback(port, status.link_up, status.speed, status.duplex);
                }
            }
        }
    }
}

/*===========================================================================*/
/*                          CLOCK FUNCTIONS                                   */
/*===========================================================================*/

/* Output Clock Control Register (0x0024) */
#define LAN9646_REG_OUTPUT_CLK_CTRL     0x0024
#define LAN9646_CLK_SYNCLKO_ENABLE      (1 << 4)
#define LAN9646_CLK_FREQ_125MHZ_BIT     (1 << 3)
#define LAN9646_CLK_SRC_MASK            0x07

lan9646r_t lan9646_switch_get_clock_config(lan9646_t* h, lan9646_clk_cfg_t* cfg) {
    uint8_t reg;
    lan9646r_t res;

    if (!h || !cfg) return lan9646INVPARAM;

    res = lan9646_read_reg8(h, LAN9646_REG_OUTPUT_CLK_CTRL, &reg);
    if (res != lan9646OK) return res;

    cfg->raw_reg = reg;

    /* Decode source (bits [2:0]) */
    uint8_t src = reg & LAN9646_CLK_SRC_MASK;
    switch (src) {
        case 0: cfg->source = LAN9646_CLK_SRC_XI;       break;
        case 1: cfg->source = LAN9646_CLK_SRC_PORT1_RX; break;
        case 2: cfg->source = LAN9646_CLK_SRC_PORT2_RX; break;
        case 3: cfg->source = LAN9646_CLK_SRC_PORT3_RX; break;
        case 4: cfg->source = LAN9646_CLK_SRC_PORT4_RX; break;
        default: cfg->source = LAN9646_CLK_SRC_XI;      break;
    }

    /* Decode frequency (bit 3) */
    cfg->frequency = (reg & LAN9646_CLK_FREQ_125MHZ_BIT)
                     ? LAN9646_CLK_FREQ_125MHZ
                     : LAN9646_CLK_FREQ_25MHZ;

    /* Decode SYNCLKO enable (bit 4) */
    cfg->synclko_enable = (reg & LAN9646_CLK_SYNCLKO_ENABLE) != 0;

    return lan9646OK;
}

/*===========================================================================*/
/*                           DEBUG                                            */
/*===========================================================================*/

#ifdef LAN9646_DEBUG
#include "log_debug.h"
#define TAG "LAN9646"

void lan9646_switch_dump_regs(lan9646_t* h) {
    uint8_t val8;
    uint16_t val16;
    uint32_t val32;

    if (!h) return;

    LOG_I(TAG, "=== LAN9646 Register Dump ===");

    lan9646_read_reg8(h, 0x0001, &val8);
    LOG_I(TAG, "Chip ID MSB: 0x%02X", val8);
    lan9646_read_reg8(h, 0x0002, &val8);
    LOG_I(TAG, "Chip ID LSB: 0x%02X", val8);

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Port 6 (GMAC) ===");
    lan9646_read_reg8(h, 0x6300, &val8);
    LOG_I(TAG, "XMII Ctrl0: 0x%02X", val8);
    lan9646_read_reg8(h, 0x6301, &val8);
    LOG_I(TAG, "XMII Ctrl1: 0x%02X (TX_DLY=%d RX_DLY=%d)",
          val8, (val8 >> 3) & 1, (val8 >> 4) & 1);
    lan9646_read_reg16(h, 0x6030, &val16);
    LOG_I(TAG, "Status: 0x%04X", val16);
    lan9646_read_reg8(h, 0x6B04, &val8);
    LOG_I(TAG, "MSTP State: 0x%02X (TX=%d RX=%d)",
          val8, (val8 >> 2) & 1, (val8 >> 1) & 1);
    lan9646_read_reg32(h, 0x6A04, &val32);
    LOG_I(TAG, "Membership: 0x%08lX", (unsigned long)val32);
}

void lan9646_switch_print_status(lan9646_t* h, uint8_t port) {
    const char* speed_str[] = {"10M", "100M", "1000M", "DOWN"};
    const char* duplex_str[] = {"HD", "FD"};
    lan9646_port_status_t status;

    if (!h) return;

    if (port == 0) {
        LOG_I(TAG, "=== Port Status ===");
        for (uint8_t p = 1; p <= 4; p++) {
            if (lan9646_switch_get_port_status(h, p, &status) == lan9646OK) {
                LOG_I(TAG, "Port %d: %s %s %s", p,
                      status.link_up ? "UP  " : "DOWN",
                      speed_str[status.speed],
                      duplex_str[status.duplex]);
            }
        }
        if (lan9646_switch_get_port_status(h, 6, &status) == lan9646OK) {
            LOG_I(TAG, "Port 6 (GMAC): %s %s %s",
                  status.link_up ? "UP  " : "DOWN",
                  speed_str[status.speed],
                  duplex_str[status.duplex]);
        }
    } else if (prv_is_valid_port(port)) {
        if (lan9646_switch_get_port_status(h, port, &status) == lan9646OK) {
            LOG_I(TAG, "Port %d: %s %s %s", port,
                  status.link_up ? "UP" : "DOWN",
                  speed_str[status.speed],
                  duplex_str[status.duplex]);
        }
    }
}

void lan9646_switch_print_clock_config(lan9646_t* h) {
    lan9646_clk_cfg_t cfg;

    if (!h) return;

    if (lan9646_switch_get_clock_config(h, &cfg) != lan9646OK) {
        LOG_E(TAG, "Failed to read clock config");
        return;
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "========== Clock Configuration ==========");
    LOG_I(TAG, "Output Clock Control (0x0024): 0x%02X", cfg.raw_reg);
    LOG_I(TAG, "");

    /* Clock source */
    const char* src_str;
    switch (cfg.source) {
        case LAN9646_CLK_SRC_XI:       src_str = "XI Crystal (25MHz)"; break;
        case LAN9646_CLK_SRC_PORT1_RX: src_str = "Port 1 Recovered RX"; break;
        case LAN9646_CLK_SRC_PORT2_RX: src_str = "Port 2 Recovered RX"; break;
        case LAN9646_CLK_SRC_PORT3_RX: src_str = "Port 3 Recovered RX"; break;
        case LAN9646_CLK_SRC_PORT4_RX: src_str = "Port 4 Recovered RX"; break;
        default:                       src_str = "Unknown"; break;
    }
    LOG_I(TAG, "Clock Source:    %s", src_str);

    /* Output frequency */
    LOG_I(TAG, "Output Freq:     %s",
          (cfg.frequency == LAN9646_CLK_FREQ_125MHZ) ? "125 MHz" : "25 MHz");

    /* SYNCLKO enable */
    LOG_I(TAG, "SYNCLKO Pin:     %s", cfg.synclko_enable ? "Enabled" : "Disabled");

    LOG_I(TAG, "");
    LOG_I(TAG, "--- RGMII Clock Architecture ---");
    LOG_I(TAG, "TX_CLK6: S32K388 outputs 125MHz -> LAN9646 input");
    LOG_I(TAG, "RX_CLK6: LAN9646 outputs 125MHz -> S32K388 input");
    LOG_I(TAG, "         (derived from XI 25MHz via internal PLL)");
    LOG_I(TAG, "=========================================");
}

#else
void lan9646_switch_dump_regs(lan9646_t* h) { (void)h; }
void lan9646_switch_print_status(lan9646_t* h, uint8_t port) { (void)h; (void)port; }
void lan9646_switch_print_clock_config(lan9646_t* h) { (void)h; }
#endif

