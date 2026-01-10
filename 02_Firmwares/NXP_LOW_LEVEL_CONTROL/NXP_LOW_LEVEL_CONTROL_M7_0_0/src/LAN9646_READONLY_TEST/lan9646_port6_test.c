/**
 * \file            lan9646_port6_test.c
 * \brief           Test Port 6 (CPU port) connectivity với GMAC
 */

#include "s32k3xx_soft_i2c.h"
#include "lan9646.h"
#include "log_debug.h"
#include "systick.h"

#define TAG "LAN9646_P6TEST"

extern lan9646_t g_lan;

/**
 * \brief           Read MIB counter - INDIRECT ACCESS
 */
static uint32_t lan9646_read_mib_counter(uint16_t port_base, uint8_t mib_index) {
    uint32_t ctrl, data;

    /* 1. Write MIB Index to bits [23:16] + Set Read Enable bit 25 */
    ctrl = ((uint32_t)mib_index << 16) | (1UL << 25);
    lan9646_write_reg32(&g_lan, port_base + 0x500, ctrl);

    /* 2. Poll bit 25 until clear (read complete) */
    uint32_t timeout = 10000;
    do {
        lan9646_read_reg32(&g_lan, port_base + 0x500, &ctrl);
        if (--timeout == 0) break;
    } while (ctrl & (1UL << 25));

    /* 3. Read count value from Port MIB Data Register */
    lan9646_read_reg32(&g_lan, port_base + 0x504, &data);

    return data;
}

/**
 * \brief           Đọc MIB counters Port 6
 */
void lan9646_port6_read_mib(void) {
    uint32_t rx_unicast, tx_unicast;
    uint32_t rx_broadcast, tx_broadcast;
    uint32_t rx_multicast, tx_multicast;
    uint32_t rx_bytes, tx_bytes;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Port 6 MIB Counters (Indirect Read) ===");

    /* Read counters using indirect access */
    rx_unicast = lan9646_read_mib_counter(0x6000, 0x0C);     /* RxUnicast */
    tx_unicast = lan9646_read_mib_counter(0x6000, 0x1A);     /* TxUnicast */
    rx_broadcast = lan9646_read_mib_counter(0x6000, 0x0A);   /* RxBroadcast */
    tx_broadcast = lan9646_read_mib_counter(0x6000, 0x18);   /* TxBroadcast */
    rx_multicast = lan9646_read_mib_counter(0x6000, 0x0B);   /* RxMulticast */
    tx_multicast = lan9646_read_mib_counter(0x6000, 0x19);   /* TxMulticast */
    rx_bytes = lan9646_read_mib_counter(0x6000, 0x80);       /* RxByteCnt */
    tx_bytes = lan9646_read_mib_counter(0x6000, 0x81);       /* TxByteCnt */

    LOG_I(TAG, "RX Unicast: %lu", rx_unicast);
    LOG_I(TAG, "RX Broadcast: %lu", rx_broadcast);
    LOG_I(TAG, "RX Multicast: %lu", rx_multicast);
    LOG_I(TAG, "RX Bytes: %lu", rx_bytes);
    LOG_I(TAG, "");
    LOG_I(TAG, "TX Unicast: %lu", tx_unicast);
    LOG_I(TAG, "TX Broadcast: %lu", tx_broadcast);
    LOG_I(TAG, "TX Multicast: %lu", tx_multicast);
    LOG_I(TAG, "TX Bytes: %lu", tx_bytes);

    uint32_t total_rx = rx_unicast + rx_broadcast + rx_multicast;
    uint32_t total_tx = tx_unicast + tx_broadcast + tx_multicast;

    LOG_I(TAG, "");
    if (total_rx > 0 || total_tx > 0) {
        LOG_I(TAG, "✓ Port 6 HAS TRAFFIC! (RX:%lu TX:%lu)", total_rx, total_tx);
    } else {
        LOG_I(TAG, "✗ Port 6 NO TRAFFIC");
    }
}

/**
 * \brief           Enable switch globally
 */
