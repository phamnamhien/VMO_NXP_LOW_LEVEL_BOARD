/**
 * \file            lan9646_traffic_test.c
 * \brief           LAN9646 Port 6 Traffic Test using MIB Counters
 *
 * Test methods:
 * 1. MIB Counter monitoring - verify counters increment with traffic
 * 2. MAC Loopback - internal loopback at MAC level
 * 3. PHY Loopback - external loopback (requires cable)
 */

#include "lan9646.h"
#include "lan9646_switch.h"
#include "log_debug.h"
#include <string.h>

#define TAG "TRAFFIC"

/*===========================================================================*/
/*                          MIB COUNTER DEFINITIONS                          */
/*===========================================================================*/

/* MIB Counter indices per datasheet Table 5-6 */
#define MIB_RX_HI_PRIO_BYTE     0x00    /* 30-bit: RX high priority bytes only */
#define MIB_RX_UNDERSIZE        0x01
#define MIB_RX_FRAGMENT         0x02
#define MIB_RX_OVERSIZE         0x03
#define MIB_RX_JABBER           0x04
#define MIB_RX_SYMBOL_ERR       0x05
#define MIB_RX_CRC_ERR          0x06
#define MIB_RX_ALIGN_ERR        0x07
#define MIB_RX_CTRL_8808        0x08
#define MIB_RX_PAUSE            0x09
#define MIB_RX_BROADCAST        0x0A
#define MIB_RX_MULTICAST        0x0B
#define MIB_RX_UNICAST          0x0C
#define MIB_RX_64_BYTE          0x0D
#define MIB_RX_65_127           0x0E
#define MIB_RX_128_255          0x0F
#define MIB_RX_256_511          0x10
#define MIB_RX_512_1023         0x11
#define MIB_RX_1024_1522        0x12
#define MIB_RX_1523_2000        0x13
#define MIB_RX_2001             0x14

#define MIB_TX_HI_PRIO_BYTE     0x15    /* 30-bit: TX high priority bytes only */
#define MIB_TX_LATE_COL         0x16
#define MIB_TX_PAUSE            0x17
#define MIB_TX_BROADCAST        0x18
#define MIB_TX_MULTICAST        0x19
#define MIB_TX_UNICAST          0x1A
#define MIB_TX_DEFERRED         0x1B
#define MIB_TX_TOTAL_COL        0x1C
#define MIB_TX_EXCESS_COL       0x1D
#define MIB_TX_SINGLE_COL       0x1E
#define MIB_TX_MULTI_COL        0x1F

/* 36-bit counters - need special handling */
#define MIB_RX_BYTE_CNT         0x80    /* 36-bit: Total RX bytes */
#define MIB_TX_BYTE_CNT         0x81    /* 36-bit: Total TX bytes */
#define MIB_RX_DROP             0x82
#define MIB_TX_DROP             0x83

/* Note: MIB_RX_TOTAL and MIB_TX_TOTAL don't exist per datasheet!
 * Use sum of unicast + multicast + broadcast instead */

/*===========================================================================*/
/*                          PRIVATE FUNCTIONS                                */
/*===========================================================================*/

/**
 * \brief           Read single MIB counter (30-bit)
 * \note            All MIB counters are READ-CLEAR!
 */
static uint32_t read_mib(lan9646_t* h, uint8_t port, uint8_t index) {
    uint16_t base = (uint16_t)port << 12;
    uint32_t ctrl, data = 0;
    uint32_t timeout = 1000;

    /* Set MIB Index [23:16] and Read Enable [25] */
    ctrl = ((uint32_t)index << 16) | 0x02000000UL;
    lan9646_write_reg32(h, base | 0x0500, ctrl);

    /* Poll until bit 25 auto-clears */
    do {
        lan9646_read_reg32(h, base | 0x0500, &ctrl);
        if (--timeout == 0) break;
    } while (ctrl & 0x02000000UL);

    /* Read counter data (32-bit) */
    lan9646_read_reg32(h, base | 0x0504, &data);
    return data;
}

/**
 * \brief           Read 36-bit MIB counter (for RxByteCnt/TxByteCnt)
 * \note            Bits [35:32] are in Control Register bits [3:0]
 */
