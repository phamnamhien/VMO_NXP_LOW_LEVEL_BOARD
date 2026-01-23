/**
 * @file    main.c
 * @brief   RGMII Hardware Diagnostic - S32K388 + LAN9646
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

#include "lan9646.h"
#include "s32k3xx_soft_i2c.h"
#include "log_debug.h"
#include "rgmii_diag.h"
#include "rgmii_config_debug.h"
#include "systick.h"
#include "uart_poll.h"  /* Simple polling UART for debug */

/* External config symbols from generated PBcfg files */
extern const Mcu_ConfigType Mcu_PreCompileConfig;
extern const Port_ConfigType Port_Config;
extern const Gpt_ConfigType Gpt_Config;
extern const Eth_43_GMAC_ConfigType Eth_43_GMAC_xPredefinedConfig;

#define TAG "MAIN"

/*===========================================================================*/
/*                          CONFIGURATION                                     */
/*===========================================================================*/

#define LAN9646_SCL_CHANNEL     DioConf_DioChannel_SCL_CH
#define LAN9646_SDA_CHANNEL     DioConf_DioChannel_SDA_CH
#define LAN9646_I2C_SPEED       5U
#define ETH_CTRL_IDX            0U

/*===========================================================================*/
/*                          GLOBAL VARIABLES                                  */
/*===========================================================================*/

static lan9646_t g_lan9646;
static softi2c_t g_i2c;

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
    /* Use SysTick module for delay */
    SysTick_DelayMs(ms);
}

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

    /* Configure Port 6 for RGMII 100Mbps - start with no delay */
    LOG_I(TAG, "Configuring Port 6 for RGMII 100Mbps...");

    /* XMII_CTRL0: Full duplex, Flow control, 100M */
    lan9646_write_reg8(&g_lan9646, 0x6300, 0x78);

    /* XMII_CTRL1: 100M mode, no delay initially (will be tested) */
    lan9646_write_reg8(&g_lan9646, 0x6301, 0x40);

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
/*                          S32K388 GMAC INIT                                 */
/*===========================================================================*/

static void configure_s32k388_rgmii(void) {
    LOG_I(TAG, "Configuring S32K388 for RGMII 100Mbps...");

    /* DCM_GPR: Set RGMII mode */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    dcmrwf1 &= ~0x03U;
    dcmrwf1 |= 0x02U;  /* RGMII mode */
    IP_DCM_GPR->DCMRWF1 = dcmrwf1;

    /* DCM_GPR: Enable TX clock output, bypass RX clock mux */
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    dcmrwf3 |= (1U << 3);  /* GMAC_TX_CLK_OUT_EN */
    dcmrwf3 |= (1U << 0);  /* GMAC_RX_CLK_MUX_BYPASS */
    IP_DCM_GPR->DCMRWF3 = dcmrwf3;

    LOG_I(TAG, "  DCMRWF1=0x%08lX DCMRWF3=0x%08lX",
          (unsigned long)IP_DCM_GPR->DCMRWF1,
          (unsigned long)IP_DCM_GPR->DCMRWF3);
}

static void configure_gmac_mac(void) {
    LOG_I(TAG, "Configuring GMAC MAC...");

    /* MAC Configuration for 100Mbps Full Duplex */
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;

    mac_cfg |= (1U << 14);  /* FES: Fast Ethernet Speed (100M) */
    mac_cfg |= (1U << 13);  /* DM: Full Duplex */
    mac_cfg |= (1U << 15);  /* PS: Port Select (MII/RMII/RGMII) */
    mac_cfg |= (1U << 0);   /* RE: Receiver Enable */
    mac_cfg |= (1U << 1);   /* TE: Transmitter Enable */

    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;

    LOG_I(TAG, "  MAC_CFG=0x%08lX", (unsigned long)IP_GMAC_0->MAC_CONFIGURATION);
}

/*===========================================================================*/
/*                          DEVICE INIT                                       */
/*===========================================================================*/