void lan9646_port6_enable_switch(void) {
    uint8_t data8;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Enable Switch Globally ===");

    /* Check Switch Operation Register (0x0300) */
    lan9646_read_reg8(&g_lan, 0x0300, &data8);
    LOG_I(TAG, "Switch Operation (0x0300): 0x%02X", data8);
    LOG_I(TAG, "  Start Switch (bit 0): %s", (data8 & 0x01) ? "YES" : "NO");

    if (!(data8 & 0x01)) {
        LOG_I(TAG, "Switch was DISABLED - enabling now");
        data8 |= 0x01;
        lan9646_write_reg8(&g_lan, 0x0300, data8);

        lan9646_read_reg8(&g_lan, 0x0300, &data8);
        LOG_I(TAG, "After: 0x%02X %s", data8,
              (data8 & 0x01) ? "SUCCESS" : "FAILED");
    }

    /* Check Switch Lookup Engine Control 0 (0x0310) */
    lan9646_read_reg8(&g_lan, 0x0310, &data8);
    LOG_I(TAG, "Lookup Engine (0x0310): 0x%02X", data8);
    LOG_I(TAG, "  VLAN Mode: %s", (data8 & 0x80) ? "EN" : "DIS");
    LOG_I(TAG, "  Address Learning: %s", (data8 & 0x40) ? "EN" : "DIS");
}

/**
 * \brief           Check và enable switch operation
 */
void lan9646_enable_switch_operation(void) {
    uint8_t data8;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Switch Operation ===");

    /* Switch Operation Register (0x0300) */
    lan9646_read_reg8(&g_lan, 0x0300, &data8);
    LOG_I(TAG, "Switch Operation (0x0300): 0x%02X", data8);
    LOG_I(TAG, "  Start Switch: %s", (data8 & 0x01) ? "YES" : "NO");

    /* Enable Start Switch (bit 0) */
    if (!(data8 & 0x01)) {
        data8 |= 0x01;
        lan9646_write_reg8(&g_lan, 0x0300, data8);
        LOG_I(TAG, "Start Switch ENABLED");
    }

    /* Verify */
    lan9646_read_reg8(&g_lan, 0x0300, &data8);
    LOG_I(TAG, "After: Switch Operation: 0x%02X", data8);
}

/**
 * \brief           Enable Port Mirroring - Mirror Port 1/2 → Port 6
 *                  This makes Port 6 receive COPIES of all packets
 */
void lan9646_enable_port_mirroring_to_port6(void) {
    uint8_t data8;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Enable Port Mirroring ===");
    LOG_I(TAG, "Mirror Port 1/2 traffic → Port 6 (CPU)");

    /* Global Port Mirroring Control (0x0370) - MUST ENABLE! */
    lan9646_read_reg8(&g_lan, 0x0370, &data8);
    LOG_I(TAG, "Global Mirror Control before: 0x%02X", data8);
    /* Bit 4:3 = Sniffer port select (Port 6 = 5 in binary = 101, but bits 4:3 only)
     * Need to check datasheet for exact bit mapping */

    /* Port 6: Set as Sniffer Port (bit 1) */
    lan9646_read_reg8(&g_lan, 0x6800, &data8);
    LOG_I(TAG, "Port 6 Mirror Control before: 0x%02X", data8);
    data8 |= 0x02;  /* Bit 1 = Sniffer Port */
    lan9646_write_reg8(&g_lan, 0x6800, data8);
    lan9646_read_reg8(&g_lan, 0x6800, &data8);
    LOG_I(TAG, "Port 6 Mirror Control after: 0x%02X (Sniffer Port)", data8);

    /* Port 1: Enable Receive Sniff (bit 6) */
    lan9646_read_reg8(&g_lan, 0x1800, &data8);
    LOG_I(TAG, "Port 1 Mirror Control before: 0x%02X", data8);
    data8 |= 0x40;  /* Bit 6 = Receive Sniff */
    lan9646_write_reg8(&g_lan, 0x1800, data8);
    lan9646_read_reg8(&g_lan, 0x1800, &data8);
    LOG_I(TAG, "Port 1 Mirror Control after: 0x%02X (RX Sniff)", data8);

    /* Port 2: Enable Receive Sniff (bit 6) */
    lan9646_read_reg8(&g_lan, 0x2800, &data8);
    LOG_I(TAG, "Port 2 Mirror Control before: 0x%02X", data8);
    data8 |= 0x40;  /* Bit 6 = Receive Sniff */
    lan9646_write_reg8(&g_lan, 0x2800, data8);
    lan9646_read_reg8(&g_lan, 0x2800, &data8);
    LOG_I(TAG, "Port 2 Mirror Control after: 0x%02X (RX Sniff)", data8);

    LOG_I(TAG, "");
    LOG_I(TAG, "✓ Port Mirroring ENABLED!");
    LOG_I(TAG, "  All packets received on Port 1/2 → copied to Port 6 TX");
    LOG_I(TAG, "");
    LOG_I(TAG, "NOTE: Mirrored packets appear on Port 6 TX (not RX!)");
    LOG_I(TAG, "  Port 6 RX = GMAC → Switch");
    LOG_I(TAG, "  Port 6 TX = Switch → GMAC (includes mirrored packets)");
}

