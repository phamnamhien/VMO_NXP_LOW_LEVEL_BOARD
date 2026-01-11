/**
 * \file            main.c
 * \brief           LAN9646 GMAC Diagnostics & Port Monitor
 * \note            Uses lan9646 low-level + lan9646_switch high-level API
 */

#include "Mcal.h"
#include "Clock_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "IntCtrl_Ip.h"

#include "lan9646.h"
#include "lan9646_switch.h"
#include "s32k3xx_soft_i2c.h"
#include "systick.h"
#include "log_debug.h"

#define TAG "MAIN"

/*===========================================================================*/
/*                           CONFIGURATION                                    */
/*===========================================================================*/

#define LAN9646_SCL_PORT        ETH_MDC_PORT
#define LAN9646_SCL_PIN         ETH_MDC_PIN
#define LAN9646_SDA_PORT        ETH_MDIO_PORT
#define LAN9646_SDA_PIN         ETH_MDIO_PIN
#define LAN9646_I2C_SPEED       5U

/*===========================================================================*/
/*                           PRIVATE VARIABLES                                */
/*===========================================================================*/

static lan9646_t g_lan9646;
static softi2c_t g_i2c;

/*===========================================================================*/
/*                          I2C CALLBACKS                                     */
/*===========================================================================*/

