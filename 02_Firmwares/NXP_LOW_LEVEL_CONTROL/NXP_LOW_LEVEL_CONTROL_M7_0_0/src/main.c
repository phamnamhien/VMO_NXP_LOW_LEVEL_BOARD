/**
 * \file            main.c
 * \brief           LAN9646 + Eth_43_GMAC Debug
 */

#include <string.h>

#include "Mcal.h"
#include "Mcu.h"
#include "Port.h"
#include "OsIf.h"
#include "Gpt.h"
#include "Platform.h"
#include "Eth_43_GMAC.h"
//#include "EthIf.h"

#include "lan9646.h"
#include "lan9646_switch.h"
#include "s32k3xx_soft_i2c.h"
#include "log_debug.h"
#include "systick.h"



#define TAG "MAIN"

/*===========================================================================*/
/*                           CONFIGURATION                                    */
/*===========================================================================*/

#define LAN9646_SCL_CHANNEL     DioConf_DioChannel_SCL_CH
#define LAN9646_SDA_CHANNEL  	DioConf_DioChannel_SDA_CH
#define LAN9646_I2C_SPEED       5U

#define ETH_CTRL_IDX            0U

/*===========================================================================*/
/*                           PRIVATE VARIABLES                                */
/*===========================================================================*/

static lan9646_t g_lan9646;
static softi2c_t g_i2c;

/*===========================================================================*/
/*                          DELAY FUNCTION                                    */
/*===========================================================================*/

static void delay_ms(uint32_t ms) {
	SysTick_DelayMs(ms);
//    uint32_t start = OsIf_GetMilliseconds();
//    while ((OsIf_GetMilliseconds() - start) < ms) {
//        /* Wait */
//    }
}

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
/*                          SWITCH DEBUG                                      */
/*===========================================================================*/

static void debug_switch_config(void) {
    uint8_t val8;
    uint32_t val32;

    LOG_I(TAG, "");
    LOG_I(TAG, "========== Switch Global Config ==========");

    /* Switch Operation (0x0300) */
    if (lan9646_read_reg8(&g_lan9646, 0x0300, &val8) == lan9646OK) {
        LOG_I(TAG, "SW_OPERATION (0x0300) = 0x%02X", val8);
        LOG_I(TAG, "  [0] Start Switch: %s", (val8 & 0x01) ? "YES" : "NO");
    }

    /* Check Port VLAN Membership and MSTP State */
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
        LOG_I(TAG, "PORT_STATUS = 0x%02X [Speed=%s, Duplex=%s]",
              status,
              speed == 0 ? "10M" : (speed == 1 ? "100M" : "1000M"),
              (status & 0x04) ? "Full" : "Half");
    }
    LOG_I(TAG, "=======================================");
}

/*===========================================================================*/
/*                          CONFIGURE PORT 6 RGMII 1G                         */
/*===========================================================================*/

static lan9646r_t configure_port6_rgmii_1g(void) {
    uint8_t ctrl0, ctrl1, port_ctrl;

    LOG_I(TAG, "Configuring Port 6 for RGMII 1G...");

    ctrl0 = 0x68;  /* Full duplex + TX flow + RX flow */
    ctrl1 = 0x08;  /* TX delay ON, 1G speed */

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

    /* Enable Port 6 */
    lan9646_read_reg8(&g_lan9646, 0x6000, &port_ctrl);
    port_ctrl |= 0x03;
    lan9646_write_reg8(&g_lan9646, 0x6000, port_ctrl);

    LOG_I(TAG, "  Port 6 config OK: RGMII 1G, Full Duplex, TX_DLY=ON");
    return lan9646OK;
}

/*===========================================================================*/
/*                          ETH TX TEST                                       */
/*===========================================================================*/

static void eth_send_test_frame(void) {
//    Eth_BufIdxType bufIdx;
//    uint8_t* txBufPtr = NULL;
//    uint16_t bufLen = 64;
//    Std_ReturnType ret;
//
//    /* ARP broadcast frame */
//    static const uint8_t arp_frame[] = {
//        /* Destination: Broadcast */
//        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//        /* Source: Our MAC */
//        0xAA, 0x12, 0x22, 0x33, 0x44, 0x55,
//        /* EtherType: 0x0806 (ARP) */
//        0x08, 0x06,
//        /* ARP Request */
//        0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01,
//        /* Sender MAC + IP */
//        0xAA, 0x12, 0x22, 0x33, 0x44, 0x55,
//        0xC0, 0xA8, 0x01, 0x64,
//        /* Target MAC + IP */
//        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//        0xC0, 0xA8, 0x01, 0x01,
//    };
//
//    LOG_I(TAG, "Sending ARP broadcast...");
//
//    /* Get TX buffer */
//    ret = Eth_43_GMAC_ProvideTxBuffer(ETH_CTRL_IDX, 0U, &bufIdx, &txBufPtr, &bufLen);
//    if (ret != E_OK || txBufPtr == NULL) {
//        LOG_E(TAG, "  ProvideTxBuffer failed: %d", ret);
//        return;
//    }
//    LOG_I(TAG, "  Got TX buffer idx=%d, len=%d", bufIdx, bufLen);
//
//    /* Copy frame data */
//    memcpy(txBufPtr, arp_frame, sizeof(arp_frame));
//    /* Pad to 64 bytes */
//    memset(txBufPtr + sizeof(arp_frame), 0, 64 - sizeof(arp_frame));
//
//    /* Transmit */
//    ret = Eth_43_GMAC_Transmit(ETH_CTRL_IDX, bufIdx, ETH_FRAME_TYPE_ARP, TRUE, 64, NULL);
//    if (ret == E_OK) {
//        LOG_I(TAG, "  TX queued OK");
//    } else {
//        LOG_E(TAG, "  Transmit failed: %d", ret);
//    }
}

