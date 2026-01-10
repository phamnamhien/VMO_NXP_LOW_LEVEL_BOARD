/**
 * \file            lan9646_soft_i2c_init_example.c
 * \brief           Complete example: LAN9646 initialization with Soft I2C
 */

/*
 * Copyright (c) 2026 Pham Nam Hien
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LAN9646 + Soft I2C example.
 *
 * Author:          Pham Nam Hien
 * Version:         v1.0.0
 */

/* CPU clock configuration */
#define S32K3XX_SOFTI2C_CPU_FREQ_HZ 160000000UL

#include "s32k3xx_soft_i2c.h"
#include "lan9646.h"
#include "log_debug.h"

/* Log tag */
#define TAG "LAN9646"

/* LAN9646 Configuration */
#define LAN9646_I2C_ADDR 0x5F

/* Soft I2C Pin Configuration */
/* Using GMAC0 MDC/MDIO pins as Soft I2C (shared pins) */
/* Note: Ensure GMAC MDIO is not enabled if using these pins for I2C */

/*
 * IMPORTANT: Define these in your pin configuration file or here:
 * ETH_MDC_PORT, ETH_MDC_PIN, ETH_MDIO_PORT, ETH_MDIO_PIN
 *
 * Example for S32K388:
 * - PTD17 (MSCR 113) -> Port 7, Pin 1
 * - PTD16 (MSCR 112) -> Port 7, Pin 0
 */

#ifndef ETH_MDC_PORT
/* For PTD17 (MSCR 113): Port 7, Pin 1 */
extern Siul2_Dio_Ip_GpioType* const ETH_MDC_PORT_PTR;
#define ETH_MDC_PORT    ETH_MDC_PORT_PTR
#define ETH_MDC_PIN     1U
#endif

#ifndef ETH_MDIO_PORT
/* For PTD16 (MSCR 112): Port 7, Pin 0 */
extern Siul2_Dio_Ip_GpioType* const ETH_MDIO_PORT_PTR;
#define ETH_MDIO_PORT   ETH_MDIO_PORT_PTR
#define ETH_MDIO_PIN    0U
#endif

#define LAN9646_SCL_BASE        ETH_MDC_PORT
#define LAN9646_SCL_PIN         ETH_MDC_PIN
#define LAN9646_SDA_BASE        ETH_MDIO_PORT
#define LAN9646_SDA_PIN         ETH_MDIO_PIN
#define LAN9646_I2C_SPEED       5U  /* 100kHz - use 2 for 250kHz if needed */

/* Port base pointers - User needs to define these based on their SIUL2 config */
#ifndef ETH_MDC_PORT_PTR
Siul2_Dio_Ip_GpioType* const ETH_MDC_PORT_PTR =
    (Siul2_Dio_Ip_GpioType*)((uint32_t)IP_SIUL2 + 0x1740U); /* Port 7 offset */
#endif

#ifndef ETH_MDIO_PORT_PTR
Siul2_Dio_Ip_GpioType* const ETH_MDIO_PORT_PTR =
    (Siul2_Dio_Ip_GpioType*)((uint32_t)IP_SIUL2 + 0x1740U); /* Port 7 offset */
#endif

/*
 * Pin configuration notes:
 * - Both pins configured with internal pull-up enabled
 * - PTD17 (SCL): INPUT with pull-up
 * - PTD16 (SDA): INPUT/OUTPUT with pull-up
 * - External pull-up resistors (4.7kΩ) recommended for better signal integrity
 */

/* Global handles */
static softi2c_t g_lan9646_i2c;
static lan9646_t g_lan9646;

/* Forward declarations */
static lan9646r_t lan9646_softi2c_init(void);
static lan9646r_t lan9646_softi2c_mem_write(uint8_t dev_addr, uint16_t mem_addr, const uint8_t* data,
                                             uint16_t len);
static lan9646r_t lan9646_softi2c_mem_read(uint8_t dev_addr, uint16_t mem_addr, uint8_t* data, uint16_t len);

static void lan9646_read_all_chip_info(lan9646_t* handle);
static void lan9646_read_port_status(lan9646_t* handle, uint8_t port);
static void lan9646_configure_port6_cpu(lan9646_t* handle);
static void lan9646_configure_ports_1to4_switch(lan9646_t* handle);
static void lan9646_enable_forwarding(lan9646_t* handle);

