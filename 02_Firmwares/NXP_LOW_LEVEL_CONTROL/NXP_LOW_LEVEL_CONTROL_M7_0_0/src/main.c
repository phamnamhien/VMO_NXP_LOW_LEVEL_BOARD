/**
 * @file    main.c
 * @brief   RGMII Hardware Diagnostic - S32K388 + LAN9646 (FreeRTOS Version)
 */

#include <string.h>
#include <stdint.h>

#include "S32K388.h"
#include "Mcal.h"
#include "Mcu.h"
#include "Mcu_Cfg.h"
#include "Port.h"
#include "Port_Cfg.h"
#include "OsIf.h"
#include "Platform.h"
#include "Gpt.h"
#include "Gpt_Cfg.h"
#include "Eth_43_GMAC.h"
#include "Eth_43_GMAC_Cfg.h"
#include "Gmac_Ip.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

#include "lan9646.h"
#include "s32k3xx_soft_i2c.h"
#include "CDD_Uart.h"
#include "log_debug.h"
#include "rgmii_diag.h"
#include "rgmii_config_debug.h"
#include "rgmii_rx_debug.h"

/* External config symbols from generated PBcfg files */
extern const Eth_43_GMAC_ConfigType Eth_43_GMAC_xPredefinedConfig;

#define TAG "MAIN"

/*===========================================================================*/
/*                          CONFIGURATION                                     */
/*===========================================================================*/

#define LAN9646_SCL_CHANNEL     DioConf_DioChannel_SCL_CH
#define LAN9646_SDA_CHANNEL     DioConf_DioChannel_SDA_CH
#define LAN9646_I2C_SPEED       5U
#define ETH_CTRL_IDX            0U

/* FreeRTOS Task Configuration */
#define DIAG_TASK_STACK_SIZE    4096U
#define DIAG_TASK_PRIORITY      (tskIDLE_PRIORITY + 2U)

/*===========================================================================*/
/*                          GLOBAL VARIABLES                                  */
/*===========================================================================*/

static lan9646_t g_lan9646;
static softi2c_t g_i2c;
static volatile bool g_scheduler_started = false;

/*===========================================================================*/
/*                          I2C CALLBACKS                                     */
/*===========================================================================*/

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

/*===========================================================================*/
/*                          DELAY FUNCTION                                    */
/*===========================================================================*/

static void delay_ms(uint32_t ms) {
    if (g_scheduler_started) {
        /* Use FreeRTOS delay - non-blocking, allows other tasks to run */
        vTaskDelay(pdMS_TO_TICKS(ms));
    } else {
        /* Busy-wait before scheduler starts (during init) */
        volatile uint32_t count;
        while (ms > 0) {
            count = 40000U;  /* Approx 1ms at 160MHz */
            while (count > 0) {
                count--;
            }
            ms--;
        }
    }
}

/* Short delay for UART flush - minimal delay */
#define UART_FLUSH_DELAY()  delay_ms(10)

/*===========================================================================*/
/*                          LAN9646 INIT                                      */
/*===========================================================================*/

static lan9646r_t init_lan9646(void) {
    LOG_I(TAG, "Initializing LAN9646...");

    lan9646_cfg_t cfg = {
        .if_type = LAN9646_IF_I2C,
        .i2c_addr = 0x5F,
        .ops.i2c = {
            .init_fn = i2c_init_cb,
            .write_fn = i2c_write_cb,
            .read_fn = i2c_read_cb,
            .mem_write_fn = i2c_mem_write_cb,
            .mem_read_fn = i2c_mem_read_cb,
        },
    };

    if (lan9646_init(&g_lan9646, &cfg) != lan9646OK) {
        LOG_E(TAG, "LAN9646 init failed!");
        return lan9646ERR;
    }

    /* Read chip ID */
    uint16_t chip_id;
    uint8_t revision;
    lan9646_get_chip_id(&g_lan9646, &chip_id, &revision);
    LOG_I(TAG, "Chip: 0x%04X Rev:%d", chip_id, revision);

    /* Configure Port 6 for RGMII 1Gbps */
    LOG_I(TAG, "Configuring Port 6 for RGMII 1Gbps...");

    /* XMII_CTRL0: Full duplex, flow control, 1Gbps mode */
    lan9646_write_reg8(&g_lan9646, 0x6300, 0x68);

    /* XMII_CTRL1: 1Gbps + RX ID + TX ID delays */
    lan9646_write_reg8(&g_lan9646, 0x6301, 0x18);

    /* Verify XMII configuration */
    uint8_t xmii_ctrl0, xmii_ctrl1;
    lan9646_read_reg8(&g_lan9646, 0x6300, &xmii_ctrl0);
    lan9646_read_reg8(&g_lan9646, 0x6301, &xmii_ctrl1);
    LOG_I(TAG, "  XMII_CTRL0=0x%02X XMII_CTRL1=0x%02X", xmii_ctrl0, xmii_ctrl1);
    LOG_I(TAG, "    Duplex: %s, Speed: %s",
          (xmii_ctrl0 & 0x40) ? "Full" : "Half",
          (xmii_ctrl1 & 0x40) ? "10/100M" : "1Gbps");
    LOG_I(TAG, "    TX ID (RX_CLK delay): %s", (xmii_ctrl1 & 0x08) ? "ON" : "OFF");
    LOG_I(TAG, "    RX ID (TX_CLK delay): %s", (xmii_ctrl1 & 0x10) ? "ON" : "OFF");

    /* Enable switch */
    lan9646_write_reg8(&g_lan9646, 0x0300, 0x01);

    /* Port membership */
    lan9646_write_reg32(&g_lan9646, 0x6A04, 0x4F);
    lan9646_write_reg32(&g_lan9646, 0x1A04, 0x6E);
    lan9646_write_reg32(&g_lan9646, 0x2A04, 0x6D);
    lan9646_write_reg32(&g_lan9646, 0x3A04, 0x6B);
    lan9646_write_reg32(&g_lan9646, 0x4A04, 0x67);

    LOG_I(TAG, "LAN9646 ready");
    return lan9646OK;
}