/*===========================================================================*/
/*                          MAIN                                             */
/*===========================================================================*/
int main(void) {
    uint16_t chip_id;
    uint8_t revision;
    uint32_t tick = 0;

    /*========== MCU Init ==========*/
    /* Initialize MCU module */
    Mcu_Init(NULL_PTR);

    /* Initialize MCU clock */
    Mcu_InitClock(McuClockSettingConfig_0);

#if (MCU_NO_PLL == STD_OFF)
    while ( MCU_PLL_LOCKED != Mcu_GetPllStatus() )
    {
        /* Busy wait until the System PLL is locked */
    }
    //    /* Use PLL clock */
    Mcu_DistributePllClock();
#endif

    Mcu_SetMode(McuModeSettingConf_0);


    /* Initialize OS Interface */
    OsIf_Init(NULL_PTR);

    /* Initialize all pins */
    Port_Init(NULL_PTR);
    /* Initialize Platform driver */
    Platform_Init(NULL_PTR);

    /* Initialize the high level configuration structure of Gpt driver */
	#if (STD_ON == GPT_PRECOMPILE_SUPPORT)
		Gpt_Init(NULL_PTR);
	#else
		Gpt_Init(&Gpt_Config_VS_0);
	#endif

    /* Initialize SysTick (GPT) */
    SysTick_Init();

    Uart_Init(NULL_PTR);

    /* Init log (after clock is ready) */
    log_init();


    LOG_I(TAG, "");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "  LAN9646 + Eth_43_GMAC Debug");
    LOG_I(TAG, "========================================");
    LOG_I(TAG, "MCU Init complete!");

    /*========== LAN9646 Init ==========*/
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

    /* Configure Port 6 for RGMII 1G */
    delay_ms(100);
    configure_port6_rgmii_1g();

    /* Wait for link stabilization */
    delay_ms(500);

    /* Debug registers */
    debug_port6_registers();
    debug_switch_config();

    /*========== Eth_43_GMAC Init ==========*/
    LOG_I(TAG, "");
    LOG_I(TAG, "Initializing Eth_43_GMAC...");
	Eth_43_GMAC_Init(NULL_PTR);
    LOG_I(TAG, "  Eth_43_GMAC_Init OK");

    /* Set controller to ACTIVE mode */
    LOG_I(TAG, "  Setting controller mode to ACTIVE...");
//    Std_ReturnType ret = Eth_43_GMAC_SetControllerMode(ETH_CTRL_IDX, ETH_MODE_ACTIVE);
//    if (ret == E_OK) {
//        LOG_I(TAG, "  Controller ACTIVE!");
//    } else {
//        LOG_E(TAG, "  SetControllerMode failed: %d", ret);
//    }

    /* Wait for GMAC to stabilize */
    LOG_I(TAG, "Waiting 200ms...");
    delay_ms(200);

    /* Check P6 MIB before TX */
    lan9646_mib_simple_t mib;
    if (lan9646_switch_read_mib_simple(&g_lan9646, 6, &mib) == lan9646OK) {
        LOG_I(TAG, "P6 MIB BEFORE TX: RX=%lu TX=%lu",
              (unsigned long)mib.rx_packets, (unsigned long)mib.tx_packets);
    }

    /* Try to send test frame */
    eth_send_test_frame();
    delay_ms(100);

//    /* Poll TX/RX */
//    EthIf_MainFunctionTx();
//    EthIf_MainFunctionRx();

    /* Check P6 MIB after TX */
    if (lan9646_switch_read_mib_simple(&g_lan9646, 6, &mib) == lan9646OK) {
        LOG_I(TAG, "P6 MIB AFTER TX:  RX=%lu TX=%lu",
              (unsigned long)mib.rx_packets, (unsigned long)mib.tx_packets);
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "Ready - Monitoring...");

    /*========== Main Loop ==========*/
    while (1) {
//        /* Poll ETH TX/RX */
//        EthIf_MainFunctionTx();
//        EthIf_MainFunctionRx();

        if (++tick >= 50) {
            tick = 0;

            /* Check LAN9646 MIB counters */
            if (lan9646_switch_read_mib_simple(&g_lan9646, 6, &mib) == lan9646OK) {
                LOG_I(TAG, "P6 MIB: RX=%lu TX=%lu",
                      (unsigned long)mib.rx_packets, (unsigned long)mib.tx_packets);
            }
            if (lan9646_switch_read_mib_simple(&g_lan9646, 1, &mib) == lan9646OK) {
                LOG_I(TAG, "P1 MIB: RX=%lu TX=%lu",
                      (unsigned long)mib.rx_packets, (unsigned long)mib.tx_packets);
            }
            if (lan9646_switch_read_mib_simple(&g_lan9646, 2, &mib) == lan9646OK) {
                LOG_I(TAG, "P2 MIB: RX=%lu TX=%lu",
                      (unsigned long)mib.rx_packets, (unsigned long)mib.tx_packets);
            }

            /* Send another test frame */
            eth_send_test_frame();
        }

        delay_ms(100);
    }

    return 0;
}


