/**
 * \file            lan9646_monitor_fixed.c
 * \brief           LAN9646 Monitor - Fixed link status + Port 6 debug
 */

#include "s32k3xx_soft_i2c.h"
#include "lan9646.h"
#include "log_debug.h"

#define TAG "LAN9646"
#define LAN9646_I2C_ADDR 0x5F

static softi2c_t g_i2c;
lan9646_t g_lan;  /* Global - không static */

static lan9646r_t i2c_init(void) { return lan9646OK; }
static lan9646r_t i2c_write(uint8_t a, uint16_t m, const uint8_t* d, uint16_t l) {
    return (softi2c_mem_write(&g_i2c, a, m, 2, d, l) == softi2cOK) ? lan9646OK : lan9646ERR;
}
static lan9646r_t i2c_read(uint8_t a, uint16_t m, uint8_t* d, uint16_t l) {
    return (softi2c_mem_read(&g_i2c, a, m, 2, d, l) == softi2cOK) ? lan9646OK : lan9646ERR;
}

void lan9646_init_monitor(void) {
    softi2c_pins_t pins = {
        .scl_port = ETH_MDC_PORT, .scl_pin = ETH_MDC_PIN,
        .sda_port = ETH_MDIO_PORT, .sda_pin = ETH_MDIO_PIN,
        .delay_us = 5
    };
    lan9646_cfg_t cfg = {
        .if_type = LAN9646_IF_I2C,
        .i2c_addr = LAN9646_I2C_ADDR,
        .ops.i2c.init_fn = i2c_init,
        .ops.i2c.write_fn = NULL,
        .ops.i2c.read_fn = NULL,
        .ops.i2c.mem_write_fn = i2c_write,
        .ops.i2c.mem_read_fn = i2c_read
    };

    softi2c_init(&g_i2c, &pins);

    for (volatile uint32_t i = 0; i < 100000; i++);

    if (softi2c_is_device_ready(&g_i2c, LAN9646_I2C_ADDR, 3) != softi2cOK) {
        LOG_E(TAG, "LAN9646 not found!");
        return;
    }

    lan9646_init(&g_lan, &cfg);

    uint16_t id;
    lan9646_read_reg16(&g_lan, 0x0000, &id);
    LOG_I(TAG, "Chip ID: 0x%04X", id);

    if (id == 0x0000 || id == 0xFFFF) {
        LOG_E(TAG, "I2C read failed!");
        return;
    }

    /* DUMP REGISTERS - tìm register map đúng */
    LOG_I(TAG, "");
    LOG_I(TAG, "=== Register Dump (0x0000-0x0100) ===");
    for (uint16_t addr = 0x0000; addr <= 0x0100; addr++) {
        uint8_t val;
        lan9646_read_reg8(&g_lan, addr, &val);
        if (val != 0x00) {
            LOG_I(TAG, "[0x%04X] = 0x%02X", addr, val);
        }
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Port 1 Registers (0x1000-0x1100) ===");
    for (uint16_t addr = 0x1000; addr <= 0x1100; addr++) {
        uint8_t val;
        lan9646_read_reg8(&g_lan, addr, &val);
        if (val != 0x00) {
            LOG_I(TAG, "[0x%04X] = 0x%02X", addr, val);
        }
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Port 6 Registers (0x6000-0x6100) ===");
    for (uint16_t addr = 0x6000; addr <= 0x6100; addr++) {
        uint8_t val;
        lan9646_read_reg8(&g_lan, addr, &val);
        if (val != 0x00) {
            LOG_I(TAG, "[0x%04X] = 0x%02X", addr, val);
        }
    }
}

/* Check links - PHY Basic Status Register */
void lan9646_check_links(void) {
    uint8_t port;
    uint16_t phy_stat;

    LOG_I(TAG, "=== Link Status ===");
    for (port = 1; port <= 4; port++) {
        /* PHY Basic Status: 0xN102 (PHY register 0x01) */
        lan9646_read_reg16(&g_lan, (port * 0x1000) + 0x102, &phy_stat);

        uint8_t link = (phy_stat & 0x0004) ? 1 : 0;  /* Bit 2 */

        if (link) {
            LOG_I(TAG, "Port %d: UP (PHY Status: 0x%04X)", port, phy_stat);
        } else {
            LOG_I(TAG, "Port %d: DOWN", port);
        }
    }
}

/* So sánh Port 1/2 (working) vs Port 3/4 (not working) */
void lan9646_compare_ports(void) {
    uint16_t phy_ctrl[5], phy_status[5];
    uint8_t port;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Compare All Ports ===");

    /* Read all ports */
    for (port = 1; port <= 4; port++) {
        lan9646_read_reg16(&g_lan, (port * 0x1000) + 0x100, &phy_ctrl[port]);
        lan9646_read_reg16(&g_lan, (port * 0x1000) + 0x102, &phy_status[port]);
    }

    /* Print comparison */
    LOG_I(TAG, "Port | PHY Ctrl | PHY Status | Link");
    LOG_I(TAG, "-----|----------|------------|-----");
    for (port = 1; port <= 4; port++) {
        LOG_I(TAG, "  %d  |  0x%04X  |   0x%04X   | %s",
              port, phy_ctrl[port], phy_status[port],
              (phy_status[port] & 0x0004) ? "UP" : "DOWN");
    }

    /* Analysis */
    LOG_I(TAG, "");
    if ((phy_ctrl[1] == phy_ctrl[3]) && (phy_ctrl[2] == phy_ctrl[4])) {
        LOG_I(TAG, "PHY Control registers SAME → Software config OK");
    } else {
        LOG_I(TAG, "PHY Control registers DIFFERENT:");
        LOG_I(TAG, "  Port 1: 0x%04X  Port 3: 0x%04X", phy_ctrl[1], phy_ctrl[3]);
        LOG_I(TAG, "  Port 2: 0x%04X  Port 4: 0x%04X", phy_ctrl[2], phy_ctrl[4]);
    }

    LOG_I(TAG, "");
    if ((phy_status[1] & 0xFF00) == (phy_status[3] & 0xFF00)) {
        LOG_I(TAG, "PHY capabilities SAME → Hardware likely OK");
        LOG_I(TAG, "Issue: NO CABLE or CABLE FAULT on Port 3/4");
    } else {
        LOG_I(TAG, "PHY capabilities DIFFERENT:");
        LOG_I(TAG, "  Port 1: 0x%04X  Port 3: 0x%04X", phy_status[1], phy_status[3]);
        LOG_I(TAG, "→ Possible HARDWARE FAULT on Port 3/4 PHY");
    }
}

/* Debug Port 3 & 4 - CHI TIẾT HỠN */
void lan9646_debug_port34(void) {
    uint8_t port;
    uint16_t phy_ctrl, phy_status, phy_adv;
    uint8_t data8, port_ctrl;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Port 3 & 4 Detailed Debug ===");

    for (port = 3; port <= 4; port++) {
        uint16_t base = port * 0x1000;

        LOG_I(TAG, "");
        LOG_I(TAG, "Port %d:", port);

        /* Port Operation Control 0 (0xN020) */
        lan9646_read_reg8(&g_lan, base + 0x020, &port_ctrl);
        LOG_I(TAG, "  Port Ctrl (0x%04X): 0x%02X", base + 0x020, port_ctrl);

        /* PHY Basic Control */
        lan9646_read_reg16(&g_lan, base + 0x100, &phy_ctrl);
        LOG_I(TAG, "  PHY Control: 0x%04X", phy_ctrl);
        LOG_I(TAG, "    Reset: %s", (phy_ctrl & 0x8000) ? "YES" : "NO");
        LOG_I(TAG, "    Loopback: %s", (phy_ctrl & 0x4000) ? "YES" : "NO");
        LOG_I(TAG, "    Speed: %s", (phy_ctrl & 0x2000) ? "1000M" : "10/100M");
        LOG_I(TAG, "    Auto-neg: %s", (phy_ctrl & 0x1000) ? "EN" : "DIS");
        LOG_I(TAG, "    Power Down: %s", (phy_ctrl & 0x0800) ? "YES" : "NO");
        LOG_I(TAG, "    Isolate: %s", (phy_ctrl & 0x0400) ? "YES" : "NO");

        /* PHY Basic Status */
        lan9646_read_reg16(&g_lan, base + 0x102, &phy_status);
        LOG_I(TAG, "  PHY Status: 0x%04X", phy_status);
        LOG_I(TAG, "    100M FD: %s", (phy_status & 0x4000) ? "YES" : "NO");
        LOG_I(TAG, "    100M HD: %s", (phy_status & 0x2000) ? "YES" : "NO");
        LOG_I(TAG, "    10M FD: %s", (phy_status & 0x1000) ? "YES" : "NO");
        LOG_I(TAG, "    10M HD: %s", (phy_status & 0x0800) ? "YES" : "NO");
        LOG_I(TAG, "    AN Complete: %s", (phy_status & 0x0020) ? "YES" : "NO");
        LOG_I(TAG, "    Link: %s", (phy_status & 0x0004) ? "UP" : "DOWN");

        /* PHY Advertisement */
        lan9646_read_reg16(&g_lan, base + 0x108, &phy_adv);
        LOG_I(TAG, "  PHY Advertisement: 0x%04X", phy_adv);

        /* MSTP State */
        lan9646_write_reg8(&g_lan, (base & 0xF000) + 0xB03, 0x00);
        lan9646_read_reg8(&g_lan, (base & 0xF000) + 0xB04, &data8);
        LOG_I(TAG, "  MSTP State: 0x%02X (TX:%s RX:%s Learn:%s)", data8,
              (data8 & 0x04) ? "EN" : "DIS",
              (data8 & 0x02) ? "EN" : "DIS",
              (data8 & 0x01) ? "DIS" : "EN");
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "Strapping: 0x%02X", 0);
    lan9646_read_reg8(&g_lan, 0x0100, &data8);
    LOG_I(TAG, "  0x0100 = 0x%02X", data8);
}
void lan9646_debug_port6(void) {
    uint16_t chip_id;
    uint8_t data8;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Port 6 Debug ===");

    /* Verify chip */
    lan9646_read_reg16(&g_lan, 0x0000, &chip_id);
    if (chip_id == 0x0000 || chip_id == 0xFFFF) {
        LOG_E(TAG, "Chip not responding (ID: 0x%04X)", chip_id);
        return;
    }
    LOG_I(TAG, "Chip ID: 0x%04X", chip_id);

    /* Set MSTP Pointer = 0 */
    lan9646_write_reg8(&g_lan, 0x6B03, 0x00);

    /* Read MSTP State */
    lan9646_read_reg8(&g_lan, 0x6B04, &data8);
    LOG_I(TAG, "Port6 MSTP State (0x6B04): 0x%02X", data8);
    LOG_I(TAG, "  TX Enable (bit2): %s", (data8 & 0x04) ? "YES" : "NO");
    LOG_I(TAG, "  RX Enable (bit1): %s", (data8 & 0x02) ? "YES" : "NO");
    LOG_I(TAG, "  Learning Disable (bit0): %s", (data8 & 0x01) ? "YES" : "NO");
}

/* Try enable Port 3 & 4 - FORCE PHY RESTART */
void lan9646_try_enable_port34(void) {
    uint8_t port, data8;
    uint16_t phy_ctrl;

    LOG_I(TAG, "");
    LOG_I(TAG, "Force enable Port 3 & 4...");

    for (port = 3; port <= 4; port++) {
        uint16_t base = port * 0x1000;

        LOG_I(TAG, "Port %d:", port);

        /* Enable MSTP TX/RX */
        lan9646_write_reg8(&g_lan, (base & 0xF000) + 0xB03, 0x00);
        lan9646_read_reg8(&g_lan, (base & 0xF000) + 0xB04, &data8);
        data8 |= 0x06;  /* TX + RX enable */
        data8 &= ~0x01; /* Learning enable */
        lan9646_write_reg8(&g_lan, (base & 0xF000) + 0xB04, data8);

        /* PHY: Clear power down + isolate, enable auto-neg */
        lan9646_read_reg16(&g_lan, base + 0x100, &phy_ctrl);
        phy_ctrl &= ~0x0C00; /* Clear bit 11 (power down) + bit 10 (isolate) */
        phy_ctrl |= 0x1000;  /* Set bit 12 (auto-neg enable) */
        lan9646_write_reg16(&g_lan, base + 0x100, phy_ctrl);

        /* Restart auto-negotiation - bit 9 */
        phy_ctrl |= 0x0200;
        lan9646_write_reg16(&g_lan, base + 0x100, phy_ctrl);

        LOG_I(TAG, "  PHY restarted");

        /* Small delay */
        for (volatile uint32_t i = 0; i < 100000; i++);

        /* Verify */
        lan9646_read_reg16(&g_lan, base + 0x100, &phy_ctrl);
        LOG_I(TAG, "  PHY Control: 0x%04X", phy_ctrl);

        lan9646_read_reg8(&g_lan, (base & 0xF000) + 0xB04, &data8);
        LOG_I(TAG, "  MSTP: 0x%02X", data8);
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "Wait 3 seconds for auto-negotiation...");
    for (volatile uint32_t i = 0; i < 3000000; i++);
}

/* Try enable Port 6 - ĐÚNG THEO DATASHEET */
void lan9646_try_enable_port6(void) {
    uint8_t data8;
    uint16_t chip_id;

    LOG_I(TAG, "");
    LOG_I(TAG, "Enable Port 6 (CPU Port)...");

    /* Verify chip responding */
    lan9646_read_reg16(&g_lan, 0x0000, &chip_id);
    if (chip_id == 0x0000 || chip_id == 0xFFFF) {
        LOG_E(TAG, "Chip not responding (ID: 0x%04X)", chip_id);
        return;
    }

    /* Set MSTP Pointer = 0 (MSTP 0) */
    lan9646_write_reg8(&g_lan, 0x6B03, 0x00);

    /* Read current MSTP State */
    lan9646_read_reg8(&g_lan, 0x6B04, &data8);
    LOG_I(TAG, "Port6 MSTP State before: 0x%02X", data8);
    LOG_I(TAG, "  TX Enable: %s", (data8 & 0x04) ? "YES" : "NO");
    LOG_I(TAG, "  RX Enable: %s", (data8 & 0x02) ? "YES" : "NO");

    /* Enable TX + RX (bit 2 + bit 1) */
    data8 |= 0x06;
    lan9646_write_reg8(&g_lan, 0x6B04, data8);

    /* Verify */
    lan9646_read_reg8(&g_lan, 0x6B04, &data8);
    LOG_I(TAG, "Port6 MSTP State after: 0x%02X %s", data8,
          ((data8 & 0x06) == 0x06) ? "SUCCESS" : "FAILED");
}

void lan9646_periodic(void) {
    static uint32_t cnt;
    if (++cnt >= 100000) {
        cnt = 0;
        lan9646_check_links();
    }
}