static uint64_t read_mib_36bit(lan9646_t* h, uint8_t port, uint8_t index) {
    uint16_t base = (uint16_t)port << 12;
    uint32_t ctrl, data_lo = 0;
    uint32_t timeout = 1000;

    /* Set MIB Index [23:16] and Read Enable [25] */
    ctrl = ((uint32_t)index << 16) | 0x02000000UL;
    lan9646_write_reg32(h, base | 0x0500, ctrl);

    /* Poll until bit 25 auto-clears */
    do {
        lan9646_read_reg32(h, base | 0x0500, &ctrl);
        if (--timeout == 0) break;
    } while (ctrl & 0x02000000UL);

    /* Read counter data low 32 bits */
    lan9646_read_reg32(h, base | 0x0504, &data_lo);

    /* Extract high 4 bits [35:32] from Control Register bits [3:0] */
    uint64_t data_hi = (uint64_t)(ctrl & 0x0F) << 32;

    return data_hi | data_lo;
}

/**
 * \brief           Flush all MIB counters for a port
 * \note            Per datasheet section 5.3.6.2
 */
static void flush_mib(lan9646_t* h, uint8_t port) {
    uint16_t base = (uint16_t)port << 12;
    uint32_t ctrl;

    /* Step 1: Enable flush/freeze for this port (bit 24) */
    ctrl = 0x01000000UL;  /* MIB Flush and Freeze Enable */
    lan9646_write_reg32(h, base | 0x0500, ctrl);

    /* Step 2: Write 0xC0 to Switch MIB Control Register (0x0336)
     * to flush counters for enabled ports
     * Bit 7 = Flush, Bit 6 = Freeze (freeze while flushing) */
    lan9646_write_reg8(h, 0x0336, 0xC0);

    /* Step 3: Wait for flush to complete (bit 7 self-clears) */
    uint8_t mib_ctrl;
    uint32_t timeout = 1000;
    do {
        lan9646_read_reg8(h, 0x0336, &mib_ctrl);
        if (--timeout == 0) break;
    } while (mib_ctrl & 0x80);

    /* Step 4: Clear freeze and disable flush/freeze for this port */
    lan9646_write_reg8(h, 0x0336, 0x00);
    lan9646_write_reg32(h, base | 0x0500, 0x00);
}

/*===========================================================================*/
/*                          TRAFFIC STATISTICS                               */
/*===========================================================================*/

typedef struct {
    /* RX counters */
    uint32_t rx_unicast;
    uint32_t rx_broadcast;
    uint32_t rx_multicast;
    uint64_t rx_bytes;      /* 36-bit counter */
    uint32_t rx_crc_err;
    uint32_t rx_drop;

    /* TX counters */
    uint32_t tx_unicast;
    uint32_t tx_broadcast;
    uint32_t tx_multicast;
    uint64_t tx_bytes;      /* 36-bit counter */
    uint32_t tx_drop;
    uint32_t tx_collision;
} traffic_stats_t;

/**
 * \brief           Read all traffic statistics for a port
 * \note            Datasheet doesn't have RX_TOTAL/TX_TOTAL counters!
 *                  Total packets = unicast + broadcast + multicast
 */
static void read_traffic_stats(lan9646_t* h, uint8_t port, traffic_stats_t* stats) {
    memset(stats, 0, sizeof(traffic_stats_t));

    /* RX counters (30-bit) */
    stats->rx_unicast   = read_mib(h, port, MIB_RX_UNICAST);
    stats->rx_broadcast = read_mib(h, port, MIB_RX_BROADCAST);
    stats->rx_multicast = read_mib(h, port, MIB_RX_MULTICAST);
    stats->rx_crc_err   = read_mib(h, port, MIB_RX_CRC_ERR);
    stats->rx_drop      = read_mib(h, port, MIB_RX_DROP);

    /* RX bytes (36-bit) */
    stats->rx_bytes     = read_mib_36bit(h, port, MIB_RX_BYTE_CNT);

    /* TX counters (30-bit) */
    stats->tx_unicast   = read_mib(h, port, MIB_TX_UNICAST);
    stats->tx_broadcast = read_mib(h, port, MIB_TX_BROADCAST);
    stats->tx_multicast = read_mib(h, port, MIB_TX_MULTICAST);
    stats->tx_drop      = read_mib(h, port, MIB_TX_DROP);
    stats->tx_collision = read_mib(h, port, MIB_TX_TOTAL_COL);

    /* TX bytes (36-bit) */
    stats->tx_bytes     = read_mib_36bit(h, port, MIB_TX_BYTE_CNT);
}

/**
 * \brief           Print traffic statistics
 */