/**
 * \brief           Check VLAN configuration
 */
void lan9646_check_vlan_config(void) {
    uint8_t data8;
    uint32_t data32;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== VLAN Configuration ===");

    /* Switch Lookup Engine Control 0 */
    lan9646_read_reg8(&g_lan, 0x0310, &data8);
    LOG_I(TAG, "LUE Control 0 (0x0310): 0x%02X", data8);
    LOG_I(TAG, "  802.1Q VLAN Enable: %s", (data8 & 0x80) ? "YES" : "NO");

    /* VLAN Table Entry for VID=1 (0x0400-0x0407) */
    /* Write VID=1 to VLAN Table Index */
    uint16_t vid = 1;
    lan9646_write_reg16(&g_lan, 0x047E, vid);

    /* Trigger read: Set bit 12 in VLAN Table Access Control */
    lan9646_write_reg8(&g_lan, 0x047D, 0x10);

    /* Wait */
    SysTick_DelayMs(1);

    /* Read VLAN Table Entry 0 (0x0400-0x0403) */
    lan9646_read_reg32(&g_lan, 0x0400, &data32);
    LOG_I(TAG, "VLAN Table Entry VID=1 (0x0400): 0x%08lX", data32);
    LOG_I(TAG, "  Valid: %s", (data32 & 0x00001000) ? "YES" : "NO");

    /* Bits 6:0 = Port membership */
    uint8_t membership = data32 & 0x7F;
    LOG_I(TAG, "  Port Membership: 0x%02X", membership);
    LOG_I(TAG, "    Port 1: %s", (membership & 0x01) ? "YES" : "NO");
    LOG_I(TAG, "    Port 2: %s", (membership & 0x02) ? "YES" : "NO");
    LOG_I(TAG, "    Port 6: %s", (membership & 0x20) ? "YES" : "NO");
}

/**
 * \brief           Enable unknown unicast forwarding to Port 6
 */
void lan9646_enable_unknown_unicast_to_port6(void) {
    uint8_t data8;

    LOG_I(TAG, "");
    LOG_I(TAG, "Enable Unknown Unicast forwarding to Port 6...");

    /* Global: Unknown Unicast Port Mapping (0x0314) */
    lan9646_read_reg8(&g_lan, 0x0314, &data8);
    LOG_I(TAG, "Before: Unknown Unicast Mapping: 0x%02X", data8);

    /* Bit 5 = Port 6 */
    data8 |= 0x20;
    lan9646_write_reg8(&g_lan, 0x0314, data8);

    lan9646_read_reg8(&g_lan, 0x0314, &data8);
    LOG_I(TAG, "After: Unknown Unicast Mapping: 0x%02X", data8);

    /* Unknown Multicast Port Mapping (0x0315) */
    lan9646_read_reg8(&g_lan, 0x0315, &data8);
    data8 |= 0x20;
    lan9646_write_reg8(&g_lan, 0x0315, data8);
    LOG_I(TAG, "Unknown Multicast Mapping: 0x%02X", data8);
}

/**
 * \brief           Setup VLAN Table để add TẤT CẢ ports vào VID=1
 */