/*===========================================================================*/
/*                          S32K388 RGMII CONFIG                              */
/*===========================================================================*/

#define DCMRWF3_MAC_RX_CLK_MUX_BYPASS_BIT   (13U)
#define DCMRWF3_MAC_TX_CLK_MUX_BYPASS_BIT   (12U)
#define DCMRWF3_MAC_TX_CLK_OUT_EN_BIT       (11U)

static void configure_s32k388_rgmii(void) {
    LOG_I(TAG, "Configuring S32K388 RGMII...");

    /* DCMRWF1: Set RGMII mode */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    LOG_I(TAG, "  DCMRWF1 before: 0x%08lX", (unsigned long)dcmrwf1);

    dcmrwf1 = (dcmrwf1 & ~0x03U) | 0x01U;  /* MAC_CONF_SEL = 1 (RGMII) */
    dcmrwf1 |= (1U << 6);                   /* MAC_TX_RMII_CLK_LPBCK_EN = 1 */
    IP_DCM_GPR->DCMRWF1 = dcmrwf1;

    dcmrwf1 = IP_DCM_GPR->DCMRWF1;  /* Read back */
    LOG_I(TAG, "  DCMRWF1 after:  0x%08lX -> %s",
          (unsigned long)dcmrwf1,
          ((dcmrwf1 & 0x03U) == 1) ? "RGMII OK" : "ERROR");

    /* DCMRWF3: Clock configuration */
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    LOG_I(TAG, "  DCMRWF3 before: 0x%08lX", (unsigned long)dcmrwf3);

    dcmrwf3 |= (1U << DCMRWF3_MAC_RX_CLK_MUX_BYPASS_BIT);  /* Bypass MUX_7 */
    dcmrwf3 |= (1U << DCMRWF3_MAC_TX_CLK_OUT_EN_BIT);      /* Enable TX_CLK output */
    IP_DCM_GPR->DCMRWF3 = dcmrwf3;

    dcmrwf3 = IP_DCM_GPR->DCMRWF3;  /* Read back */
    LOG_I(TAG, "  DCMRWF3 after:  0x%08lX", (unsigned long)dcmrwf3);
    LOG_I(TAG, "    RX_CLK bypass [13] = %lu -> %s",
          (unsigned long)((dcmrwf3 >> 13) & 1),
          ((dcmrwf3 >> 13) & 1) ? "BYPASS OK" : "ERROR");
    LOG_I(TAG, "    TX_CLK output [11] = %lu -> %s",
          (unsigned long)((dcmrwf3 >> 11) & 1),
          ((dcmrwf3 >> 11) & 1) ? "ENABLED OK" : "ERROR");
}

static void configure_gmac_mac(void) {
    LOG_I(TAG, "Configuring GMAC MAC for 1Gbps...");

    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    mac_cfg &= ~(1U << 15);  /* PS = 0 for 1Gbps */
    mac_cfg &= ~(1U << 14);  /* FES = 0 */
    mac_cfg |= (1U << 13);   /* DM: Full Duplex */
    mac_cfg |= (1U << 0);    /* RE: Receiver Enable */
    mac_cfg |= (1U << 1);    /* TE: Transmitter Enable */
    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;

    LOG_I(TAG, "  MAC_CFG=0x%08lX", (unsigned long)IP_GMAC_0->MAC_CONFIGURATION);
}

/*===========================================================================*/
/*                          DEVICE INIT                                       */
/*===========================================================================*/

