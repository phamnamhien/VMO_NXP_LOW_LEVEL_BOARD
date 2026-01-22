/**
 * \file            main.c
 * \brief           LAN9646 + Eth + FreeRTOS + lwIP
 */

#include <string.h>

#include "S32K388.h"
#include "Mcal.h"
#include "Mcu.h"
#include "Port.h"
#include "OsIf.h"
#include "Platform.h"
#include "Gpt.h"
#include "ethif_port.h"
#include "Eth_43_GMAC.h"

#include "lan9646.h"
#include "lan9646_switch.h"
#include "s32k3xx_soft_i2c.h"
#include "log_debug.h"

#define TAG "MAIN"

#define LAN9646_SCL_CHANNEL     DioConf_DioChannel_SCL_CH
#define LAN9646_SDA_CHANNEL     DioConf_DioChannel_SDA_CH
#define LAN9646_I2C_SPEED       5U

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

static void delay_ms(uint32_t ms) {
    volatile uint32_t i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 16000; j++) {
            __asm("NOP");
        }
    }
}

/* Thêm vào configure_port6_rgmii_1g() để debug */
static lan9646r_t configure_port6_rgmii_1g(void) {
    uint8_t ctrl0, ctrl1, port_ctrl;
    uint8_t read_back;

    LOG_I(TAG, "Configuring Port 6 for RGMII 1G...");

    ctrl0 = 0x68;  /* RGMII, 1G speed */
    ctrl1 = 0x08;  /* TX_DLY enable */

    lan9646_write_reg8(&g_lan9646, 0x6300, ctrl0);
    lan9646_write_reg8(&g_lan9646, 0x6301, ctrl1);

    /* Enable port */
    lan9646_read_reg8(&g_lan9646, 0x6000, &port_ctrl);
    port_ctrl |= 0x03;  /* TX/RX enable */
    lan9646_write_reg8(&g_lan9646, 0x6000, port_ctrl);

    /* Verify */
    lan9646_read_reg8(&g_lan9646, 0x6300, &read_back);
    LOG_I(TAG, "XMII_CTRL0: 0x%02X (expect 0x68)", read_back);

    lan9646_read_reg8(&g_lan9646, 0x6301, &read_back);
    LOG_I(TAG, "XMII_CTRL1: 0x%02X (expect 0x08)", read_back);

    lan9646_read_reg8(&g_lan9646, 0x6000, &read_back);
    LOG_I(TAG, "PORT6_CTRL: 0x%02X (expect bit0,1=1)", read_back);

    /* Check link status */
    uint8_t link_status;
    lan9646_read_reg8(&g_lan9646, 0x6100, &link_status);
    LOG_I(TAG, "PORT6 Link Status: 0x%02X", link_status);

    return lan9646OK;
}

static void debug_gmac_tx(void)
{
    Eth_BufIdxType bufIdx;
    uint8_t* bufPtr;
    Std_ReturnType ret;

    /* Try to get TX buffer */
    ret = Eth_43_GMAC_ProvideTxBuffer(0, 0, &bufIdx, &bufPtr, NULL);
    LOG_I(TAG, "GMAC ProvideTxBuffer: %s (ret=%d)",
          (ret == E_OK) ? "OK" : "FAIL", ret);

    if (ret == E_OK) {
        /* Build simple ARP request */
        uint8_t test_frame[64];
        memset(test_frame, 0, sizeof(test_frame));

        /* Dest MAC: broadcast */
        memset(&test_frame[0], 0xFF, 6);
        /* Src MAC */
        test_frame[6] = 0x10; test_frame[7] = 0x11;
        test_frame[8] = 0x22; test_frame[9] = 0x77;
        test_frame[10] = 0x77; test_frame[11] = 0x77;
        /* EtherType: ARP */
        test_frame[12] = 0x08; test_frame[13] = 0x06;

        memcpy(bufPtr, test_frame, 64);

        ret = Eth_43_GMAC_Transmit(0, bufIdx, 0x0806, FALSE, 64, NULL);
        LOG_I(TAG, "GMAC Transmit: %s (ret=%d)",
              (ret == E_OK) ? "OK" : "FAIL", ret);
    }

    /* Check controller mode */
    Eth_ModeType mode;
    Eth_43_GMAC_GetControllerMode(0, &mode);
    LOG_I(TAG, "GMAC Mode: %d (1=DOWN, 2=ACTIVE)", mode);
}