void lan9646_setup_vlan_table_port6(void) {
    uint32_t data32;

    LOG_I(TAG, "");
    LOG_I(TAG, "Setup VLAN Table - Add ALL ports to VID=1...");

    /* Write VID=1 */
    lan9646_write_reg16(&g_lan, 0x047E, 1);

    /* Trigger read */
    lan9646_write_reg8(&g_lan, 0x047D, 0x10);
    SysTick_DelayMs(1);

    /* Read current entry */
    lan9646_read_reg32(&g_lan, 0x0400, &data32);
    LOG_I(TAG, "Current VLAN Entry: 0x%08lX", data32);

    /* Set ALL ports + Valid
     * Bit 0 = Port 1
     * Bit 1 = Port 2
     * Bit 2 = Port 3
     * Bit 3 = Port 4
     * Bit 5 = Port 6
     * Bit 6 = Port 7
     * Bit 12 = Valid
     */
    data32 = 0x0000106F;  /* 0x6F = 01101111 = P7,P6,P4,P3,P2,P1 */

    /* Write VLAN Table Entry 0 (Port Membership) */
    lan9646_write_reg32(&g_lan, 0x0400, data32);

    /* Write VLAN Table Entry 1 (FID = 0) */
    lan9646_write_reg32(&g_lan, 0x0404, 0x00000000);

    /* Trigger write: Set bit 13 */
    lan9646_write_reg8(&g_lan, 0x047D, 0x20);
    SysTick_DelayMs(1);

    LOG_I(TAG, "VLAN Table updated: 0x%08lX", data32);

    /* Verify */
    lan9646_write_reg8(&g_lan, 0x047D, 0x10);
    SysTick_DelayMs(1);
    lan9646_read_reg32(&g_lan, 0x0400, &data32);
    LOG_I(TAG, "Verify VLAN Entry: 0x%08lX", data32);
    LOG_I(TAG, "  Port 1: %s", (data32 & 0x01) ? "YES" : "NO");
    LOG_I(TAG, "  Port 2: %s", (data32 & 0x02) ? "YES" : "NO");
    LOG_I(TAG, "  Port 6: %s", (data32 & 0x20) ? "YES" : "NO");
    LOG_I(TAG, "  Valid: %s", (data32 & 0x1000) ? "YES" : "NO");
}

/**
 * \brief           Enable forwarding Port 1/2 ↔ Port 6
 */
void lan9646_port6_enable_forwarding(void) {
    uint32_t data32;

    LOG_I(TAG, "");
    LOG_I(TAG, "Enable Port 1/2 ↔ Port 6 forwarding...");

    /* Port 1: Allow forward to Port 6 (Port Control 1: 0x1A04-0x1A07)
     * Bit 5 = Port 6
     * Default 0x7F = all ports */
    lan9646_read_reg32(&g_lan, 0x1A04, &data32);
    LOG_I(TAG, "Port 1 VLAN Membership before: 0x%08lX", data32);
    data32 |= (1 << 5);  /* Bit 5 = Port 6 */
    lan9646_write_reg32(&g_lan, 0x1A04, data32);
    LOG_I(TAG, "Port 1 → Port 6: Enabled");

    /* Port 2: Allow forward to Port 6 */
    lan9646_read_reg32(&g_lan, 0x2A04, &data32);
    LOG_I(TAG, "Port 2 VLAN Membership before: 0x%08lX", data32);
    data32 |= (1 << 5);
    lan9646_write_reg32(&g_lan, 0x2A04, data32);
    LOG_I(TAG, "Port 2 → Port 6: Enabled");

    /* Port 6: Allow forward to Port 1/2 */
    lan9646_read_reg32(&g_lan, 0x6A04, &data32);
    LOG_I(TAG, "Port 6 VLAN Membership before: 0x%08lX", data32);
    data32 |= (1 << 0) | (1 << 1);  /* Bit 0 = Port 1, Bit 1 = Port 2 */
    lan9646_write_reg32(&g_lan, 0x6A04, data32);
    LOG_I(TAG, "Port 6 → Port 1/2: Enabled");

    LOG_I(TAG, "");
    LOG_I(TAG, "Verify forwarding:");
    lan9646_read_reg32(&g_lan, 0x1A04, &data32);
    LOG_I(TAG, "Port 1 membership: 0x%08lX", data32);
    lan9646_read_reg32(&g_lan, 0x2A04, &data32);
    LOG_I(TAG, "Port 2 membership: 0x%08lX", data32);
    lan9646_read_reg32(&g_lan, 0x6A04, &data32);
    LOG_I(TAG, "Port 6 membership: 0x%08lX", data32);
}