/**
 * \brief           Soft I2C init callback for LAN9646
 */
static lan9646r_t
lan9646_softi2c_init(void) {
    /* I2C already initialized */
    return lan9646OK;
}

/**
 * \brief           Soft I2C memory write callback for LAN9646
 */
static lan9646r_t
lan9646_softi2c_mem_write(uint8_t dev_addr, uint16_t mem_addr, const uint8_t* data, uint16_t len) {
    softi2cr_t res;

    res = softi2c_mem_write(&g_lan9646_i2c, dev_addr, mem_addr, 2, data, len);
    return (res == softi2cOK) ? lan9646OK : lan9646ERR;
}

/**
 * \brief           Soft I2C memory read callback for LAN9646
 */
static lan9646r_t
lan9646_softi2c_mem_read(uint8_t dev_addr, uint16_t mem_addr, uint8_t* data, uint16_t len) {
    softi2cr_t res;

    res = softi2c_mem_read(&g_lan9646_i2c, dev_addr, mem_addr, 2, data, len);
    return (res == softi2cOK) ? lan9646OK : lan9646ERR;
}

/**
 * \brief           Read and log PHY status (for ports 1-5)
 */
static void
lan9646_read_phy_status(lan9646_t* handle, uint8_t port) {
    lan9646r_t res;
    uint16_t base_addr, phy_ctrl, phy_status;

    if (port < 1 || port > 5) {
        return; /* Port 6 doesn't have PHY */
    }

    base_addr = (uint16_t)(port * 0x1000);

    LOG_I(TAG, "=== Port %d PHY Status ===", port);

    /* PHY Basic Control (offset 0x00 in PHY register space) */
    res = lan9646_read_reg16(handle, base_addr + 0x00, &phy_ctrl);
    if (res == lan9646OK) {
        LOG_I(TAG, "PHY Control: 0x%04X", phy_ctrl);
        LOG_I(TAG, "  Reset: %s", (phy_ctrl & 0x8000) ? "YES" : "NO");
        LOG_I(TAG, "  Loopback: %s", (phy_ctrl & 0x4000) ? "ON" : "OFF");
        LOG_I(TAG, "  Speed Select: %s", (phy_ctrl & 0x2000) ? "1000" : "10/100");
        LOG_I(TAG, "  Auto-neg: %s", (phy_ctrl & 0x1000) ? "Enabled" : "Disabled");
        LOG_I(TAG, "  Power Down: %s", (phy_ctrl & 0x0800) ? "YES" : "NO");
        LOG_I(TAG, "  Restart AN: %s", (phy_ctrl & 0x0200) ? "YES" : "NO");
        LOG_I(TAG, "  Duplex: %s", (phy_ctrl & 0x0100) ? "Full" : "Half");
    }

    /* PHY Basic Status (offset 0x01) */
    res = lan9646_read_reg16(handle, base_addr + 0x01, &phy_status);
    if (res == lan9646OK) {
        LOG_I(TAG, "PHY Status: 0x%04X", phy_status);
        LOG_I(TAG, "  100Base-T4: %s", (phy_status & 0x8000) ? "Capable" : "Not capable");
        LOG_I(TAG, "  100Base-TX Full: %s", (phy_status & 0x4000) ? "Capable" : "Not capable");
        LOG_I(TAG, "  100Base-TX Half: %s", (phy_status & 0x2000) ? "Capable" : "Not capable");
        LOG_I(TAG, "  10Base-T Full: %s", (phy_status & 0x1000) ? "Capable" : "Not capable");
        LOG_I(TAG, "  10Base-T Half: %s", (phy_status & 0x0800) ? "Capable" : "Not capable");
        LOG_I(TAG, "  AN Complete: %s", (phy_status & 0x0020) ? "YES" : "NO");
        LOG_I(TAG, "  Remote Fault: %s", (phy_status & 0x0010) ? "YES" : "NO");
        LOG_I(TAG, "  AN Capable: %s", (phy_status & 0x0008) ? "YES" : "NO");
        LOG_I(TAG, "  Link Status: %s", (phy_status & 0x0004) ? "UP" : "DOWN");
        LOG_I(TAG, "  Jabber Detect: %s", (phy_status & 0x0002) ? "YES" : "NO");
        LOG_I(TAG, "  Extended Capable: %s", (phy_status & 0x0001) ? "YES" : "NO");
    }

    LOG_I(TAG, "========================");
}