static void print_traffic_stats(const char* title, uint8_t port,
                                const traffic_stats_t* stats) {
    /* Calculate total packets = unicast + broadcast + multicast */
    uint32_t rx_total = stats->rx_unicast + stats->rx_broadcast + stats->rx_multicast;
    uint32_t tx_total = stats->tx_unicast + stats->tx_broadcast + stats->tx_multicast;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== %s (Port %d) ===", title, port);
    LOG_I(TAG, "");
    LOG_I(TAG, "RX Statistics:");
    LOG_I(TAG, "  Total Packets:  %lu (uni+bcast+mcast)", (unsigned long)rx_total);
    LOG_I(TAG, "  Unicast:        %lu", (unsigned long)stats->rx_unicast);
    LOG_I(TAG, "  Broadcast:      %lu", (unsigned long)stats->rx_broadcast);
    LOG_I(TAG, "  Multicast:      %lu", (unsigned long)stats->rx_multicast);
    LOG_I(TAG, "  Bytes (36-bit): %lu", (unsigned long)stats->rx_bytes);
    LOG_I(TAG, "  CRC Errors:     %lu", (unsigned long)stats->rx_crc_err);
    LOG_I(TAG, "  Dropped:        %lu", (unsigned long)stats->rx_drop);
    LOG_I(TAG, "");
    LOG_I(TAG, "TX Statistics:");
    LOG_I(TAG, "  Total Packets:  %lu (uni+bcast+mcast)", (unsigned long)tx_total);
    LOG_I(TAG, "  Unicast:        %lu", (unsigned long)stats->tx_unicast);
    LOG_I(TAG, "  Broadcast:      %lu", (unsigned long)stats->tx_broadcast);
    LOG_I(TAG, "  Multicast:      %lu", (unsigned long)stats->tx_multicast);
    LOG_I(TAG, "  Bytes (36-bit): %lu", (unsigned long)stats->tx_bytes);
    LOG_I(TAG, "  Dropped:        %lu", (unsigned long)stats->tx_drop);
    LOG_I(TAG, "  Collisions:     %lu", (unsigned long)stats->tx_collision);
}

/**
 * \brief           Compare two stats and print differences
 * \note            Since counters are READ-CLEAR, we just show final values
 */
static void print_stats_diff(const traffic_stats_t* before,
                             const traffic_stats_t* after) {
    /* Calculate totals */
    uint32_t rx_before = before->rx_unicast + before->rx_broadcast + before->rx_multicast;
    uint32_t rx_after = after->rx_unicast + after->rx_broadcast + after->rx_multicast;
    uint32_t tx_before = before->tx_unicast + before->tx_broadcast + before->tx_multicast;
    uint32_t tx_after = after->tx_unicast + after->tx_broadcast + after->tx_multicast;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Traffic Delta ===");
    LOG_I(TAG, "  RX Packets: %lu (was %lu)",
          (unsigned long)rx_after, (unsigned long)rx_before);
    LOG_I(TAG, "  RX Bytes:   %lu", (unsigned long)after->rx_bytes);
    LOG_I(TAG, "  TX Packets: %lu (was %lu)",
          (unsigned long)tx_after, (unsigned long)tx_before);
    LOG_I(TAG, "  TX Bytes:   %lu", (unsigned long)after->tx_bytes);

    if (after->rx_crc_err > 0) {
        LOG_W(TAG, "  CRC Errors: %lu (WARNING!)",
              (unsigned long)after->rx_crc_err);
    }
    if (after->rx_drop > 0) {
        LOG_W(TAG, "  RX Dropped: %lu (WARNING!)",
              (unsigned long)after->rx_drop);
    }
    if (after->tx_drop > 0) {
        LOG_W(TAG, "  TX Dropped: %lu (WARNING!)",
              (unsigned long)after->tx_drop);
    }
}

/*===========================================================================*/
/*                          LOOPBACK CONTROL                                 */
/*===========================================================================*/

/**
 * \brief           Enable/disable MAC loopback on Port 6
 * \note            In MAC loopback, data from switch fabric is looped back
 *                  at the MAC without going to external pins
 */
lan9646r_t lan9646_set_mac_loopback(lan9646_t* h, uint8_t port, bool enable) {
    uint16_t reg = ((uint16_t)port << 12) | 0x0020;  /* Port Operation Control 0 */
    uint8_t val;
    lan9646r_t res;

    res = lan9646_read_reg8(h, reg, &val);
    if (res != lan9646OK) return res;

    if (enable) {
        val |= 0x80;   /* Bit 7: Local MAC Loopback */
    } else {
        val &= ~0x80;
    }

    return lan9646_write_reg8(h, reg, val);
}