/**
 * \brief           Test Port 6 loopback
 */
void lan9646_port6_test_loopback(void) {
    uint8_t data8;
    uint32_t rx_before, tx_before, rx_after, tx_after;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Port 6 Loopback Test ===");

    /* Read counters before */
    lan9646_read_reg32(&g_lan, 0x6500, &rx_before);
    lan9646_read_reg32(&g_lan, 0x6504, &tx_before);
    LOG_I(TAG, "Before: RX=%lu TX=%lu", rx_before, tx_before);

    /* Enable local loopback (bit 7 of Port Operation Control 0) */
    lan9646_read_reg8(&g_lan, 0x6020, &data8);
    data8 |= 0x80;
    lan9646_write_reg8(&g_lan, 0x6020, data8);
    LOG_I(TAG, "Loopback enabled");

    /* Wait for GMAC to send packets */
    LOG_I(TAG, "Send packets from GMAC now...");
    LOG_I(TAG, "Waiting 10 seconds...");
    SysTick_DelayMs(10000);

    /* Read counters after */
    lan9646_read_reg32(&g_lan, 0x6500, &rx_after);
    lan9646_read_reg32(&g_lan, 0x6504, &tx_after);
    LOG_I(TAG, "After: RX=%lu TX=%lu", rx_after, tx_after);

    /* Disable loopback */
    data8 &= ~0x80;
    lan9646_write_reg8(&g_lan, 0x6020, data8);

    /* Results */
    LOG_I(TAG, "");
    if (rx_after > rx_before || tx_after > tx_before) {
        LOG_I(TAG, "✓ LOOPBACK WORKING!");
        LOG_I(TAG, "  RX: +%lu packets", rx_after - rx_before);
        LOG_I(TAG, "  TX: +%lu packets", tx_after - tx_before);
    } else {
        LOG_I(TAG, "✗ NO TRAFFIC DETECTED");
        LOG_I(TAG, "  Check GMAC driver is sending packets");
    }
}

/**
 * \brief           Monitor Port 6 traffic (chạy trong loop)
 */
void lan9646_port6_monitor_traffic(void) {
    static uint32_t last_rx = 0, last_tx = 0;
    uint32_t rx, tx;

    lan9646_read_reg32(&g_lan, 0x6500, &rx);
    lan9646_read_reg32(&g_lan, 0x6504, &tx);

    if (rx != last_rx || tx != last_tx) {
        LOG_I(TAG, "Port 6 Traffic: RX=%lu TX=%lu (Δ RX=%lu TX=%lu)",
              rx, tx, rx - last_rx, tx - last_tx);
        last_rx = rx;
        last_tx = tx;
    }
}

/**
 * \brief           Dump Port 6 MAC registers
 */