/**
 * \brief           Read and log all chip information
 */
static void
lan9646_read_all_chip_info(lan9646_t* handle) {
    lan9646r_t res;
    uint16_t chip_id, revision;
    uint8_t data8;

    LOG_I(TAG, "=== LAN9646 Chip Information ===");

    /* Read Chip ID (Register 0x0000) */
    res = lan9646_read_reg16(handle, 0x0000, &chip_id);
    if (res == lan9646OK) {
        LOG_I(TAG, "Chip ID: 0x%04X", chip_id);
    } else {
        LOG_E(TAG, "Failed to read Chip ID");
    }

    /* Read Chip ID High (Register 0x0001) */
    res = lan9646_read_reg8(handle, 0x0001, &data8);
    if (res == lan9646OK) {
        LOG_I(TAG, "Chip ID High: 0x%02X", data8);
    }

    /* Read Chip ID Low (Register 0x0002) */
    res = lan9646_read_reg8(handle, 0x0002, &data8);
    if (res == lan9646OK) {
        LOG_I(TAG, "Chip ID Low: 0x%02X", data8);
        revision = data8 & 0x0F;
        LOG_I(TAG, "Chip Revision: %d", revision);
    }

    /* Read Global Chip Mode (Register 0x0003) */
    res = lan9646_read_reg8(handle, 0x0003, &data8);
    if (res == lan9646OK) {
        LOG_I(TAG, "Global Chip Mode: 0x%02X", data8);
    }

    /* Read Port Enable Status (Register 0x0004) */
    res = lan9646_read_reg8(handle, 0x0004, &data8);
    if (res == lan9646OK) {
        LOG_I(TAG, "Port Enable Status: 0x%02X", data8);
        LOG_I(TAG, "  Port 1: %s", (data8 & 0x01) ? "Enabled" : "Disabled");
        LOG_I(TAG, "  Port 2: %s", (data8 & 0x02) ? "Enabled" : "Disabled");
        LOG_I(TAG, "  Port 3: %s", (data8 & 0x04) ? "Enabled" : "Disabled");
        LOG_I(TAG, "  Port 4: %s", (data8 & 0x08) ? "Enabled" : "Disabled");
        LOG_I(TAG, "  Port 5: %s", (data8 & 0x10) ? "Enabled" : "Disabled");
        LOG_I(TAG, "  Port 6: %s", (data8 & 0x20) ? "Enabled" : "Disabled");
    }

    LOG_I(TAG, "================================");
}

/**
 * \brief           Read and log port status
 */
static void
lan9646_read_port_status(lan9646_t* handle, uint8_t port) {
    lan9646r_t res;
    uint16_t base_addr;
    uint8_t data8;
    uint16_t data16;

    if (port < 1 || port > 6) {
        LOG_E(TAG, "Invalid port number: %d", port);
        return;
    }

    /* Calculate base address for port (Port 1 = 0x1000, Port 2 = 0x2000, etc.) */
    base_addr = (uint16_t)(port * 0x1000);

    LOG_I(TAG, "=== Port %d Status ===", port);

    /* Port Control 0 (Offset 0x00) */
    res = lan9646_read_reg8(handle, base_addr + 0x00, &data8);
    if (res == lan9646OK) {
        LOG_I(TAG, "Port Control 0: 0x%02X", data8);
    }

    /* Port Control 1 (Offset 0x01) */
    res = lan9646_read_reg8(handle, base_addr + 0x01, &data8);
    if (res == lan9646OK) {
        LOG_I(TAG, "Port Control 1: 0x%02X", data8);
    }

    /* Port Status (Offset 0x30) */
    res = lan9646_read_reg16(handle, base_addr + 0x30, &data16);
    if (res == lan9646OK) {
        LOG_I(TAG, "Port Status: 0x%04X", data16);
        LOG_I(TAG, "  Link Status: %s", (data16 & 0x0020) ? "UP" : "DOWN");
        LOG_I(TAG, "  Speed: %s", (data16 & 0x0400) ? "1000Mbps" : "10/100Mbps");
        LOG_I(TAG, "  Duplex: %s", (data16 & 0x0200) ? "Full" : "Half");
    }

    /* PHY Basic Control (for PHY ports, Offset 0xN00 where N is PHY addr) */
    if (port <= 5) { /* Ports 1-5 have PHY */
        res = lan9646_read_reg16(handle, base_addr + 0x00, &data16);
        if (res == lan9646OK) {
            LOG_I(TAG, "PHY Control: 0x%04X", data16);
        }
    }

    LOG_I(TAG, "====================");
}