/**
 * \brief           Enable/disable Remote MAC loopback on Port 6
 * \note            In remote loopback, data received at external pins
 *                  is looped back out the same port
 */
lan9646r_t lan9646_set_remote_loopback(lan9646_t* h, uint8_t port, bool enable) {
    uint16_t reg = ((uint16_t)port << 12) | 0x0020;
    uint8_t val;
    lan9646r_t res;

    res = lan9646_read_reg8(h, reg, &val);
    if (res != lan9646OK) return res;

    if (enable) {
        val |= 0x40;   /* Bit 6: Remote MAC Loopback */
    } else {
        val &= ~0x40;
    }

    return lan9646_write_reg8(h, reg, val);
}

/*===========================================================================*/
/*                          TEST FUNCTIONS                                   */
/*===========================================================================*/

/**
 * \brief           Test 1: Monitor traffic counters over time
 * \param[in]       h: Device handle
 * \param[in]       port: Port to monitor (typically 6)
 * \param[in]       duration_ms: How long to monitor
 * \param[in]       delay_fn: Delay function (ms)
 */
void lan9646_traffic_test_monitor(lan9646_t* h, uint8_t port,
                                   uint32_t duration_ms,
                                   void (*delay_fn)(uint32_t)) {
    traffic_stats_t stats_start, stats_end;

    LOG_I(TAG, "");
    LOG_I(TAG, "########################################################");
    LOG_I(TAG, "#         TRAFFIC MONITOR TEST - Port %d                #", port);
    LOG_I(TAG, "########################################################");

    /* Clear counters */
    LOG_I(TAG, "Flushing MIB counters...");
    flush_mib(h, port);

    /* Read initial stats */
    read_traffic_stats(h, port, &stats_start);
    print_traffic_stats("Initial Statistics", port, &stats_start);

    /* Wait for traffic */
    LOG_I(TAG, "");
    LOG_I(TAG, "Monitoring for %lu ms...", (unsigned long)duration_ms);
    LOG_I(TAG, "(Send traffic to/from Port %d now)", port);

    if (delay_fn) {
        delay_fn(duration_ms);
    }

    /* Read final stats */
    read_traffic_stats(h, port, &stats_end);
    print_traffic_stats("Final Statistics", port, &stats_end);

    /* Print difference */
    print_stats_diff(&stats_start, &stats_end);

    /* Verdict - calculate totals locally */
    uint32_t rx_end = stats_end.rx_unicast + stats_end.rx_broadcast + stats_end.rx_multicast;
    uint32_t tx_end = stats_end.tx_unicast + stats_end.tx_broadcast + stats_end.tx_multicast;

    LOG_I(TAG, "");
    if (rx_end > 0 || tx_end > 0 || stats_end.rx_bytes > 0 || stats_end.tx_bytes > 0) {
        LOG_I(TAG, "RESULT: TRAFFIC DETECTED!");

        if (stats_end.rx_crc_err > 0) {
            LOG_W(TAG, "WARNING: CRC errors detected - check RGMII timing!");
        }
    } else {
        LOG_W(TAG, "RESULT: NO TRAFFIC DETECTED");
    }
}

/**
 * \brief           Test 2: MAC Loopback test
 * \note            Requires GMAC to send packets. They will loop back
 *                  in the switch and increment RX counters.
 */