void lan9646_port6_dump_mac(void) {
    uint8_t data8, mstp_state;
    uint16_t data16;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Port 6 Configuration ===");

    /* MSTP State - TX/RX control */
    lan9646_write_reg8(&g_lan, 0x6B03, 0x00);
    lan9646_read_reg8(&g_lan, 0x6B04, &mstp_state);
    LOG_I(TAG, "MSTP State (0x6B04): 0x%02X", mstp_state);
    LOG_I(TAG, "  TX Enable: %s", (mstp_state & 0x04) ? "YES" : "NO");
    LOG_I(TAG, "  RX Enable: %s", (mstp_state & 0x02) ? "YES" : "NO");

    /* Interface Mode (0x6300) */
    lan9646_read_reg8(&g_lan, 0x6300, &data8);
    LOG_I(TAG, "Interface Mode (0x6300): 0x%02X", data8);
    const char* mode[] = {"RGMII", "RMII", "Reserved", "Reserved"};
    LOG_I(TAG, "  Mode: %s", mode[data8 & 0x03]);

    /* RGMII Control (0x6301) */
    lan9646_read_reg8(&g_lan, 0x6301, &data8);
    LOG_I(TAG, "RGMII Control (0x6301): 0x%02X", data8);
    LOG_I(TAG, "  TX Delay: %s", (data8 & 0x01) ? "EN" : "DIS");
    LOG_I(TAG, "  RX Delay: %s", (data8 & 0x02) ? "EN" : "DIS");

    /* Port Status (0x6030) */
    lan9646_read_reg16(&g_lan, 0x6030, &data16);
    LOG_I(TAG, "Port Status (0x6030): 0x%04X", data16);
    LOG_I(TAG, "  Speed: %s", (data16 & 0x0400) ? "1000M" :
                              (data16 & 0x0200) ? "100M" : "10M");
    LOG_I(TAG, "  Duplex: %s", (data16 & 0x0100) ? "Full" : "Half");

    /* VLAN Membership */
    uint32_t membership;
    lan9646_read_reg32(&g_lan, 0x6A04, &membership);
    LOG_I(TAG, "VLAN Membership (0x6A04): 0x%08lX", membership);
    LOG_I(TAG, "  Can forward to:");
    if (membership & 0x01) LOG_I(TAG, "    Port 1");
    if (membership & 0x02) LOG_I(TAG, "    Port 2");
    if (membership & 0x04) LOG_I(TAG, "    Port 3");
    if (membership & 0x08) LOG_I(TAG, "    Port 4");
    if (membership & 0x40) LOG_I(TAG, "    Port 7");
}

/**
 * \brief           Debug tại sao không có traffic
 */
void lan9646_port6_debug_no_traffic(void) {
    uint8_t data8;
    uint16_t data16;
    uint32_t data32;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Debug: Why No Traffic? ===");

    /* 1. Switch enabled? */
    lan9646_read_reg8(&g_lan, 0x0300, &data8);
    if (!(data8 & 0x01)) {
        LOG_E(TAG, "✗ SWITCH DISABLED! (0x0300 bit0 = 0)");
        return;
    }
    LOG_I(TAG, "✓ Switch enabled");

    /* 2. Port 1 link? */
    lan9646_read_reg16(&g_lan, 0x1102, &data16);
    if (!(data16 & 0x0004)) {
        LOG_E(TAG, "✗ PORT 1 LINK DOWN!");
        return;
    }
    LOG_I(TAG, "✓ Port 1 link UP");

    /* 3. Port 6 MSTP? */
    lan9646_write_reg8(&g_lan, 0x6B03, 0x00);
    lan9646_read_reg8(&g_lan, 0x6B04, &data8);
    if ((data8 & 0x06) != 0x06) {
        LOG_E(TAG, "✗ PORT 6 TX/RX DISABLED! (0x6B04 = 0x%02X)", data8);
        return;
    }
    LOG_I(TAG, "✓ Port 6 TX/RX enabled");

    /* 4. Port 1 → Port 6 forwarding? */
    lan9646_read_reg32(&g_lan, 0x1A04, &data32);
    if (!(data32 & (1 << 5))) {
        LOG_E(TAG, "✗ PORT 1 CANNOT FORWARD TO PORT 6! (0x1A04 bit5 = 0)");
        return;
    }
    LOG_I(TAG, "✓ Port 1 → Port 6 forwarding enabled");

    /* 5. Broadcast storm protection? */
    lan9646_read_reg8(&g_lan, 0x1400, &data8);
    LOG_I(TAG, "Port 1 MAC Control (0x1400): 0x%02X", data8);
    LOG_I(TAG, "  Broadcast Storm: %s", (data8 & 0x02) ? "BLOCKED" : "PASS");

    /* 6. Learning disabled? */
    lan9646_read_reg8(&g_lan, 0x0310, &data8);
    LOG_I(TAG, "Lookup Engine (0x0310): 0x%02X", data8);
    if (!(data8 & 0x40)) {
        LOG_W(TAG, "! Address Learning DISABLED");
    } else {
        LOG_I(TAG, "✓ Address Learning enabled");
    }

    /* 7. Check actual Port 1 RX counter */
    uint32_t port1_rx;
    lan9646_read_reg32(&g_lan, 0x1500, &port1_rx);
    LOG_I(TAG, "");
    LOG_I(TAG, "Port 1 RX packets: %lu", port1_rx);
    if (port1_rx == 0) {
        LOG_E(TAG, "✗ PORT 1 NOT RECEIVING ANYTHING!");
        LOG_E(TAG, "  → Check cable connection to PC");
    } else {
        LOG_I(TAG, "✓ Port 1 is receiving packets");
        LOG_E(TAG, "  → But Port 6 NOT receiving → FORWARDING ISSUE!");
    }
}