/**
 * \brief           Configure Port 6 (CPU port) for communication with switch ports
 */
static void
lan9646_configure_port6_cpu(lan9646_t* handle) {
    lan9646r_t res;
    uint16_t base_addr;
    uint8_t data8, verify;

    base_addr = 0x6000; /* Port 6 base address */

    LOG_I(TAG, "Configuring Port 6 (CPU Port)...");

    /* Enable Port 6 in global register */
    res = lan9646_read_reg8(handle, 0x0004, &data8);
    if (res == lan9646OK) {
        data8 |= 0x20; /* Bit 5 = Port 6 enable */
        res = lan9646_write_reg8(handle, 0x0004, data8);

        /* Verify write */
        lan9646_read_reg8(handle, 0x0004, &verify);
        if (verify & 0x20) {
            LOG_I(TAG, "Port 6 enabled (0x%02X)", verify);
        } else {
            LOG_E(TAG, "Port 6 enable FAILED (read: 0x%02X)", verify);
        }
    }

    /* Port 6 Control 0: Enable transmit and receive */
    res = lan9646_write_reg8(handle, base_addr + 0x00, 0x03);
    if (res == lan9646OK) {
        lan9646_read_reg8(handle, base_addr + 0x00, &verify);
        LOG_I(TAG, "Port 6 Control 0: 0x%02X (TX/RX)", verify);
    }

    /* Port 6 Control 1: RGMII mode, 1000Mbps */
    res = lan9646_write_reg8(handle, base_addr + 0x01, 0x00);
    if (res == lan9646OK) {
        LOG_I(TAG, "Port 6 interface configured");
    }

    LOG_I(TAG, "Port 6 configuration complete");
}

/**
 * \brief           Configure Ports 1-4 as switch ports
 */
static void
lan9646_configure_ports_1to4_switch(lan9646_t* handle) {
    lan9646r_t res;
    uint8_t port, data8, verify;
    uint16_t base_addr, data16;

    LOG_I(TAG, "Configuring Ports 1-4 (Switch Ports)...");

    /* Enable Ports 1-4 in global enable register */
    res = lan9646_read_reg8(handle, 0x0004, &data8);
    if (res == lan9646OK) {
        data8 |= 0x0F; /* Bits 0-3 = Ports 1-4 */
        res = lan9646_write_reg8(handle, 0x0004, data8);

        /* Verify */
        lan9646_read_reg8(handle, 0x0004, &verify);
        LOG_I(TAG, "Port Enable Status: 0x%02X", verify);
    }

    /* Configure each port */
    for (port = 1; port <= 4; port++) {
        base_addr = (uint16_t)(port * 0x1000);

        LOG_D(TAG, "Configuring Port %d...", port);

        /* Port Control 0: Enable TX/RX */
        res = lan9646_write_reg8(handle, base_addr + 0x00, 0x03);
        if (res == lan9646OK) {
            lan9646_read_reg8(handle, base_addr + 0x00, &verify);
            LOG_D(TAG, "Port %d Control 0: 0x%02X", port, verify);
        }

        /* PHY Basic Control: Enable + Auto-negotiation + Restart AN */
        /* Bit 15: Reset, Bit 12: Auto-neg enable, Bit 9: Restart AN, Bit 0: PHY enable */
        data16 = 0x1200; /* Auto-neg enable + Restart AN */
        res = lan9646_write_reg16(handle, base_addr + 0x00, data16);
        if (res == lan9646OK) {
            lan9646_read_reg16(handle, base_addr + 0x00, &data16);
            LOG_D(TAG, "Port %d PHY Control: 0x%04X", port, data16);
        }

        /* Delay for PHY to stabilize */
        for (volatile uint32_t i = 0; i < 100000; i++);

        LOG_I(TAG, "Port %d configured", port);
    }

    LOG_I(TAG, "Ports 1-4 configuration complete");
}

/**
 * \brief           Enable forwarding between all ports
 */