void lan9646_traffic_test_mac_loopback(lan9646_t* h, uint8_t port,
                                        void (*delay_fn)(uint32_t)) {
    traffic_stats_t stats_before, stats_after;

    LOG_I(TAG, "");
    LOG_I(TAG, "########################################################");
    LOG_I(TAG, "#         MAC LOOPBACK TEST - Port %d                   #", port);
    LOG_I(TAG, "########################################################");
    LOG_I(TAG, "");
    LOG_I(TAG, "This test enables MAC loopback so packets sent from GMAC");
    LOG_I(TAG, "will be looped back without going to external pins.");

    /* Clear and read initial */
    flush_mib(h, port);
    read_traffic_stats(h, port, &stats_before);

    /* Enable loopback */
    LOG_I(TAG, "");
    LOG_I(TAG, "Enabling MAC loopback...");
    if (lan9646_set_mac_loopback(h, port, true) != lan9646OK) {
        LOG_E(TAG, "Failed to enable loopback!");
        return;
    }

    /* Verify loopback enabled */
    uint8_t ctrl;
    lan9646_read_reg8(h, ((uint16_t)port << 12) | 0x0020, &ctrl);
    LOG_I(TAG, "Port Operation Control 0: 0x%02X (bit7=%d)", ctrl, (ctrl >> 7) & 1);

    LOG_I(TAG, "");
    LOG_I(TAG, "Loopback ENABLED. Send packets from GMAC now!");
    LOG_I(TAG, "Waiting 5 seconds...");

    if (delay_fn) {
        delay_fn(5000);
    }

    /* Read stats */
    read_traffic_stats(h, port, &stats_after);

    /* Disable loopback */
    LOG_I(TAG, "Disabling MAC loopback...");
    lan9646_set_mac_loopback(h, port, false);

    /* Results */
    print_stats_diff(&stats_before, &stats_after);

    /* Calculate totals */
    uint32_t rx_after = stats_after.rx_unicast + stats_after.rx_broadcast + stats_after.rx_multicast;
    uint32_t tx_after = stats_after.tx_unicast + stats_after.tx_broadcast + stats_after.tx_multicast;

    LOG_I(TAG, "");
    if (tx_after > 0 || stats_after.tx_bytes > 0) {
        if (rx_after > 0 || stats_after.rx_bytes > 0) {
            LOG_I(TAG, "RESULT: LOOPBACK WORKING!");
            LOG_I(TAG, "  TX packets sent and RX packets received back.");
        } else {
            LOG_W(TAG, "RESULT: TX OK but RX NOT incrementing");
            LOG_W(TAG, "  Check: Is GMAC sending? Is RX enabled?");
        }
    } else {
        LOG_W(TAG, "RESULT: NO TX PACKETS");
        LOG_W(TAG, "  GMAC is not sending packets to Port 6.");
    }
}

/**
 * \brief           Test 3: Remote loopback (external cable loopback)
 * \note            Connect Port 6 TX to Port 6 RX externally
 *                  or use another device to echo packets
 */
void lan9646_traffic_test_remote_loopback(lan9646_t* h, uint8_t port,
                                           void (*delay_fn)(uint32_t)) {
    traffic_stats_t stats_before, stats_after;

    LOG_I(TAG, "");
    LOG_I(TAG, "########################################################");
    LOG_I(TAG, "#       REMOTE LOOPBACK TEST - Port %d                  #", port);
    LOG_I(TAG, "########################################################");
    LOG_I(TAG, "");
    LOG_I(TAG, "This test enables remote loopback: data received at");
    LOG_I(TAG, "external pins is looped back out the same port.");
    LOG_I(TAG, "");
    LOG_I(TAG, "Connect another device that sends packets to Port 6.");

    flush_mib(h, port);
    read_traffic_stats(h, port, &stats_before);

    LOG_I(TAG, "");
    LOG_I(TAG, "Enabling Remote loopback...");
    lan9646_set_remote_loopback(h, port, true);

    uint8_t ctrl;
    lan9646_read_reg8(h, ((uint16_t)port << 12) | 0x0020, &ctrl);
    LOG_I(TAG, "Port Operation Control 0: 0x%02X (bit6=%d)", ctrl, (ctrl >> 6) & 1);

    LOG_I(TAG, "");
    LOG_I(TAG, "Remote loopback ENABLED.");
    LOG_I(TAG, "External device should receive echoed packets.");
    LOG_I(TAG, "Waiting 10 seconds...");

    if (delay_fn) {
        delay_fn(10000);
    }

    read_traffic_stats(h, port, &stats_after);
    lan9646_set_remote_loopback(h, port, false);

    print_stats_diff(&stats_before, &stats_after);

    /* Calculate total */
    uint32_t rx_after = stats_after.rx_unicast + stats_after.rx_broadcast + stats_after.rx_multicast;

    LOG_I(TAG, "");
    if (rx_after > 0 || stats_after.rx_bytes > 0) {
        LOG_I(TAG, "RESULT: PACKETS RECEIVED AND ECHOED!");
    } else {
        LOG_W(TAG, "RESULT: NO PACKETS RECEIVED");
    }
}

/**
 * \brief           Test 4: Check for errors
 */