/**
 * \brief           Test PC → Port1 → Port6 TX → GMAC
 *                  NOTE: Mirrored packets appear on Port 6 TX (not RX!)
 */
void lan9646_port6_test_rx_from_port1(void) {
    uint32_t p1_rx, p6_tx;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Test PC → Port1 → Port6 TX → GMAC ===");
    LOG_I(TAG, "NOTE: Mirrored packets go to Port 6 TX!");
    LOG_I(TAG, "");

    /* Clear counters by reading (MIB are read-clear) */
    lan9646_read_mib_counter(0x1000, 0x0C);  /* Port 1 RX Unicast */
    lan9646_read_mib_counter(0x1000, 0x0A);  /* Port 1 RX Broadcast */
    lan9646_read_mib_counter(0x1000, 0x0B);  /* Port 1 RX Multicast */
    lan9646_read_mib_counter(0x6000, 0x1A);  /* Port 6 TX Unicast */
    lan9646_read_mib_counter(0x6000, 0x18);  /* Port 6 TX Broadcast */
    lan9646_read_mib_counter(0x6000, 0x19);  /* Port 6 TX Multicast */

    LOG_I(TAG, "Counters CLEARED. Waiting 5 seconds...");
    LOG_I(TAG, "NOW: Ping from PC!");

    SysTick_DelayMs(5000);

    /* Read Port 1 RX (packets from PC) */
    p1_rx = lan9646_read_mib_counter(0x1000, 0x0C) +
            lan9646_read_mib_counter(0x1000, 0x0A) +
            lan9646_read_mib_counter(0x1000, 0x0B);

    /* Read Port 6 TX (mirrored packets to GMAC) */
    p6_tx = lan9646_read_mib_counter(0x6000, 0x1A) +
            lan9646_read_mib_counter(0x6000, 0x18) +
            lan9646_read_mib_counter(0x6000, 0x19);

    LOG_I(TAG, "");
    LOG_I(TAG, "Results after 5 seconds:");
    LOG_I(TAG, "  Port 1 RX: %lu packets (from PC)", p1_rx);
    LOG_I(TAG, "  Port 6 TX: %lu packets (to GMAC via mirror)", p6_tx);

    LOG_I(TAG, "");
    if (p1_rx > 0) {
        LOG_I(TAG, "✓ Port 1 RECEIVED %lu packets from PC!", p1_rx);

        if (p6_tx > 0) {
            LOG_I(TAG, "✓ Port 6 TX = %lu packets mirrored to GMAC!", p6_tx);
            LOG_I(TAG, "");
            LOG_I(TAG, "========================================");
            LOG_I(TAG, "  LAN9646 → GMAC PATH WORKING!");
            LOG_I(TAG, "  Switch đang gửi packets đến GMAC");
            LOG_I(TAG, "  NEXT: Config GMAC driver để nhận");
            LOG_I(TAG, "========================================");
        } else {
            LOG_E(TAG, "✗ Port 6 TX = 0");
            LOG_E(TAG, "  Port Mirroring may not be working");
            LOG_E(TAG, "  Check 0x0370 Global Mirroring register");
        }
    } else {
        LOG_E(TAG, "✗ Port 1 received NOTHING");
        LOG_E(TAG, "  Check: PC cable, Port 1 link status");
    }
}