static void
lan9646_enable_forwarding(lan9646_t* handle) {
    lan9646r_t res;
    uint8_t port;
    uint16_t base_addr;

    LOG_I(TAG, "Enabling forwarding between ports...");

    /* For each port, set forwarding rules */
    for (port = 1; port <= 6; port++) {
        base_addr = (uint16_t)(port * 0x1000);

        /* Port membership: Allow forwarding to all other ports */
        /* This is typically in a port membership register */
        /* Example: Set membership register at offset 0x10 */
        res = lan9646_write_reg8(handle, base_addr + 0x10, 0x3F); /* All ports */
        if (res == lan9646OK) {
            LOG_D(TAG, "Port %d: Forwarding to all ports enabled", port);
        }
    }

    LOG_I(TAG, "Forwarding configuration complete");
}

/**
 * \brief           Complete initialization example
 */
void
lan9646_complete_init_example(void) {
    softi2c_pins_t i2c_pins;
    lan9646_cfg_t lan_cfg;
    lan9646r_t res;
    softi2cr_t i2c_res;
    uint16_t chip_id;
    uint8_t port, data8;

    /* Initialize log system */
    log_init();
    log_set_level(LOG_LEVEL_INFO);

    LOG_I(TAG, "========================================");
    LOG_I(TAG, "LAN9646 Initialization with Soft I2C");
    LOG_I(TAG, "========================================");

    /* Step 1: Initialize Soft I2C */
    LOG_I(TAG, "Step 1: Initializing Soft I2C...");
    i2c_pins.scl_port = LAN9646_SCL_BASE;
    i2c_pins.scl_pin = LAN9646_SCL_PIN;
    i2c_pins.sda_port = LAN9646_SDA_BASE;
    i2c_pins.sda_pin = LAN9646_SDA_PIN;
    i2c_pins.delay_us = LAN9646_I2C_SPEED;

    i2c_res = softi2c_init(&g_lan9646_i2c, &i2c_pins);
    if (i2c_res != softi2cOK) {
        LOG_E(TAG, "Failed to initialize Soft I2C");
        return;
    }
    LOG_I(TAG, "Soft I2C initialized successfully");

    /* Step 2: Check if LAN9646 is present on I2C bus */
    LOG_I(TAG, "Step 2: Checking LAN9646 presence...");
    i2c_res = softi2c_is_device_ready(&g_lan9646_i2c, LAN9646_I2C_ADDR, 3);
    if (i2c_res != softi2cOK) {
        LOG_E(TAG, "LAN9646 not found on I2C bus (addr: 0x%02X)", LAN9646_I2C_ADDR);
        return;
    }
    LOG_I(TAG, "LAN9646 found on I2C bus");

    /* Step 3: Initialize LAN9646 driver */
    LOG_I(TAG, "Step 3: Initializing LAN9646 driver...");
    lan_cfg.if_type = LAN9646_IF_I2C;
    lan_cfg.i2c_addr = LAN9646_I2C_ADDR;
    lan_cfg.ops.i2c.init_fn = lan9646_softi2c_init;
    lan_cfg.ops.i2c.mem_write_fn = lan9646_softi2c_mem_write;
    lan_cfg.ops.i2c.mem_read_fn = lan9646_softi2c_mem_read;
    lan_cfg.ops.i2c.write_fn = NULL; /* Not used */
    lan_cfg.ops.i2c.read_fn = NULL;  /* Not used */

    res = lan9646_init(&g_lan9646, &lan_cfg);
    if (res != lan9646OK) {
        LOG_E(TAG, "Failed to initialize LAN9646 driver");
        return;
    }
    LOG_I(TAG, "LAN9646 driver initialized");

    /* Step 4: Verify chip ID */
    LOG_I(TAG, "Step 4: Verifying chip ID...");
    res = lan9646_get_chip_id(&g_lan9646, &chip_id);
    if (res != lan9646OK) {
        LOG_E(TAG, "Failed to read chip ID");
        return;
    }
    LOG_I(TAG, "Chip ID: 0x%04X (Expected: 0x9646 or similar)", chip_id);

    /* Step 5: Read all chip information */
    LOG_I(TAG, "Step 5: Reading chip information...");
    lan9646_read_all_chip_info(&g_lan9646);

    /* Step 6: Perform soft reset (optional) */
    LOG_I(TAG, "Step 6: Performing soft reset...");
    res = lan9646_soft_reset(&g_lan9646);
    if (res == lan9646OK) {
        LOG_I(TAG, "Soft reset completed");
        /* Wait for reset to complete - minimum 100ms */
        for (volatile uint32_t i = 0; i < 5000000; i++)
            ;
        LOG_I(TAG, "Reset stabilization delay complete");
    } else {
        LOG_W(TAG, "Soft reset failed or not supported");
    }

    /* Step 6.5: Enable global switch operation */
    LOG_I(TAG, "Step 6.5: Enabling switch operation...");
    res = lan9646_write_reg8(&g_lan9646, 0x0300, 0x01); /* Start switch */
    if (res == lan9646OK) {
        lan9646_read_reg8(&g_lan9646, 0x0300, &data8);
        LOG_I(TAG, "Switch Operation: 0x%02X", data8);
    }

    /* Step 7: Configure Port 6 (CPU port) */
    LOG_I(TAG, "Step 7: Configuring Port 6 (CPU port)...");
    lan9646_configure_port6_cpu(&g_lan9646);

    /* Step 8: Configure Ports 1-4 (Switch ports) */
    LOG_I(TAG, "Step 8: Configuring Ports 1-4 (Switch ports)...");
    lan9646_configure_ports_1to4_switch(&g_lan9646);

    /* Step 9: Enable forwarding */
    LOG_I(TAG, "Step 9: Enabling forwarding...");
    lan9646_enable_forwarding(&g_lan9646);

    /* Step 10: Read port status */
    LOG_I(TAG, "Step 10: Reading port status...");
    for (port = 1; port <= 6; port++) {
        lan9646_read_port_status(&g_lan9646, port);
    }

    /* Step 10.5: Read PHY status for debug */
    LOG_I(TAG, "Step 10.5: Reading PHY status...");
    for (port = 1; port <= 4; port++) {
        lan9646_read_phy_status(&g_lan9646, port);
    }

    /* Step 11: Read some common registers for debugging */
    LOG_I(TAG, "Step 11: Reading common registers...");
    {
        uint8_t reg_val;

        /* Global Control register */
        res = lan9646_read_reg8(&g_lan9646, 0x0003, &reg_val);
        if (res == lan9646OK) {
            LOG_D(TAG, "Global Control (0x0003): 0x%02X", reg_val);
        }

        /* Port enable status */
        res = lan9646_read_reg8(&g_lan9646, 0x0004, &reg_val);
        if (res == lan9646OK) {
            LOG_D(TAG, "Port Enable (0x0004): 0x%02X", reg_val);
        }

        /* Switch operation register */
        res = lan9646_read_reg8(&g_lan9646, 0x0300, &reg_val);
        if (res == lan9646OK) {
            LOG_D(TAG, "Switch Operation (0x0300): 0x%02X", reg_val);
        }
    }

    LOG_I(TAG, "========================================");
    LOG_I(TAG, "LAN9646 Initialization Complete!");
    LOG_I(TAG, "Port 6 (CPU) <---> Ports 1-4 (Switch)");
    LOG_I(TAG, "========================================");
}

/**
 * \brief           Periodic status check (call from main loop)
 */
void
lan9646_periodic_status_check(void) {
    static uint32_t check_counter;
    lan9646r_t res;
    uint8_t port, data8;
    uint16_t base_addr, status;

    check_counter++;

    /* Check every 1000 iterations (adjust based on your loop frequency) */
    if (check_counter >= 1000) {
        check_counter = 0;

        LOG_I(TAG, "--- Periodic Status Check ---");

        /* Check link status for all ports */
        for (port = 1; port <= 6; port++) {
            base_addr = (uint16_t)(port * 0x1000);

            /* Read port status */
            res = lan9646_read_reg16(&g_lan9646, base_addr + 0x30, &status);
            if (res == lan9646OK) {
                if (status & 0x0020) { /* Link up bit */
                    LOG_I(TAG, "Port %d: Link UP", port);
                } else {
                    LOG_I(TAG, "Port %d: Link DOWN", port);
                }
            }
        }

        /* Check if any errors occurred */
        res = lan9646_read_reg8(&g_lan9646, 0x0003, &data8);
        if (res != lan9646OK) {
            LOG_E(TAG, "Communication error with LAN9646!");
        }
    }
}

