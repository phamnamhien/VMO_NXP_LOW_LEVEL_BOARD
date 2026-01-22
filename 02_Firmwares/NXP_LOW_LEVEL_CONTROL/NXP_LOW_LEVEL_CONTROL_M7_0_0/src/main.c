/**
 * \file            main.c
 * \brief           LAN9646 + Eth + FreeRTOS
 */

#include <string.h>

#include "S32K388.h"
#include "Mcal.h"
#include "Mcu.h"
#include "Port.h"
#include "OsIf.h"
#include "osif_rtd_port.h"
#include "Gpt.h"
#include "Platform.h"
#include "ethif_port.h"
#include "Eth_43_GMAC.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lan9646.h"
#include "lan9646_switch.h"
#include "s32k3xx_soft_i2c.h"
#include "log_debug.h"

#define TAG "MAIN"

#define LAN9646_SCL_CHANNEL     DioConf_DioChannel_SCL_CH
#define LAN9646_SDA_CHANNEL     DioConf_DioChannel_SDA_CH
#define LAN9646_I2C_SPEED       5U
#define ETH_CTRL_IDX            0U

static lan9646_t g_lan9646;
static softi2c_t g_i2c;

/* I2C Callbacks */
static lan9646r_t i2c_init_cb(void) {
    softi2c_pins_t pins = {
        .scl_channel = LAN9646_SCL_CHANNEL,
        .sda_channel = LAN9646_SDA_CHANNEL,
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

/* Configure LAN9646 Port 6 for RGMII 1G */
static lan9646r_t configure_port6_rgmii_1g(void) {
    uint8_t ctrl0, ctrl1, port_ctrl;

    LOG_I(TAG, "Configuring Port 6 for RGMII 1G...");

    ctrl0 = 0x68;
    ctrl1 = 0x08;

    LOG_I(TAG, "  Writing XMII_CTRL0 = 0x%02X", ctrl0);
    if (lan9646_write_reg8(&g_lan9646, 0x6300, ctrl0) != lan9646OK) {
        LOG_E(TAG, "  Failed to write XMII_CTRL0!");
        return lan9646ERR;
    }

    LOG_I(TAG, "  Writing XMII_CTRL1 = 0x%02X", ctrl1);
    if (lan9646_write_reg8(&g_lan9646, 0x6301, ctrl1) != lan9646OK) {
        LOG_E(TAG, "  Failed to write XMII_CTRL1!");
        return lan9646ERR;
    }

    lan9646_read_reg8(&g_lan9646, 0x6000, &port_ctrl);
    port_ctrl |= 0x03;
    lan9646_write_reg8(&g_lan9646, 0x6000, port_ctrl);

    LOG_I(TAG, "  Port 6 config OK: RGMII 1G, Full Duplex, TX_DLY=ON");
    return lan9646OK;
}

/* Debug all ports */
static void debug_all_ports(void) {
    uint8_t status;
    lan9646_mib_simple_t mib;
    const uint8_t ports[] = {1, 2, 3, 4, 5, 6};

    for (int i = 0; i < 6; i++) {
        uint16_t base = (uint16_t)ports[i] << 12;

        lan9646_read_reg8(&g_lan9646, base | 0x0030, &status);
        lan9646_switch_read_mib_simple(&g_lan9646, ports[i], &mib);

        LOG_I(TAG, "P%d: Link=%d Speed=%d | RX=%lu TX=%lu",
              ports[i],
              (status >> 5) & 1,
              (status >> 3) & 3,
              (unsigned long)mib.rx_packets,
              (unsigned long)mib.tx_packets);
    }
}

/* Debug switch config */
static void debug_switch_config(void) {
    uint8_t val8;
    uint32_t val32;

    LOG_I(TAG, "");
    LOG_I(TAG, "========== Switch Global Config ==========");

    if (lan9646_read_reg8(&g_lan9646, 0x0300, &val8) == lan9646OK) {
        LOG_I(TAG, "SW_OPERATION (0x0300) = 0x%02X", val8);
        LOG_I(TAG, "  [0] Start Switch: %s", (val8 & 0x01) ? "YES" : "NO");
    }

    const uint8_t ports[] = {1, 2, 6};
    for (int i = 0; i < 3; i++) {
        uint16_t base = (uint16_t)ports[i] << 12;

        if (lan9646_read_reg32(&g_lan9646, base | 0x0A04, &val32) == lan9646OK) {
            LOG_I(TAG, "P%d VLAN_MEMBER = 0x%02lX [P6=%d P2=%d P1=%d]",
                  ports[i], (unsigned long)(val32 & 0x7F),
                  (int)((val32 >> 5) & 1),
                  (int)((val32 >> 1) & 1),
                  (int)(val32 & 1));
        }

        if (lan9646_read_reg8(&g_lan9646, base | 0x0B04, &val8) == lan9646OK) {
            LOG_I(TAG, "P%d MSTP_STATE = 0x%02X [TxEn=%d RxEn=%d]",
                  ports[i], val8,
                  (int)((val8 >> 2) & 1),
                  (int)((val8 >> 1) & 1));
        }
    }
    LOG_I(TAG, "===========================================");
}

/* Debug Port 6 registers */
static void debug_port6_registers(void) {
    uint8_t ctrl0, ctrl1, status;

    LOG_I(TAG, "");
    LOG_I(TAG, "========== Port 6 Registers ==========");

    if (lan9646_read_reg8(&g_lan9646, 0x6300, &ctrl0) == lan9646OK) {
        LOG_I(TAG, "XMII_CTRL0 = 0x%02X [Duplex=%s, Speed100=%d]",
              ctrl0, (ctrl0 & 0x40) ? "Full" : "Half", (ctrl0 >> 4) & 1);
    }

    if (lan9646_read_reg8(&g_lan9646, 0x6301, &ctrl1) == lan9646OK) {
        LOG_I(TAG, "XMII_CTRL1 = 0x%02X [Speed1000=%s, TxDly=%d, RxDly=%d]",
              ctrl1, (ctrl1 & 0x40) ? "10/100" : "1000", (ctrl1 >> 3) & 1, (ctrl1 >> 4) & 1);
    }

    if (lan9646_read_reg8(&g_lan9646, 0x6030, &status) == lan9646OK) {
        uint8_t speed = (status >> 3) & 0x03;
        LOG_I(TAG, "PORT_STATUS = 0x%02X [Speed=%s, Duplex=%s, Link=%d]",
              status,
              speed == 0 ? "10M" : (speed == 1 ? "100M" : "1000M"),
              (status & 0x04) ? "Full" : "Half",
              (status >> 5) & 1);
    }
    LOG_I(TAG, "=======================================");
}

/* Main task */
static void mainLoopTask(void *pvParameters) {
    (void)pvParameters;
    uint32_t loop_count = 0;

    LOG_I(TAG, "Task started");

    vTaskDelay(pdMS_TO_TICKS(500));

    /* Check GMAC mode */
    Eth_ModeType mode;
    Eth_43_GMAC_GetControllerMode(ETH_CTRL_IDX, &mode);
    LOG_I(TAG, "GMAC mode: %d (1=DOWN, 2=ACTIVE)", mode);

    /* Get MAC address */
    uint8_t macAddr[6];
    Eth_43_GMAC_GetPhysAddr(ETH_CTRL_IDX, macAddr);
    LOG_I(TAG, "GMAC MAC: %02X:%02X:%02X:%02X:%02X:%02X",
          macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

    /* Debug after GMAC init */
    debug_port6_registers();

    LOG_I(TAG, "");
    LOG_I(TAG, "Ready - Monitoring...");

    /* Main Loop */
    while (1) {
        loop_count++;

        if ((loop_count % 10) == 0) {
            LOG_I(TAG, "");
            LOG_I(TAG, "=== Loop %lu ===", (unsigned long)loop_count);
            debug_all_ports();
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* FreeRTOS hooks */
void vApplicationMallocFailedHook(void) {
    LOG_E(TAG, "Malloc failed!");
    while(1);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    (void)pxTask;
    LOG_E(TAG, "Stack overflow: %s", pcTaskName);
    while(1);
}

void vApplicationIdleHook(void) {
}

void vApplicationTickHook(void) {
}

/* Main */
int main(void) {
    uint16_t chip_id;
    uint8_t revision;

    /* RGMII: Bypass MUX_7 */
    IP_DCM_GPR->DCMRWF3 |= DCM_GPR_DCMRWF3_MAC_RX_CLK_MUX_BYPASS(1U);

    /* Initialize MCU */
    Mcu_Init(NULL_PTR);
    Mcu_InitClock(McuClockSettingConfig_0);

#if (MCU_NO_PLL == STD_OFF)
    while (MCU_PLL_LOCKED != Mcu_GetPllStatus()) {}
    Mcu_DistributePllClock();
#endif

    Mcu_SetMode(McuModeSettingConf_0);

    OsIf_Init(NULL_PTR);
    Port_Init(NULL_PTR);
    Platform_Init(NULL_PTR);

#if (STD_ON == GPT_PRECOMPILE_SUPPORT)
    Gpt_Init(NULL_PTR);
#else
    Gpt_Init(&Gpt_Config_VS_0);
#endif

    Uart_Init(NULL_PTR);
    log_init();

    LOG_I(TAG, "");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "  LAN9646 + GMAC + FreeRTOS");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "MCU Init complete!");

    /* LAN9646 Init */
    lan9646_cfg_t cfg = {
        .if_type = LAN9646_IF_I2C,
        .i2c_addr = LAN9646_I2C_ADDR_DEFAULT,
        .ops.i2c = {
            .init_fn = i2c_init_cb,
            .write_fn = i2c_write_cb,
            .read_fn = i2c_read_cb,
            .mem_write_fn = i2c_mem_write_cb,
            .mem_read_fn = i2c_mem_read_cb
        }
    };

    LOG_I(TAG, "");
    LOG_I(TAG, "Initializing LAN9646...");
    if (lan9646_init(&g_lan9646, &cfg) != lan9646OK) {
        LOG_E(TAG, "LAN9646 init FAILED!");
        while(1);
    }

    if (lan9646_get_chip_id(&g_lan9646, &chip_id, &revision) != lan9646OK) {
        LOG_E(TAG, "Failed to read chip ID!");
        while(1);
    }
    LOG_I(TAG, "Chip: 0x%04X Rev:%d", chip_id, revision);

    OsIf_TimeDelay(100);
    configure_port6_rgmii_1g();
    OsIf_TimeDelay(500);

    debug_port6_registers();
    debug_switch_config();

    /* Eth Init */
    LOG_I(TAG, "");
    LOG_I(TAG, "Initializing Eth...");
    Eth_Init(NULL_PTR);
    LOG_I(TAG, "  Eth_Init OK");

    LOG_I(TAG, "  Setting controller mode to ACTIVE...");
    Std_ReturnType ret = Eth_43_GMAC_SetControllerMode(ETH_CTRL_IDX, ETH_MODE_ACTIVE);
    if (ret == E_OK) {
        LOG_I(TAG, "  Controller ACTIVE!");
    } else {
        LOG_E(TAG, "  SetControllerMode failed: %d", ret);
    }

    OsIf_TimeDelay(200);

    LOG_I(TAG, "Starting FreeRTOS scheduler...");

    /* Create main task */
    BaseType_t xRet = xTaskCreate(
        mainLoopTask,
        "mainloop",
        1024U,
        NULL,
        (tskIDLE_PRIORITY + 1),
        NULL
    );

    if (xRet != pdPASS) {
        LOG_E(TAG, "Failed to create task");
        while(1);
    }

    /* Start scheduler */
    vTaskStartScheduler();

    for (;;) {}

    return 0;
}
