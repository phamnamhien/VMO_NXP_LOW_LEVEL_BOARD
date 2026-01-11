/**
 * \file            lan9646_dump.c
 * \brief           LAN9646 Complete Register Dump Implementation
 */

#include "lan9646.h"
#include "lan9646_dump.h"
#include "log_debug.h"
#include <stdio.h>

#define TAG "DUMP"

/*===========================================================================*/
/*                          HELPER FUNCTIONS                                  */
/*===========================================================================*/

static void separator(const char* title) {
    LOG_I(TAG, "");
    LOG_I(TAG, "============================================================");
    LOG_I(TAG, "  %s", title);
    LOG_I(TAG, "============================================================");
}

static void print_reg8(lan9646_t* h, const char* name, uint16_t addr) {
    uint8_t val;
    if (lan9646_read_reg8(h, addr, &val) == lan9646OK) {
        LOG_I(TAG, "[0x%04X] %-32s = 0x%02X", addr, name, val);
    } else {
        LOG_E(TAG, "[0x%04X] %-32s = READ ERROR", addr, name);
    }
}

static void print_reg16(lan9646_t* h, const char* name, uint16_t addr) {
    uint16_t val;
    if (lan9646_read_reg16(h, addr, &val) == lan9646OK) {
        LOG_I(TAG, "[0x%04X] %-32s = 0x%04X", addr, name, val);
    } else {
        LOG_E(TAG, "[0x%04X] %-32s = READ ERROR", addr, name);
    }
}

static void print_reg32(lan9646_t* h, const char* name, uint16_t addr) {
    uint32_t val;
    if (lan9646_read_reg32(h, addr, &val) == lan9646OK) {
        LOG_I(TAG, "[0x%04X] %-32s = 0x%08lX", addr, name, (unsigned long)val);
    } else {
        LOG_E(TAG, "[0x%04X] %-32s = READ ERROR", addr, name);
    }
}

static uint8_t read8(lan9646_t* h, uint16_t addr) {
    uint8_t val = 0;
    lan9646_read_reg8(h, addr, &val);
    return val;
}

static uint16_t read16(lan9646_t* h, uint16_t addr) {
    uint16_t val = 0;
    lan9646_read_reg16(h, addr, &val);
    return val;
}

static uint32_t read32(lan9646_t* h, uint16_t addr) {
    uint32_t val = 0;
    lan9646_read_reg32(h, addr, &val);
    return val;
}

/**
 * \brief           Read MIB counter (corrected per datasheet)
 */
static uint32_t read_mib(lan9646_t* h, uint8_t port, uint8_t index) {
    uint16_t base = (uint16_t)port << 12;
    uint32_t ctrl;
    uint32_t data = 0;
    uint32_t timeout = 1000;

    /* Write: MIB Index [23:16] + Read Enable [25] */
    ctrl = ((uint32_t)index << 16) | LAN9646_MIB_READ_EN;
    lan9646_write_reg32(h, base | 0x0500, ctrl);

    /* Poll until bit 25 clears */
    do {
        lan9646_read_reg32(h, base | 0x0500, &ctrl);
        if (--timeout == 0) break;
    } while (ctrl & LAN9646_MIB_READ_EN);

    /* Read data: bits [35:32] in ctrl[3:0], bits [31:0] in data reg */
    lan9646_read_reg32(h, base | 0x0504, &data);

    return data;
}

/*===========================================================================*/
/*                          GLOBAL DUMP                                       */
/*===========================================================================*/