static lan9646r_t i2c_init_cb(void) {
    softi2c_pins_t pins = {
        .scl_port = LAN9646_SCL_PORT,
        .scl_pin  = LAN9646_SCL_PIN,
        .sda_port = LAN9646_SDA_PORT,
        .sda_pin  = LAN9646_SDA_PIN,
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
/*                          PORT 6 CONFIG (RGMII)                             */
/*===========================================================================*/

static void print_port6_config(void) {
    uint8_t ctrl0, ctrl1;

    /* Read XMII Control registers */
    if (lan9646_read_reg8(&g_lan9646, LAN9646_REG_PORT_XMII_CTRL0(6), &ctrl0) != lan9646OK ||
        lan9646_read_reg8(&g_lan9646, LAN9646_REG_PORT_XMII_CTRL1(6), &ctrl1) != lan9646OK) {
        LOG_E(TAG, "Failed to read Port 6 config!");
        return;
    }

    /* Decode interface type */
    const char* if_str;
    bool is_1g = (ctrl1 & LAN9646_XMII_SPEED_1000) == 0;  /* 0 = 1000Mbps */
    if (is_1g) {
        if_str = "RGMII";
    } else {
        if_str = (ctrl1 & LAN9646_XMII_MII_RMII_MODE) ? "RMII" : "MII";
    }

    /* Decode speed */
    const char* spd_str;
    if (is_1g) {
        spd_str = "1000 Mbps";
    } else {
        spd_str = (ctrl0 & LAN9646_XMII_SPEED_100) ? "100 Mbps" : "10 Mbps";
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "========== GMAC0 (Port 6) Configuration ==========");
    LOG_I(TAG, "Interface:   %s", if_str);
    LOG_I(TAG, "Speed:       %s", spd_str);
    LOG_I(TAG, "Duplex:      %s", (ctrl0 & LAN9646_XMII_DUPLEX) ? "Full" : "Half");

    LOG_I(TAG, "");
    LOG_I(TAG, "--- RGMII Delay (Critical for S32K388 GMAC) ---");
    LOG_I(TAG, "TX Egress Delay:   %s", (ctrl1 & LAN9646_XMII_RGMII_TX_DLY_EN) ? "ON +1.5ns" : "OFF");
    LOG_I(TAG, "RX Ingress Delay:  %s", (ctrl1 & LAN9646_XMII_RGMII_RX_DLY_EN) ? "ON +1.5ns" : "OFF");

    LOG_I(TAG, "");
    LOG_I(TAG, "--- Flow Control ---");
    LOG_I(TAG, "TX Flow Ctrl: %s", (ctrl0 & LAN9646_XMII_TX_FLOW_EN) ? "Enabled" : "Disabled");
    LOG_I(TAG, "RX Flow Ctrl: %s", (ctrl0 & LAN9646_XMII_RX_FLOW_EN) ? "Enabled" : "Disabled");
    LOG_I(TAG, "===================================================");
}

/*===========================================================================*/
/*                          ALL PORTS STATUS                                  */
/*===========================================================================*/

static void print_all_ports_status(void) {
    lan9646_port_status_t status;
    const uint8_t ports[] = {1, 2, 3, 4, 6};
    const char* port_names[] = {"Port 1 (PHY)", "Port 2 (PHY)", "Port 3 (PHY)",
                                "Port 4 (PHY)", "Port 6 (GMAC)"};

    LOG_I(TAG, "");
    LOG_I(TAG, "========== All Ports Initial Status ==========");

    for (int i = 0; i < 5; i++) {
        uint8_t port = ports[i];

        if (lan9646_switch_get_port_status(&g_lan9646, port, &status) != lan9646OK) {
            LOG_E(TAG, "%s: READ ERROR", port_names[i]);
            continue;
        }

        const char* spd_str;
        switch (status.speed) {
            case LAN9646_SPEED_1000M: spd_str = "1000M"; break;
            case LAN9646_SPEED_100M:  spd_str = "100M"; break;
            case LAN9646_SPEED_10M:   spd_str = "10M"; break;
            default:                  spd_str = "---"; break;
        }

        LOG_I(TAG, "%s: %s  %s  %s",
              port_names[i],
              status.link_up ? "UP  " : "DOWN",
              spd_str,
              status.link_up ? (status.duplex == LAN9646_DUPLEX_FULL ? "Full" : "Half") : "---");
    }

    LOG_I(TAG, "===============================================");
}

/*===========================================================================*/
/*                          PORT MONITOR                                      */
/*===========================================================================*/

static void print_port_monitor(uint8_t port) {
    lan9646_port_status_t status;
    lan9646_mib_simple_t mib;

    if (lan9646_switch_get_port_status(&g_lan9646, port, &status) != lan9646OK) {
        LOG_E(TAG, "P%d: READ ERROR", port);
        return;
    }

    if (!status.link_up) {
        LOG_I(TAG, "P%d: DOWN", port);
        return;
    }

    /* Read MIB counters */
    if (lan9646_switch_read_mib_simple(&g_lan9646, port, &mib) != lan9646OK) {
        LOG_E(TAG, "P%d: MIB ERROR", port);
        return;
    }

    const char* spd_str;
    switch (status.speed) {
        case LAN9646_SPEED_1000M: spd_str = "1G"; break;
        case LAN9646_SPEED_100M:  spd_str = "100M"; break;
        case LAN9646_SPEED_10M:   spd_str = "10M"; break;
        default:                  spd_str = "??"; break;
    }

    LOG_I(TAG, "P%d: %s %s RX=%lu TX=%lu",
          port, spd_str,
          (status.duplex == LAN9646_DUPLEX_FULL) ? "FD" : "HD",
          (unsigned long)mib.rx_packets,
          (unsigned long)mib.tx_packets);
}

/*===========================================================================*/
/*                          PORT 6 RGMII 1G CONFIG                            */
/*===========================================================================*/

static lan9646r_t configure_port6_rgmii_1g(void) {
    uint8_t ctrl0, ctrl1;

    LOG_I(TAG, "Configuring Port 6 for RGMII 1G...");

    /* XMII_CTRL0: Full duplex, Flow control enabled */
    ctrl0 = LAN9646_XMII_DUPLEX | LAN9646_XMII_TX_FLOW_EN | LAN9646_XMII_RX_FLOW_EN;

    /* XMII_CTRL1: 1000Mbps (bit6=0), TX delay ON, RX delay OFF */
    ctrl1 = LAN9646_XMII_RGMII_TX_DLY_EN;  /* TX delay = ON (+1.5ns) */

    if (lan9646_write_reg8(&g_lan9646, LAN9646_REG_PORT_XMII_CTRL0(6), ctrl0) != lan9646OK ||
        lan9646_write_reg8(&g_lan9646, LAN9646_REG_PORT_XMII_CTRL1(6), ctrl1) != lan9646OK) {
        LOG_E(TAG, "Failed to configure Port 6!");
        return lan9646ERR;
    }

    LOG_I(TAG, "Port 6 configured: RGMII 1G, Full Duplex, TX_DLY=ON");
    return lan9646OK;
}

/*===========================================================================*/
/*                          MAIN                                             */
/*===========================================================================*/

int main(void) {
    uint16_t chip_id;
    uint8_t revision;

    /* MCU Init */
    Clock_Ip_Init(&Clock_Ip_aClockConfig[0]);
    SysTick_Init();
    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals,
                       g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);
    IntCtrl_Ip_Init(&IntCtrlConfig_0);
    log_init();

    LOG_I(TAG, "");
    LOG_I(TAG, "######################################");
    LOG_I(TAG, "#  LAN9646 GMAC Diagnostics Tool    #");
    LOG_I(TAG, "######################################");

    /* Configure LAN9646 low-level */
    lan9646_cfg_t cfg = {
        .if_type = LAN9646_IF_I2C,
        .i2c_addr = LAN9646_I2C_ADDR_DEFAULT,
        .ops.i2c = {
            .init_fn      = i2c_init_cb,
            .write_fn     = i2c_write_cb,
            .read_fn      = i2c_read_cb,
            .mem_write_fn = i2c_mem_write_cb,
            .mem_read_fn  = i2c_mem_read_cb
        }
    };

    LOG_I(TAG, "Initializing LAN9646...");
    if (lan9646_init(&g_lan9646, &cfg) != lan9646OK) {
        LOG_E(TAG, "LAN9646 init FAILED!");
        while(1);
    }

    /* Verify chip ID */
    if (lan9646_get_chip_id(&g_lan9646, &chip_id, &revision) != lan9646OK) {
        LOG_E(TAG, "Failed to read chip ID!");
        while(1);
    }

    if (chip_id != LAN9646_CHIP_ID) {
        LOG_E(TAG, "Chip ID mismatch: 0x%04X (expected 0x%04X)", chip_id, LAN9646_CHIP_ID);
        while(1);
    }

    LOG_I(TAG, "Chip: 0x%04X Rev:%d", chip_id, revision);

    /* Configure Port 6 for RGMII 1G */
    configure_port6_rgmii_1g();

    /* Print Port 6 configuration */
    print_port6_config();

    /* Print clock configuration */
    lan9646_switch_print_clock_config(&g_lan9646);

    /* Print all ports initial status */
    print_all_ports_status();

    LOG_I(TAG, "");
    LOG_I(TAG, "--- Port Monitor (5s interval) ---");
    LOG_I(TAG, "Format: Port: Speed Duplex RX TX");

    /* Main loop - monitor all ports */
    while (1) {
        SysTick_DelayMs(5000);
        LOG_I(TAG, "");
        print_port_monitor(1);
        print_port_monitor(2);
        print_port_monitor(3);
        print_port_monitor(4);
        print_port_monitor(6);
    }

    return 0;
}