static void debug_gmac_clock(void)
{
    LOG_I(TAG, "=== GMAC Clock Debug ===");

    /* Check DCM_GPR registers */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;

    LOG_I(TAG, "DCMRWF1: 0x%08lX [MAC_CONF_SEL=%lu]",
          (unsigned long)dcmrwf1,
          (unsigned long)((dcmrwf1 >> 0) & 0x7));
    LOG_I(TAG, "DCMRWF3: 0x%08lX [RX_CLK_MUX_BYPASS=%lu]",
          (unsigned long)dcmrwf3,
          (unsigned long)((dcmrwf3 >> 0) & 0x1));

    /* Check MC_CGM for GMAC clocks */
    /* GMAC0_TX_CLK typically from MC_CGM_MUX_7 or MUX_9 */
    uint32_t mux7_css = IP_MC_CGM->MUX_7_CSS;
    uint32_t mux7_csc = IP_MC_CGM->MUX_7_CSC;
    uint32_t mux9_css = IP_MC_CGM->MUX_9_CSS;
    uint32_t mux9_csc = IP_MC_CGM->MUX_9_CSC;

    LOG_I(TAG, "MC_CGM MUX_7_CSS: 0x%08lX", (unsigned long)mux7_css);
    LOG_I(TAG, "MC_CGM MUX_7_CSC: 0x%08lX", (unsigned long)mux7_csc);
    LOG_I(TAG, "MC_CGM MUX_9_CSS: 0x%08lX", (unsigned long)mux9_css);
    LOG_I(TAG, "MC_CGM MUX_9_CSC: 0x%08lX", (unsigned long)mux9_csc);

    /* Check GMAC registers for clock config */
    uint32_t mac_config = IP_GMAC_0->MAC_CONFIGURATION;
    uint32_t mac_ext_config = IP_GMAC_0->MAC_EXT_CONFIGURATION;

    LOG_I(TAG, "MAC_CONFIG: 0x%08lX", (unsigned long)mac_config);
    LOG_I(TAG, "MAC_EXT_CONFIG: 0x%08lX", (unsigned long)mac_ext_config);

    /* Port Select: bits [14:12] */
    uint8_t ps = (mac_ext_config >> 12) & 0x7;
    LOG_I(TAG, "  Port Select: %d (0=GMII, 1=RGMII)", ps);
}


static void device_init(void) {
    OsIf_Init(NULL_PTR);

    Mcu_Init(NULL_PTR);
    Mcu_InitClock(McuClockSettingConfig_0);

#if (MCU_NO_PLL == STD_OFF)
    while (MCU_PLL_LOCKED != Mcu_GetPllStatus()) {}
    Mcu_DistributePllClock();
#endif

    Mcu_SetMode(McuModeSettingConf_0);

    Port_Init(NULL_PTR);
    Platform_Init(NULL_PTR);
    Uart_Init(NULL_PTR);
    log_init();

    LOG_I("INIT", "Setting RGMII mode...");

    /* Log BEFORE Eth_Init */
    LOG_I("INIT", "DCMRWF1 before Eth_Init: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF1);
    LOG_I("INIT", "DCMRWF3 before Eth_Init: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF3);

    Eth_Init(NULL_PTR);

    /* Log AFTER Eth_Init */
    LOG_I("INIT", "DCMRWF1 after Eth_Init: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF1);
    LOG_I("INIT", "DCMRWF3 after Eth_Init: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF3);

    /* Thử ghi và verify */
    IP_DCM_GPR->DCMRWF3 |= (1U << 0);  /* RX_CLK_MUX_BYPASS */
    LOG_I("INIT", "DCMRWF3 after set bypass: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF3);

    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    dcmrwf1 = (dcmrwf1 & ~0x7U) | 2U;  /* MAC_CONF_SEL = 2 */
    IP_DCM_GPR->DCMRWF1 = dcmrwf1;
    LOG_I("INIT", "DCMRWF1 after set RGMII: 0x%08lX", (unsigned long)IP_DCM_GPR->DCMRWF1);

    uint8_t mac[6];
    Eth_43_GMAC_GetPhysAddr(0, mac);
    LOG_I(TAG, "GMAC MAC: %02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}


static void lan9646_init_device(void) {
    uint16_t chip_id;
    uint8_t revision;

    /* Init LAN9646 FIRST to provide RX_CLK */
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

    delay_ms(100);
    configure_port6_rgmii_1g();
    delay_ms(500);

    LOG_I(TAG, "LAN9646 ready, GMAC will be activated by lwIP");
}

/* lwIP test entry */
extern void start_example(void);

int main(void) {
    device_init();

    LOG_I(TAG, "");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "  LAN9646 + GMAC + lwIP");
    LOG_I(TAG, "========================================");

    debug_gmac_clock();

    lan9646_init_device();

    start_example();

    return 0;
}