static void device_init(void) {
    OsIf_Init(NULL_PTR);
    Port_Init(NULL_PTR);

    Mcu_Init(NULL_PTR);
    Mcu_InitClock(McuClockSettingConfig_0);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {}
    Mcu_DistributePllClock();
    Mcu_SetMode(McuModeSettingConf_0);

    Platform_Init(NULL_PTR);

    Gpt_Init(NULL_PTR);
    Gpt_StartTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0, 40000U);
    Gpt_EnableNotification(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);

    Uart_Init(NULL_PTR);
    log_init();

    LOG_I(TAG, "");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "  RGMII 1Gbps DIAGNOSTIC - S32K388 + LAN9646 (FreeRTOS)");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "");

    if (init_lan9646() != lan9646OK) {
        LOG_E(TAG, "FATAL: LAN9646 init failed!");
        while (1) {}
    }

    Eth_43_GMAC_Init(&Eth_43_GMAC_xPredefinedConfig);
    configure_gmac_mac();
    Eth_43_GMAC_SetControllerMode(ETH_CTRL_IDX, ETH_MODE_ACTIVE);
    configure_s32k388_rgmii();

    LOG_I(TAG, "Device init complete!");
}

/*===========================================================================*/
/*                          DIAGNOSTIC TASK                                   */
/*===========================================================================*/

static void diagnostic_task(void *pvParameters) {
    (void)pvParameters;

    /* Start log auto-flush timer (runs every 10ms to drain ring buffer) */
    log_start_flush_timer();

    LOG_I(TAG, "");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "    RX PATH DEBUG - S32K388 GMAC <-- LAN9646 Port 6");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "");

    /* Initialize diagnostic modules */
    rgmii_diag_init(&g_lan9646, delay_ms);
    rgmii_debug_init(&g_lan9646, delay_ms);
    rx_debug_init(&g_lan9646, delay_ms);

    /*=========================================================================*/
    /*                    RX PATH DEBUGGING SEQUENCE                           */
    /*=========================================================================*/

    /* Step 1: Quick config summary */
    LOG_I(TAG, "[STEP 1] Quick Configuration Summary");
    rgmii_debug_quick_summary();

    /* Step 2: Full RX path analysis (includes loopback test) */
    LOG_I(TAG, "");
    LOG_I(TAG, "[STEP 2] Full RX Path Analysis");
    rx_debug_full_analysis();

    /* Step 3: TX Delay sweep to find optimal timing */
    LOG_I(TAG, "");
    LOG_I(TAG, "[STEP 3] TX Delay Sweep");
    rx_debug_delay_sweep();

    /* Get final RX count for summary */
    uint32_t rx_count = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;

    /* Final summary */
    LOG_I(TAG, "");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "                    FINAL SUMMARY");
    LOG_I(TAG, "================================================================");
    if (rx_count > 0) {
        LOG_I(TAG, "  RX PATH STATUS: WORKING!");
        LOG_I(TAG, "  Received %lu packets via RX path.", (unsigned long)rx_count);
    } else {
        LOG_E(TAG, "  RX PATH STATUS: NOT WORKING");
        LOG_E(TAG, "  Check the analysis above for issues.");
    }
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "");
    LOG_I(TAG, "Diagnostic complete. Entering monitoring mode...");
    LOG_I(TAG, "(Will print status every 2 seconds)");
    LOG_I(TAG, "");

    /*=========================================================================*/
    /*                    MONITORING LOOP                                      */
    /*=========================================================================*/

    uint32_t loop_count = 0;
    for (;;) {
        loop_count++;
        LOG_I(TAG, "[%lu] Waiting 1s...", (unsigned long)loop_count);

        vTaskDelay(pdMS_TO_TICKS(1000));  /* 1 second delay */

        LOG_I(TAG, "[%lu] After delay, RX=%lu",
              (unsigned long)loop_count,
              (unsigned long)IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD);

        /* Every 30 seconds (15 iterations), show counters */
        if (loop_count % 15 == 0) {
            LOG_I(TAG, "--- Periodic Counter Check ---");
            rx_debug_dump_gmac_counters();
            rx_debug_dump_lan9646_tx_counters();
        }

        /* Every 60 seconds (30 iterations), full analysis */
        if (loop_count % 30 == 0) {
            LOG_I(TAG, "--- Periodic RX Analysis ---");
            rx_debug_full_analysis();
        }
    }
}

/*===========================================================================*/
/*                          MAIN                                              */
/*===========================================================================*/

int main(void) {
    /* Initialize hardware (before scheduler starts) */
    device_init();

    /* Create diagnostic task */
    BaseType_t result = xTaskCreate(
        diagnostic_task,
        "DiagTask",
        DIAG_TASK_STACK_SIZE,
        NULL,
        DIAG_TASK_PRIORITY,
        NULL
    );

    if (result != pdPASS) {
        LOG_E(TAG, "Failed to create diagnostic task!");
        while (1) {}
    }

    LOG_I(TAG, "Starting FreeRTOS scheduler...");
    LOG_I(TAG, "");

    /* Wait for UART to complete before starting scheduler */
    delay_ms(50);

    /* Mark scheduler as started and start it */
    g_scheduler_started = true;
    vTaskStartScheduler();

    /* Should never reach here */
    LOG_E(TAG, "Scheduler exited unexpectedly!");
    while (1) {}

    return 0;
}