void lan9646_traffic_test_errors(lan9646_t* h, uint8_t port) {
    LOG_I(TAG, "");
    LOG_I(TAG, "########################################################");
    LOG_I(TAG, "#         ERROR CHECK - Port %d                         #", port);
    LOG_I(TAG, "########################################################");

    uint32_t crc_err    = read_mib(h, port, MIB_RX_CRC_ERR);
    uint32_t align_err  = read_mib(h, port, MIB_RX_ALIGN_ERR);
    uint32_t symbol_err = read_mib(h, port, MIB_RX_SYMBOL_ERR);
    uint32_t undersize  = read_mib(h, port, MIB_RX_UNDERSIZE);
    uint32_t oversize   = read_mib(h, port, MIB_RX_OVERSIZE);
    uint32_t fragment   = read_mib(h, port, MIB_RX_FRAGMENT);
    uint32_t jabber     = read_mib(h, port, MIB_RX_JABBER);
    uint32_t rx_drop    = read_mib(h, port, MIB_RX_DROP);
    uint32_t tx_drop    = read_mib(h, port, MIB_TX_DROP);
    uint32_t late_col   = read_mib(h, port, MIB_TX_LATE_COL);
    uint32_t excess_col = read_mib(h, port, MIB_TX_EXCESS_COL);

    LOG_I(TAG, "");
    LOG_I(TAG, "RX Errors:");
    LOG_I(TAG, "  CRC Errors:       %lu %s", (unsigned long)crc_err,
          crc_err ? "<-- CHECK RGMII TIMING!" : "");
    LOG_I(TAG, "  Alignment Errors: %lu", (unsigned long)align_err);
    LOG_I(TAG, "  Symbol Errors:    %lu", (unsigned long)symbol_err);
    LOG_I(TAG, "  Undersize:        %lu", (unsigned long)undersize);
    LOG_I(TAG, "  Oversize:         %lu", (unsigned long)oversize);
    LOG_I(TAG, "  Fragments:        %lu", (unsigned long)fragment);
    LOG_I(TAG, "  Jabber:           %lu", (unsigned long)jabber);
    LOG_I(TAG, "  RX Dropped:       %lu", (unsigned long)rx_drop);

    LOG_I(TAG, "");
    LOG_I(TAG, "TX Errors:");
    LOG_I(TAG, "  TX Dropped:       %lu", (unsigned long)tx_drop);
    LOG_I(TAG, "  Late Collisions:  %lu %s", (unsigned long)late_col,
          late_col ? "<-- DUPLEX MISMATCH?" : "");
    LOG_I(TAG, "  Excess Collisions:%lu", (unsigned long)excess_col);

    LOG_I(TAG, "");
    uint32_t total_err = crc_err + align_err + symbol_err + rx_drop +
                         tx_drop + late_col + excess_col;
    if (total_err == 0) {
        LOG_I(TAG, "RESULT: NO ERRORS DETECTED");
    } else {
        LOG_W(TAG, "RESULT: %lu TOTAL ERRORS", (unsigned long)total_err);
        if (crc_err > 0) {
            LOG_W(TAG, "TIP: CRC errors usually mean RGMII timing issue.");
            LOG_W(TAG, "     Try adjusting TX/RX delay on LAN9646 or S32K GMAC.");
        }
    }
}

/**
 * \brief           Run all traffic tests
 */
void lan9646_traffic_test_all(lan9646_t* h, void (*delay_fn)(uint32_t)) {
    LOG_I(TAG, "");
    LOG_I(TAG, "########################################################");
    LOG_I(TAG, "#                                                      #");
    LOG_I(TAG, "#         LAN9646 PORT 6 TRAFFIC TEST SUITE            #");
    LOG_I(TAG, "#                                                      #");
    LOG_I(TAG, "########################################################");

    /* Test 1: Check current errors */
    lan9646_traffic_test_errors(h, 6);

    /* Test 2: Monitor traffic for 10 seconds */
    lan9646_traffic_test_monitor(h, 6, 10000, delay_fn);

    /* Test 3: MAC loopback */
    lan9646_traffic_test_mac_loopback(h, 6, delay_fn);

    /* Test 4: Final error check */
    lan9646_traffic_test_errors(h, 6);

    LOG_I(TAG, "");
    LOG_I(TAG, "########################################################");
    LOG_I(TAG, "#              TRAFFIC TESTS COMPLETE                  #");
    LOG_I(TAG, "########################################################");
}

