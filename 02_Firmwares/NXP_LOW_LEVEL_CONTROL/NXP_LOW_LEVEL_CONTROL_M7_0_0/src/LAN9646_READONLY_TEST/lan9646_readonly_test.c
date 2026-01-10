/**
 * \file            lan9646_readonly_test.c
 * \brief           Test LAN9646 READ ONLY - không ghi gì
 */

#include "s32k3xx_soft_i2c.h"
#include "lan9646.h"
#include "log_debug.h"

#define TAG "LAN9646_TEST"
#define LAN9646_I2C_ADDR 0x5F

/* Global handles */
static softi2c_t g_i2c;
static lan9646_t g_lan;

/* I2C callbacks */
static lan9646r_t lan9646_i2c_init(void) { return lan9646OK; }

static lan9646r_t lan9646_i2c_mem_write(uint8_t dev_addr, uint16_t mem_addr,
                                         const uint8_t* data, uint16_t len) {
    softi2cr_t res = softi2c_mem_write(&g_i2c, dev_addr, mem_addr, 2, data, len);
    return (res == softi2cOK) ? lan9646OK : lan9646ERR;
}

static lan9646r_t lan9646_i2c_mem_read(uint8_t dev_addr, uint16_t mem_addr,
                                        uint8_t* data, uint16_t len) {
    softi2cr_t res = softi2c_mem_read(&g_i2c, dev_addr, mem_addr, 2, data, len);
    return (res == softi2cOK) ? lan9646OK : lan9646ERR;
}

/**
 * \brief           Test LAN9646 READ ONLY
 */
void lan9646_readonly_test(void) {
    softi2c_pins_t i2c_pins;
    lan9646_cfg_t lan_cfg;
    lan9646r_t res;
    uint8_t data8, port;
    uint16_t data16, base;

    LOG_I(TAG, "========================================");
    LOG_I(TAG, "LAN9646 READ-ONLY TEST");
    LOG_I(TAG, "No writes - chip stays in default mode");
    LOG_I(TAG, "========================================");

    /* Init I2C */
    i2c_pins.scl_port = ETH_MDC_PORT;
    i2c_pins.scl_pin = ETH_MDC_PIN;
    i2c_pins.sda_port = ETH_MDIO_PORT;
    i2c_pins.sda_pin = ETH_MDIO_PIN;
    i2c_pins.delay_us = 5;

    if (softi2c_init(&g_i2c, &i2c_pins) != softi2cOK) {
        LOG_E(TAG, "I2C init failed");
        return;
    }

    /* Init LAN9646 driver */
    lan_cfg.if_type = LAN9646_IF_I2C;
    lan_cfg.i2c_addr = LAN9646_I2C_ADDR;
    lan_cfg.ops.i2c.init_fn = lan9646_i2c_init;
    lan_cfg.ops.i2c.mem_write_fn = lan9646_i2c_mem_write;
    lan_cfg.ops.i2c.mem_read_fn = lan9646_i2c_mem_read;
    lan_cfg.ops.i2c.write_fn = NULL;
    lan_cfg.ops.i2c.read_fn = NULL;

    if (lan9646_init(&g_lan, &lan_cfg) != lan9646OK) {
        LOG_E(TAG, "Driver init failed");
        return;
    }

    /* ═══════════════════════════════════════════════════════════════
     * ONLY READ - NO WRITES AT ALL!
     * ═══════════════════════════════════════════════════════════════ */

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Reading Chip ID ===");
    res = lan9646_read_reg16(&g_lan, 0x0000, &data16);
    LOG_I(TAG, "Chip ID (0x0000): 0x%04X", data16);

    res = lan9646_read_reg8(&g_lan, 0x0002, &data8);
    LOG_I(TAG, "Chip Revision (0x0002): 0x%02X (Rev: %d)", data8, data8 & 0x0F);

    LOG_I(TAG, "");
    LOG_I(TAG, "=== Global Registers (Default State) ===");

    res = lan9646_read_reg8(&g_lan, 0x0003, &data8);
    LOG_I(TAG, "Global Mode (0x0003): 0x%02X", data8);

    res = lan9646_read_reg8(&g_lan, 0x0004, &data8);
    LOG_I(TAG, "Port Enable (0x0004): 0x%02X", data8);
    LOG_I(TAG, "  Port 1: %s", (data8 & 0x01) ? "EN" : "DIS");
    LOG_I(TAG, "  Port 2: %s", (data8 & 0x02) ? "EN" : "DIS");
    LOG_I(TAG, "  Port 3: %s", (data8 & 0x04) ? "EN" : "DIS");
    LOG_I(TAG, "  Port 4: %s", (data8 & 0x08) ? "EN" : "DIS");
    LOG_I(TAG, "  Port 5: %s", (data8 & 0x10) ? "EN" : "DIS");
    LOG_I(TAG, "  Port 6: %s", (data8 & 0x20) ? "EN" : "DIS");

    res = lan9646_read_reg8(&g_lan, 0x0300, &data8);
    LOG_I(TAG, "Switch Operation (0x0300): 0x%02X", data8);

    /* Read all port status */
    LOG_I(TAG, "");
    LOG_I(TAG, "=== Port Status (Default) ===");
    for (port = 1; port <= 6; port++) {
        base = port * 0x1000;

        LOG_I(TAG, "--- Port %d ---", port);

        res = lan9646_read_reg8(&g_lan, base + 0x00, &data8);
        LOG_I(TAG, "  Control 0 (0x%04X): 0x%02X", base, data8);

        res = lan9646_read_reg16(&g_lan, base + 0x30, &data16);
        LOG_I(TAG, "  Status (0x%04X): 0x%04X", base + 0x30, data16);
        LOG_I(TAG, "    Link: %s", (data16 & 0x0020) ? "UP" : "DOWN");

        if (port <= 5) {
            res = lan9646_read_reg16(&g_lan, base + 0x00, &data16);
            LOG_I(TAG, "  PHY Control (0x%04X): 0x%04X", base, data16);

            res = lan9646_read_reg16(&g_lan, base + 0x01, &data16);
            LOG_I(TAG, "  PHY Status (0x%04X): 0x%04X", base + 0x01, data16);
            LOG_I(TAG, "    Link: %s", (data16 & 0x0004) ? "UP" : "DOWN");
            LOG_I(TAG, "    AN Complete: %s", (data16 & 0x0020) ? "YES" : "NO");
        }
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "READ-ONLY TEST COMPLETE");
    LOG_I(TAG, "Chip should still be working!");
    LOG_I(TAG, "Try ping between Port 1 & 2 now");
    LOG_I(TAG, "========================================");
}

/**
 * \brief           Periodic check chỉ đọc
 */
void lan9646_readonly_periodic_check(void) {
    static uint32_t counter;
    uint16_t data16;
    uint8_t port;

    counter++;
    if (counter >= 1000) {
        counter = 0;

        LOG_I(TAG, "--- Status Check ---");
        for (port = 1; port <= 4; port++) {
            lan9646_read_reg16(&g_lan, (port * 0x1000) + 0x30, &data16);
            LOG_I(TAG, "Port %d: %s", port,
                  (data16 & 0x0020) ? "UP" : "DOWN");
        }
    }
}