static void device_init(void) {
    /* Step 1: MCU Init (no logging available yet) */
    Mcu_Init(&Mcu_PreCompileConfig);
    Mcu_InitClock(McuClockSettingConfig_0);
    while (MCU_PLL_LOCKED != Mcu_GetPllStatus()) {}
    Mcu_DistributePllClock();
    Mcu_SetMode(McuModeSettingConf_0);

    /* Step 2: Platform & Port */
    OsIf_Init(NULL);
    Platform_Init(NULL);
    /* Note: GMAC IRQ handlers are installed by Eth_43_GMAC driver */
    Port_Init(&Port_Config);
    Gpt_Init(&Gpt_Config);
    Gpt_StartTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0, 0xFFFFFFFFU);

    /* Initialize simple polling UART for debug - no interrupts needed */
    uart_poll_init();
    uart_poll_printf("\n\n");
    uart_poll_printf("================================================================\n");
    uart_poll_printf("          RGMII HARDWARE DIAGNOSTIC - S32K388 + LAN9646\n");
    uart_poll_printf("================================================================\n");
    uart_poll_printf("[Step 1] MCU Init... Done\n");
    uart_poll_printf("[Step 2] Platform & Port Init... Done\n");

    /* Initialize SysTick for delay functions */
    uart_poll_printf("[Step 2b] SysTick Init...\n");
    SysTick_Init();
    uart_poll_printf("[Step 2b] SysTick Init... Done\n");

    /* Step 3: Configure S32K388 RGMII */
    uart_poll_printf("[Step 3] Configure S32K388 RGMII...\n");
    configure_s32k388_rgmii();
    uart_poll_printf("[Step 3] Configure S32K388 RGMII... Done\n");

    /* Step 4: Init LAN9646 */
    uart_poll_printf("[Step 4] Init LAN9646...\n");
    if (init_lan9646() != lan9646OK) {
        uart_poll_printf("FATAL: LAN9646 init failed!\n");
        while (1) {}
    }
    uart_poll_printf("[Step 4] Init LAN9646... Done\n");

    /* Step 5: Init Ethernet (AUTOSAR) */
    uart_poll_printf("[Step 5] Init Ethernet...\n");
    Eth_43_GMAC_Init(&Eth_43_GMAC_xPredefinedConfig);
    uart_poll_printf("[Step 5] Init Ethernet... Done\n");

    /* Step 6: Configure GMAC MAC */
    uart_poll_printf("[Step 6] Configure GMAC MAC...\n");
    configure_gmac_mac();
    uart_poll_printf("[Step 6] Configure GMAC MAC... Done\n");

    /* Step 7: Set controller active */
    uart_poll_printf("[Step 7] Activate Ethernet controller...\n");
    Eth_43_GMAC_SetControllerMode(ETH_CTRL_IDX, ETH_MODE_ACTIVE);
    uart_poll_printf("[Step 7] Activate Ethernet controller... Done\n");

    uart_poll_printf("\n");
    uart_poll_printf("Device initialization complete!\n");
    uart_poll_printf("\n");
}

/*===========================================================================*/
/*                          MAIN                                              */
/*===========================================================================*/

int main(void) {
    /* Initialize hardware */
    device_init();

    /* Small delay for hardware to stabilize */
    uart_poll_printf("Waiting 100ms for hardware to stabilize...\n");
    delay_ms(100);
    uart_poll_printf("Hardware stabilization complete.\n");

    /* Initialize diagnostic modules */
    uart_poll_printf("Initializing diagnostic modules...\n");
    rgmii_diag_init(&g_lan9646, delay_ms);
    rgmii_debug_init(&g_lan9646, delay_ms);
    uart_poll_printf("Diagnostic modules initialized.\n");

    uart_poll_printf("\n");
    uart_poll_printf("================================================================\n");
    uart_poll_printf("  SELECT DEBUG MODE:\n");
    uart_poll_printf("  1. Quick Summary (rgmii_debug_quick_summary)\n");
    uart_poll_printf("  2. Full Configuration Dump (rgmii_debug_dump_all)\n");
    uart_poll_printf("  3. Full Diagnostic with Tests (rgmii_debug_full_diagnostic)\n");
    uart_poll_printf("  4. Original RGMII Diagnostic (rgmii_diag_run_all)\n");
    uart_poll_printf("================================================================\n");
    uart_poll_printf("\n");

    /* --- Run Quick Summary first for overview --- */
    uart_poll_printf("Running Quick Summary...\n");
    rgmii_debug_quick_summary();

    /* --- Run Full Configuration Dump --- */
    LOG_I(TAG, "");
    LOG_I(TAG, "Running Full Configuration Dump...");
    delay_ms(100);
    rgmii_debug_dump_all();

    /* --- Run Full Diagnostic with Tests --- */
    LOG_I(TAG, "");
    LOG_I(TAG, "Running Full Diagnostic with Tests...");
    delay_ms(100);
    rgmii_debug_full_diagnostic();

    /* --- Also run original diagnostic for comparison --- */
    LOG_I(TAG, "");
    LOG_I(TAG, "Running Original RGMII Diagnostic...");
    delay_ms(100);
    rgmii_test_result_t result = rgmii_diag_run_all();

    LOG_I(TAG, "");
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "                    FINAL SUMMARY");
    LOG_I(TAG, "================================================================");
    if (result == RGMII_TEST_PASS) {
        LOG_I(TAG, "  RGMII DIAGNOSTIC: ALL TESTS PASSED!");
        LOG_I(TAG, "  Hardware is working correctly.");
    } else {
        LOG_E(TAG, "  RGMII DIAGNOSTIC: ISSUES DETECTED");
        LOG_E(TAG, "  Result: %s", rgmii_diag_result_str(result));
        LOG_E(TAG, "");
        LOG_E(TAG, "  Recommended Actions:");
        LOG_E(TAG, "  1. Check the configuration dump above");
        LOG_E(TAG, "  2. Review the timing sweep results");
        LOG_E(TAG, "  3. See troubleshooting guide above");
    }
    LOG_I(TAG, "================================================================");
    LOG_I(TAG, "");

    /* Infinite loop with periodic status */
    uint32_t loop_count = 0;
    while (1) {
        delay_ms(5000);
        loop_count++;

        LOG_I(TAG, "[%lu] System running...", (unsigned long)loop_count);

        /* Every 60 seconds, print quick summary */
        if (loop_count % 12 == 0) {
            LOG_I(TAG, "--- Periodic Status Check ---");
            rgmii_debug_quick_summary();
        }
    }

    return 0;
}