void lan9646_dump_global(lan9646_t* h) {
    if (!h) return;

    separator("GLOBAL REGISTERS");

    /* Chip ID */
    LOG_I(TAG, "--- Chip Identification ---");
    print_reg8(h, "CHIP_ID0 (fixed=0x00)", 0x0000);
    print_reg8(h, "CHIP_ID1 (MSB=0x94)", 0x0001);
    print_reg8(h, "CHIP_ID2 (LSB=0x77)", 0x0002);
    print_reg8(h, "CHIP_ID3 (Rev+Reset)", 0x0003);

    uint8_t id1 = read8(h, 0x0001);
    uint8_t id2 = read8(h, 0x0002);
    uint8_t id3 = read8(h, 0x0003);
    LOG_I(TAG, "  -> Full Chip ID: 0x%02X%02X", id1, id2);
    LOG_I(TAG, "  -> Revision: %d", (id3 >> 4) & 0x0F);

    /* PME */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- PME Control ---");
    print_reg8(h, "PME_PIN_CTRL", 0x0006);

    /* Global Interrupts */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Global Interrupts ---");
    print_reg32(h, "GLOBAL_INT_STATUS", 0x0010);
    print_reg32(h, "GLOBAL_INT_MASK", 0x0014);
    print_reg32(h, "PORT_INT_STATUS", 0x0018);
    print_reg32(h, "PORT_INT_MASK", 0x001C);

    /* I/O Control */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- I/O Control ---");
    print_reg8(h, "IO_CTRL0", 0x0100);
    print_reg32(h, "LED_OVERRIDE", 0x0120);
    print_reg32(h, "LED_OUTPUT", 0x0124);

    /* PHY Power */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- PHY Power ---");
    print_reg8(h, "PHY_POWER_CTRL", 0x0201);

    /* Switch Operation */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Switch Operation ---");
    print_reg8(h, "SWITCH_OP", 0x0300);

    /* Switch MAC Address */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Switch MAC Address ---");
    uint8_t mac[6];
    for (int i = 0; i < 6; i++) {
        mac[i] = read8(h, 0x0302 + i);
    }
    LOG_I(TAG, "  -> MAC: %02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    /* MIB Control */
    print_reg8(h, "SWITCH_MIB_CTRL", 0x0308);

    /* LUE Control */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Lookup Engine (LUE) ---");
    print_reg8(h, "LUE_CTRL0", 0x0310);
    print_reg8(h, "LUE_CTRL1", 0x0311);
    print_reg8(h, "LUE_CTRL2", 0x0312);
    print_reg8(h, "AGE_PERIOD", 0x0313);

    uint8_t lue0 = read8(h, 0x0310);
    LOG_I(TAG, "  -> VLAN Enable: %s", (lue0 & 0x10) ? "YES" : "NO");
    LOG_I(TAG, "  -> Learning Disable: %s", (lue0 & 0x01) ? "YES" : "NO");

    /* ALU Interrupt */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- ALU Interrupt ---");
    print_reg8(h, "ALU_INT_STATUS", 0x0314);
    print_reg8(h, "ALU_INT_MASK", 0x0315);

    /* Unknown Destination */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Unknown Destination ---");
    print_reg32(h, "UNKNOWN_UNICAST_CTRL", 0x0320);
    print_reg32(h, "UNKNOWN_MULTICAST_CTRL", 0x0324);
    print_reg32(h, "UNKNOWN_VID_CTRL", 0x0328);

    /* Mirror */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Global Mirror ---");
    print_reg8(h, "GLOBAL_MIRROR_CTRL", 0x0370);

    /* Queue Management */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Queue Management ---");
    print_reg32(h, "QUEUE_MGMT_CTRL0", 0x0390);
}

/*===========================================================================*/
/*                          PORT DUMP                                         */
/*===========================================================================*/

void lan9646_dump_port(lan9646_t* h, uint8_t port) {
    if (!h) return;
    if (port == 0 || port == 5 || port > 7) return;

    char title[64];
    char name[48];
    uint16_t base = (uint16_t)port << 12;

    snprintf(title, sizeof(title), "PORT %d REGISTERS %s", port,
             (port <= 4) ? "(PHY)" : "(RGMII)");
    separator(title);

    /* Default Tag */
    LOG_I(TAG, "--- Default Tag (802.1Q) ---");
    snprintf(name, sizeof(name), "P%d_DEFAULT_TAG0", port);
    print_reg8(h, name, base | 0x0000);
    snprintf(name, sizeof(name), "P%d_DEFAULT_TAG1", port);
    print_reg8(h, name, base | 0x0001);

    uint8_t tag0 = read8(h, base | 0x0000);
    uint8_t tag1 = read8(h, base | 0x0001);
    uint16_t pvid = ((tag0 & 0x0F) << 8) | tag1;
    uint8_t pcp = (tag0 >> 5) & 0x07;
    LOG_I(TAG, "  -> PVID: %d, PCP: %d", pvid, pcp);

    /* PME */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- PME/WoL ---");
    snprintf(name, sizeof(name), "P%d_PME_EVENT", port);
    print_reg8(h, name, base | 0x0013);
    snprintf(name, sizeof(name), "P%d_PME_ENABLE", port);
    print_reg8(h, name, base | 0x0017);

    /* Interrupt */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port Interrupt ---");
    snprintf(name, sizeof(name), "P%d_INT_STATUS", port);
    print_reg8(h, name, base | 0x001B);
    snprintf(name, sizeof(name), "P%d_INT_MASK", port);
    print_reg8(h, name, base | 0x001F);

    /* Operation Control */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Operation Control ---");
    snprintf(name, sizeof(name), "P%d_OP_CTRL0", port);
    print_reg8(h, name, base | 0x0020);
    snprintf(name, sizeof(name), "P%d_OP_CTRL1", port);
    print_reg8(h, name, base | 0x0021);

    /* Port Status */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port Status ---");
    snprintf(name, sizeof(name), "P%d_STATUS", port);
    print_reg16(h, name, base | 0x0030);

    uint16_t status = read16(h, base | 0x0030);
    if (port <= 4) {
        /* PHY port status decode */
        bool link = (status & 0x0001) != 0;
        bool duplex = (status & 0x0002) != 0;
        uint8_t speed = (status >> 2) & 0x03;
        const char* speed_str[] = {"10M", "100M", "1000M", "???"};
        LOG_I(TAG, "  -> Link: %s, Speed: %s, Duplex: %s",
              link ? "UP" : "DOWN", speed_str[speed], duplex ? "Full" : "Half");
    } else {
        /* RGMII port status decode */
        uint8_t op = (status >> 8) & 0x0F;
        const char* op_str;
        switch (op) {
            case 0x0F: op_str = "1000M Full"; break;
            case 0x0D: op_str = "100M Full"; break;
            case 0x05: op_str = "100M Half"; break;
            case 0x03: op_str = "10M Full"; break;
            case 0x01: op_str = "10M Half"; break;
            default:   op_str = "Link Down"; break;
        }
        LOG_I(TAG, "  -> Operation: %s (0x%02X)", op_str, op);
    }

    /* PHY or XMII */
    if (port <= 4) {
        lan9646_dump_phy(h, port);
    } else {
        lan9646_dump_xmii(h, port);
    }

    /* MAC Control */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- MAC Control ---");
    snprintf(name, sizeof(name), "P%d_MAC_CTRL0", port);
    print_reg8(h, name, base | 0x0400);
    snprintf(name, sizeof(name), "P%d_MAC_CTRL1", port);
    print_reg8(h, name, base | 0x0401);

    /* Rate Limiting */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Rate Limiting ---");
    snprintf(name, sizeof(name), "P%d_IN_RATE_CTRL", port);
    print_reg8(h, name, base | 0x0410);
    snprintf(name, sizeof(name), "P%d_OUT_RATE_CTRL", port);
    print_reg8(h, name, base | 0x0420);

    /* Classification */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Classification ---");
    snprintf(name, sizeof(name), "P%d_CLASS_CTRL", port);
    print_reg8(h, name, base | 0x0800);

    /* Mirror */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port Mirror ---");
    snprintf(name, sizeof(name), "P%d_MIRROR_CTRL", port);
    print_reg8(h, name, base | 0x0804);

    uint8_t mirror = read8(h, base | 0x0804);
    LOG_I(TAG, "  -> Sniffer: %s, RX Sniff: %s, TX Sniff: %s",
          (mirror & 0x02) ? "YES" : "NO",
          (mirror & 0x40) ? "YES" : "NO",
          (mirror & 0x20) ? "YES" : "NO");

    /* Priority */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Priority ---");
    snprintf(name, sizeof(name), "P%d_PRIO_CTRL", port);
    print_reg8(h, name, base | 0x0808);

    /* Queue */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Queue Control ---");
    snprintf(name, sizeof(name), "P%d_QUEUE_CTRL", port);
    print_reg8(h, name, base | 0x0A00);

    /* Membership */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port VLAN Membership ---");
    snprintf(name, sizeof(name), "P%d_MEMBERSHIP", port);
    print_reg32(h, name, base | 0x0A04);

    uint32_t memb = read32(h, base | 0x0A04);
    LOG_I(TAG, "  -> Can forward to ports:");
    for (int p = 1; p <= 7; p++) {
        if (p == 5) continue;
        if (memb & (1 << (p - 1))) {
            LOG_I(TAG, "     Port %d: YES", p);
        }
    }

    /* Port Control 2 */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port Control 2 ---");
    snprintf(name, sizeof(name), "P%d_CTRL2", port);
    print_reg8(h, name, base | 0x0B00);

    /* MSTP */
    LOG_I(TAG, "");
    LOG_I(TAG, "--- MSTP State ---");
    snprintf(name, sizeof(name), "P%d_MSTP_PTR", port);
    print_reg8(h, name, base | 0x0B01);
    snprintf(name, sizeof(name), "P%d_MSTP_STATE", port);
    print_reg8(h, name, base | 0x0B04);

    uint8_t mstp = read8(h, base | 0x0B04);
    LOG_I(TAG, "  -> TX Enable: %s", (mstp & 0x04) ? "YES" : "NO");
    LOG_I(TAG, "  -> RX Enable: %s", (mstp & 0x02) ? "YES" : "NO");
    LOG_I(TAG, "  -> Learning: %s", (mstp & 0x01) ? "DISABLED" : "ENABLED");
}

/*===========================================================================*/
/*                          PHY DUMP                                          */
/*===========================================================================*/

void lan9646_dump_phy(lan9646_t* h, uint8_t port) {
    if (!h || port == 0 || port > 4) return;

    char name[48];
    uint16_t base = (uint16_t)port << 12;

    LOG_I(TAG, "");
    LOG_I(TAG, "--- PHY Registers ---");

    snprintf(name, sizeof(name), "P%d_PHY_BASIC_CTRL", port);
    print_reg16(h, name, base | 0x0100);
    snprintf(name, sizeof(name), "P%d_PHY_BASIC_STATUS", port);
    print_reg16(h, name, base | 0x0102);
    snprintf(name, sizeof(name), "P%d_PHY_ID_H", port);
    print_reg16(h, name, base | 0x0104);
    snprintf(name, sizeof(name), "P%d_PHY_ID_L", port);
    print_reg16(h, name, base | 0x0106);
    snprintf(name, sizeof(name), "P%d_PHY_AUTONEG_ADV", port);
    print_reg16(h, name, base | 0x0108);
    snprintf(name, sizeof(name), "P%d_PHY_LP_ABILITY", port);
    print_reg16(h, name, base | 0x010A);
    snprintf(name, sizeof(name), "P%d_PHY_1000_CTRL", port);
    print_reg16(h, name, base | 0x0112);
    snprintf(name, sizeof(name), "P%d_PHY_1000_STATUS", port);
    print_reg16(h, name, base | 0x0114);

    /* Decode PHY status */
    uint16_t phy_stat = read16(h, base | 0x0102);
    LOG_I(TAG, "  -> AutoNeg Complete: %s", (phy_stat & 0x0020) ? "YES" : "NO");
    LOG_I(TAG, "  -> Link Status: %s", (phy_stat & 0x0004) ? "UP" : "DOWN");
}

/*===========================================================================*/
/*                          XMII DUMP                                         */
/*===========================================================================*/

void lan9646_dump_xmii(lan9646_t* h, uint8_t port) {
    if (!h || port < 6 || port > 7) return;

    char name[48];
    uint16_t base = (uint16_t)port << 12;

    LOG_I(TAG, "");
    LOG_I(TAG, "--- XMII/RGMII Control (CRITICAL FOR GMAC!) ---");

    snprintf(name, sizeof(name), "P%d_XMII_CTRL0", port);
    print_reg8(h, name, base | 0x0300);
    snprintf(name, sizeof(name), "P%d_XMII_CTRL1", port);
    print_reg8(h, name, base | 0x0301);

    uint8_t ctrl0 = read8(h, base | 0x0300);
    uint8_t ctrl1 = read8(h, base | 0x0301);

    LOG_I(TAG, "");
    LOG_I(TAG, "  XMII_CTRL0 (0x%02X) decode:", ctrl0);
    LOG_I(TAG, "    Duplex: %s", (ctrl0 & 0x40) ? "Full" : "Half");
    LOG_I(TAG, "    TX Flow Control: %s", (ctrl0 & 0x20) ? "ON" : "OFF");
    LOG_I(TAG, "    Speed 100: %s", (ctrl0 & 0x10) ? "YES" : "NO (10)");
    LOG_I(TAG, "    RX Flow Control: %s", (ctrl0 & 0x08) ? "ON" : "OFF");

    LOG_I(TAG, "");
    LOG_I(TAG, "  XMII_CTRL1 (0x%02X) decode:", ctrl1);
    LOG_I(TAG, "    Speed 1000: %s", (ctrl1 & 0x40) ? "NO (10/100)" : "YES (1000)");
    LOG_I(TAG, "    RX Ingress Delay (bit4): %s", (ctrl1 & 0x10) ? "ON (1.5ns)" : "OFF");
    LOG_I(TAG, "    TX Egress Delay (bit3): %s", (ctrl1 & 0x08) ? "ON (1.5ns)" : "OFF");
    LOG_I(TAG, "    MII/RMII Mode (bit2): 0x%X", (ctrl1 >> 2) & 1);

    /* SGMII for Port 7 */
    if (port == 7) {
        LOG_I(TAG, "");
        LOG_I(TAG, "--- SGMII Control (Port 7) ---");
        snprintf(name, sizeof(name), "P%d_SGMII_ADDR", port);
        print_reg32(h, name, base | 0x0200);
        snprintf(name, sizeof(name), "P%d_SGMII_DATA", port);
        print_reg16(h, name, base | 0x0206);
    }
}

/*===========================================================================*/
/*                          MIB DUMP                                          */
/*===========================================================================*/

void lan9646_dump_mib(lan9646_t* h, uint8_t port) {
    if (!h) return;
    if (port == 0 || port == 5 || port > 7) return;

    char title[64];
    snprintf(title, sizeof(title), "PORT %d MIB COUNTERS", port);
    separator(title);

    LOG_I(TAG, "");
    LOG_I(TAG, "--- RX Counters ---");
    LOG_I(TAG, "  RX Hi Priority Bytes: %lu", (unsigned long)read_mib(h, port, 0x00));
    LOG_I(TAG, "  RX Undersize:         %lu", (unsigned long)read_mib(h, port, 0x01));
    LOG_I(TAG, "  RX Fragments:         %lu", (unsigned long)read_mib(h, port, 0x02));
    LOG_I(TAG, "  RX Oversize:          %lu", (unsigned long)read_mib(h, port, 0x03));
    LOG_I(TAG, "  RX Jabbers:           %lu", (unsigned long)read_mib(h, port, 0x04));
    LOG_I(TAG, "  RX Symbol Errors:     %lu", (unsigned long)read_mib(h, port, 0x05));
    LOG_I(TAG, "  RX CRC Errors:        %lu", (unsigned long)read_mib(h, port, 0x06));
    LOG_I(TAG, "  RX Alignment Errors:  %lu", (unsigned long)read_mib(h, port, 0x07));
    LOG_I(TAG, "  RX Control (0x8808):  %lu", (unsigned long)read_mib(h, port, 0x08));
    LOG_I(TAG, "  RX Pause:             %lu", (unsigned long)read_mib(h, port, 0x09));
    LOG_I(TAG, "  RX Broadcast:         %lu", (unsigned long)read_mib(h, port, 0x0A));
    LOG_I(TAG, "  RX Multicast:         %lu", (unsigned long)read_mib(h, port, 0x0B));
    LOG_I(TAG, "  RX Unicast:           %lu", (unsigned long)read_mib(h, port, 0x0C));
    LOG_I(TAG, "  RX 64:                %lu", (unsigned long)read_mib(h, port, 0x0D));
    LOG_I(TAG, "  RX 65-127:            %lu", (unsigned long)read_mib(h, port, 0x0E));
    LOG_I(TAG, "  RX 128-255:           %lu", (unsigned long)read_mib(h, port, 0x0F));
    LOG_I(TAG, "  RX 256-511:           %lu", (unsigned long)read_mib(h, port, 0x10));
    LOG_I(TAG, "  RX 512-1023:          %lu", (unsigned long)read_mib(h, port, 0x11));
    LOG_I(TAG, "  RX 1024-1522:         %lu", (unsigned long)read_mib(h, port, 0x12));
    LOG_I(TAG, "  RX 1523-2000:         %lu", (unsigned long)read_mib(h, port, 0x13));
    LOG_I(TAG, "  RX 2001+:             %lu", (unsigned long)read_mib(h, port, 0x14));

    LOG_I(TAG, "");
    LOG_I(TAG, "--- TX Counters ---");
    LOG_I(TAG, "  TX Hi Priority Bytes: %lu", (unsigned long)read_mib(h, port, 0x60));
    LOG_I(TAG, "  TX Late Collisions:   %lu", (unsigned long)read_mib(h, port, 0x61));
    LOG_I(TAG, "  TX Pause:             %lu", (unsigned long)read_mib(h, port, 0x62));
    LOG_I(TAG, "  TX Broadcast:         %lu", (unsigned long)read_mib(h, port, 0x63));
    LOG_I(TAG, "  TX Multicast:         %lu", (unsigned long)read_mib(h, port, 0x64));
    LOG_I(TAG, "  TX Unicast:           %lu", (unsigned long)read_mib(h, port, 0x65));
    LOG_I(TAG, "  TX Deferred:          %lu", (unsigned long)read_mib(h, port, 0x66));
    LOG_I(TAG, "  TX Total Collisions:  %lu", (unsigned long)read_mib(h, port, 0x67));
    LOG_I(TAG, "  TX Excess Collisions: %lu", (unsigned long)read_mib(h, port, 0x68));
    LOG_I(TAG, "  TX Single Collision:  %lu", (unsigned long)read_mib(h, port, 0x69));
    LOG_I(TAG, "  TX Multi Collision:   %lu", (unsigned long)read_mib(h, port, 0x6A));

    LOG_I(TAG, "");
    LOG_I(TAG, "--- Summary Counters ---");
    LOG_I(TAG, "  RX Total Packets:     %lu", (unsigned long)read_mib(h, port, 0x80));
    LOG_I(TAG, "  TX Total Bytes:       %lu", (unsigned long)read_mib(h, port, 0x81));
    LOG_I(TAG, "  RX Dropped:           %lu", (unsigned long)read_mib(h, port, 0x82));
    LOG_I(TAG, "  TX Dropped:           %lu", (unsigned long)read_mib(h, port, 0x83));
}

void lan9646_dump_all_mib(lan9646_t* h) {
    uint8_t ports[] = {1, 2, 3, 4, 6, 7};
    for (int i = 0; i < 6; i++) {
        lan9646_dump_mib(h, ports[i]);
    }
}

/*===========================================================================*/
/*                          STATUS SUMMARY                                    */
/*===========================================================================*/

void lan9646_dump_status_summary(lan9646_t* h) {
    if (!h) return;

    separator("PORT STATUS SUMMARY");

    LOG_I(TAG, "");
    LOG_I(TAG, "Port | Type   | Link  | Speed  | Duplex | TX_EN | RX_EN | Learn");
    LOG_I(TAG, "-----|--------|-------|--------|--------|-------|-------|------");

    uint8_t ports[] = {1, 2, 3, 4, 6, 7};

    for (int i = 0; i < 6; i++) {
        uint8_t port = ports[i];
        uint16_t base = (uint16_t)port << 12;
        uint8_t port_stat = read8(h, base | 0x0030);  /* 8-bit Port Status */
        uint8_t mstp = read8(h, base | 0x0B04);

        bool link_up;
        const char* speed_str;
        const char* duplex_str;
        const char* type_str = (port <= 4) ? "PHY   " : "RGMII ";

        if (port <= 4) {
            /* PHY ports: Link from PHY Basic Status (0xN102)
             * Bit 2 is LATCH LOW - must read TWICE!
             * 1st read clears latch, 2nd read returns actual value */
            (void)read16(h, base | 0x0102);  /* Clear latch */
            uint16_t phy_stat = read16(h, base | 0x0102);  /* Real value */
            link_up = (phy_stat & 0x0004) != 0;

            /* Speed/Duplex from Port Status (0xN030) bits [4:3] and [2] */
            uint8_t speed = (port_stat >> 3) & 0x03;
            const char* speeds[] = {"10M  ", "100M ", "1000M", "???  "};
            speed_str = speeds[speed];
            duplex_str = (port_stat & 0x04) ? "Full" : "Half";
        } else {
            /* RGMII ports: Speed/Duplex from Port Status bits [4:3] and [2] */
            uint8_t speed = (port_stat >> 3) & 0x03;
            link_up = true;  /* RGMII has no auto link detect, assume up */
            switch (speed) {
                case 2:  speed_str = "1000M"; break;
                case 1:  speed_str = "100M "; break;
                case 0:  speed_str = "10M  "; break;
                default: speed_str = "---- "; break;
            }
            duplex_str = (port_stat & 0x04) ? "Full" : "Half";
        }

        if (!link_up) {
            speed_str = "---- ";
            duplex_str = "----";
        }

        LOG_I(TAG, "  %d  | %s | %-5s | %s | %-6s | %-5s | %-5s | %s",
              port, type_str,
              link_up ? "UP" : "DOWN",
              speed_str, duplex_str,
              (mstp & 0x04) ? "YES" : "NO",
              (mstp & 0x02) ? "YES" : "NO",
              (mstp & 0x01) ? "NO" : "YES");
    }
}

/*===========================================================================*/
/*                          GMAC CHECK                                        */
/*===========================================================================*/

void lan9646_dump_gmac_check(lan9646_t* h) {
    if (!h) return;

    separator("GMAC CONFIGURATION CHECK (Port 6)");

    uint8_t ctrl0 = read8(h, 0x6300);
    uint8_t ctrl1 = read8(h, 0x6301);
    uint8_t mstp = read8(h, 0x6B04);
    uint8_t status = read8(h, 0x6030);  /* 8-bit register! */
    uint32_t membership = read32(h, 0x6A04);

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Raw Register Values ===");
    LOG_I(TAG, "  XMII_CTRL0 (0x6300): 0x%02X", ctrl0);
    LOG_I(TAG, "  XMII_CTRL1 (0x6301): 0x%02X", ctrl1);
    LOG_I(TAG, "  PORT_STATUS (0x6030): 0x%02X", status);
    LOG_I(TAG, "  MSTP_STATE (0x6B04): 0x%02X", mstp);
    LOG_I(TAG, "  MEMBERSHIP (0x6A04): 0x%08lX", (unsigned long)membership);

    LOG_I(TAG, "");
    LOG_I(TAG, "=== RGMII Interface Settings ===");
    LOG_I(TAG, "  Duplex:           %s", (ctrl0 & 0x40) ? "Full" : "Half");
    LOG_I(TAG, "  TX Flow Control:  %s", (ctrl0 & 0x20) ? "Enabled" : "Disabled");
    LOG_I(TAG, "  RX Flow Control:  %s", (ctrl0 & 0x08) ? "Enabled" : "Disabled");

    bool speed_1000 = !(ctrl1 & 0x40);
    bool speed_100 = (ctrl0 & 0x10);
    LOG_I(TAG, "  Speed Setting:    %s",
          speed_1000 ? "1000 Mbps" : (speed_100 ? "100 Mbps" : "10 Mbps"));

    LOG_I(TAG, "");
    LOG_I(TAG, "=== RGMII Delay Settings (CRITICAL!) ===");
    LOG_I(TAG, "  TX Egress Delay (bit3):  %s", (ctrl1 & 0x08) ? "ON (1.5ns)" : "OFF");
    LOG_I(TAG, "  RX Ingress Delay (bit4): %s", (ctrl1 & 0x10) ? "ON (1.5ns)" : "OFF");

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Port Status (0xN030 is 8-bit) ===");
    /* Bits 4:3 = Speed, Bit 2 = Duplex, Bit 1 = TX FC, Bit 0 = RX FC */
    uint8_t spd = (status >> 3) & 0x03;
    const char* spd_str;
    switch (spd) {
        case 2:  spd_str = "1000 Mbps"; break;
        case 1:  spd_str = "100 Mbps"; break;
        case 0:  spd_str = "10 Mbps"; break;
        default: spd_str = "Unknown"; break;
    }
    LOG_I(TAG, "  Speed Status:  %s", spd_str);
    LOG_I(TAG, "  Duplex Status: %s", (status & 0x04) ? "Full" : "Half");
    LOG_I(TAG, "  TX FC Status:  %s", (status & 0x02) ? "Enabled" : "Disabled");
    LOG_I(TAG, "  RX FC Status:  %s", (status & 0x01) ? "Enabled" : "Disabled");

    LOG_I(TAG, "");
    LOG_I(TAG, "=== MSTP State ===");
    LOG_I(TAG, "  TX Enable: %s", (mstp & 0x04) ? "YES" : "NO");
    LOG_I(TAG, "  RX Enable: %s", (mstp & 0x02) ? "YES" : "NO");
    LOG_I(TAG, "  Learning:  %s", (mstp & 0x01) ? "DISABLED" : "ENABLED");

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Forwarding Membership ===");
    LOG_I(TAG, "  Port 1: %s", (membership & 0x01) ? "YES" : "NO");
    LOG_I(TAG, "  Port 2: %s", (membership & 0x02) ? "YES" : "NO");
    LOG_I(TAG, "  Port 3: %s", (membership & 0x04) ? "YES" : "NO");
    LOG_I(TAG, "  Port 4: %s", (membership & 0x08) ? "YES" : "NO");
    LOG_I(TAG, "  Port 7: %s", (membership & 0x40) ? "YES" : "NO");

    LOG_I(TAG, "");
    LOG_I(TAG, "============================================================");
    LOG_I(TAG, "  RECOMMENDED S32K3xx GMAC SETTINGS:");
    LOG_I(TAG, "============================================================");
    LOG_I(TAG, "  Interface: RGMII");
    LOG_I(TAG, "  Speed/Duplex: Match Port 6 status above");
    LOG_I(TAG, "");
    LOG_I(TAG, "  If LAN9646 TX_DELAY=ON  -> S32K GMAC TX_DELAY=OFF");
    LOG_I(TAG, "  If LAN9646 TX_DELAY=OFF -> S32K GMAC TX_DELAY=ON");
    LOG_I(TAG, "  If LAN9646 RX_DELAY=ON  -> S32K GMAC RX_DELAY=OFF");
    LOG_I(TAG, "  If LAN9646 RX_DELAY=OFF -> S32K GMAC RX_DELAY=ON");
    LOG_I(TAG, "");
    LOG_I(TAG, "  Current LAN9646: TX_DLY=%s, RX_DLY=%s",
          (ctrl1 & 0x08) ? "ON" : "OFF", (ctrl1 & 0x10) ? "ON" : "OFF");
    LOG_I(TAG, "  -> S32K GMAC:    TX_DLY=%s, RX_DLY=%s",
          (ctrl1 & 0x08) ? "OFF" : "ON", (ctrl1 & 0x10) ? "OFF" : "ON");
}

/*===========================================================================*/
/*                          MEMBERSHIP DUMP                                   */
/*===========================================================================*/

void lan9646_dump_membership(lan9646_t* h) {
    if (!h) return;

    separator("PORT VLAN MEMBERSHIP TABLE");

    LOG_I(TAG, "");
    LOG_I(TAG, "         | Forward to Port:");
    LOG_I(TAG, "Port     |  1    2    3    4    6    7");
    LOG_I(TAG, "---------|-----------------------------");

    uint8_t ports[] = {1, 2, 3, 4, 6, 7};

    for (int i = 0; i < 6; i++) {
        uint8_t port = ports[i];
        uint32_t memb = read32(h, ((uint16_t)port << 12) | 0x0A04);

        LOG_I(TAG, "   %d     |  %s    %s    %s    %s    %s    %s",
              port,
              (memb & 0x01) ? "Y" : "-",
              (memb & 0x02) ? "Y" : "-",
              (memb & 0x04) ? "Y" : "-",
              (memb & 0x08) ? "Y" : "-",
              (memb & 0x20) ? "Y" : "-",
              (memb & 0x40) ? "Y" : "-");
    }
}

/*===========================================================================*/
/*                          QUICK DUMP                                        */
/*===========================================================================*/

void lan9646_dump_quick(lan9646_t* h) {
    if (!h) return;

    LOG_I(TAG, "");
    LOG_I(TAG, "############################################################");
    LOG_I(TAG, "#           LAN9646 QUICK DIAGNOSTIC DUMP                  #");
    LOG_I(TAG, "############################################################");

    /* Chip ID */
    uint8_t id1 = read8(h, 0x0001);
    uint8_t id2 = read8(h, 0x0002);
    uint8_t id3 = read8(h, 0x0003);
    LOG_I(TAG, "");
    LOG_I(TAG, "Chip ID: 0x%02X%02X, Revision: %d", id1, id2, (id3 >> 4) & 0x0F);

    /* Port Status Summary */
    lan9646_dump_status_summary(h);

    /* GMAC Check */
    lan9646_dump_gmac_check(h);

    /* Membership */
    lan9646_dump_membership(h);

    LOG_I(TAG, "");
    LOG_I(TAG, "############################################################");
}

/*===========================================================================*/
/*                          MAIN DUMP FUNCTIONS                               */
/*===========================================================================*/

void lan9646_dump_all_registers(lan9646_t* h) {
    if (!h) {
        LOG_E(TAG, "Handle is NULL!");
        return;
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "############################################################");
    LOG_I(TAG, "#                                                          #");
    LOG_I(TAG, "#           LAN9646 COMPLETE REGISTER DUMP                 #");
    LOG_I(TAG, "#                                                          #");
    LOG_I(TAG, "############################################################");

    lan9646_dump_global(h);
    lan9646_dump_status_summary(h);
    lan9646_dump_gmac_check(h);
    lan9646_dump_membership(h);

    /* Port 6 (GMAC) - Most important */
    lan9646_dump_port(h, 6);

    /* Port 1-4 (PHY) */
    lan9646_dump_port(h, 1);
    lan9646_dump_port(h, 2);
    lan9646_dump_port(h, 3);
    lan9646_dump_port(h, 4);

    /* Port 7 (RGMII/SGMII) */
    lan9646_dump_port(h, 7);

    LOG_I(TAG, "");
    LOG_I(TAG, "############################################################");
    LOG_I(TAG, "#                    END OF DUMP                           #");
    LOG_I(TAG, "############################################################");
}

void lan9646_dump_port6_only(lan9646_t* h) {
    if (!h) {
        LOG_E(TAG, "Handle is NULL!");
        return;
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "############################################################");
    LOG_I(TAG, "#           LAN9646 PORT 6 (GMAC) DUMP                     #");
    LOG_I(TAG, "############################################################");

    lan9646_dump_status_summary(h);
    lan9646_dump_gmac_check(h);
    lan9646_dump_port(h, 6);

    LOG_I(TAG, "");
    LOG_I(TAG, "############################################################");
}

void lan9646_dump_custom(lan9646_t* h, const lan9646_dump_cfg_t* cfg) {
    if (!h || !cfg) return;

    if (cfg->global_regs) lan9646_dump_global(h);
    if (cfg->port_status) lan9646_dump_status_summary(h);

    for (int p = 1; p <= 7; p++) {
        if (p == 5) continue;
        if (cfg->port_regs[p]) {
            lan9646_dump_port(h, p);
        }
    }

    if (cfg->mib_counters) lan9646_dump_all_mib(h);
}

void lan9646_dump_vlan_table(lan9646_t* h, uint16_t start_vid, uint16_t count) {
    if (!h) return;

    separator("VLAN TABLE");

    for (uint16_t vid = start_vid; vid < start_vid + count; vid++) {
        uint32_t entry = read32(h, 0x0480 + (vid - 1) * 4);
        if (entry != 0) {
            LOG_I(TAG, "VID %4d: 0x%08lX", vid, (unsigned long)entry);
        }
    }
}

