/*
 * test_lan9646.c
 *
 *  Created on: Jan 8, 2026
 *      Author: phamnamhien
 */

///* src/test_lan9646.c */
//#include "Gmac_Ip.h"
//#include "Gmac_Ip_Cfg.h"
//#include "log_debug.h"
//#include "systick.h"
//
//#define LAN9646_MDIO_ADDR    0x00
//#define LAN9646_CHIP_ID_REG  0x00
//
//static const char* TAG = "LAN9646";
//
//void Test_LAN9646_Init(void)
//{
//    Gmac_Ip_StatusType status;
//    uint16 chipId = 0;
//
//    LOG_I(TAG, "=== LAN9646 Test Start ===");
//
//    status = Gmac_Ip_Init(0, &Gmac_0_ConfigPB);
//    if (status == GMAC_STATUS_SUCCESS) {
//        LOG_I(TAG, "GMAC Init OK");
//    } else {
//        LOG_E(TAG, "GMAC Init FAIL: %d", status);
//        return;
//    }
//
//    Gmac_Ip_EnableController(0);
//    LOG_I(TAG, "GMAC Controller Enabled");
//
//    status = Gmac_Ip_MDIORead(0, LAN9646_MDIO_ADDR, LAN9646_CHIP_ID_REG, &chipId, FALSE);
//    if (status == GMAC_STATUS_SUCCESS) {
//        LOG_I(TAG, "Chip ID: 0x%04X (expect 0x9646)", chipId);
//        if (chipId == 0x9646) {
//            LOG_I(TAG, "LAN9646 detected!");
//        } else {
//            LOG_W(TAG, "Unexpected Chip ID");
//        }
//    } else {
//        LOG_E(TAG, "MDIO Read FAIL: %d", status);
//    }
//
//    LOG_I(TAG, "=== Test Complete ===");
//}
//
//void Test_LAN9646_PeriodicRead(void)
//{
//    static uint32 lastTick = 0;
//    uint32 currentTick = SysTick_GetTick();
//    uint16 chipId = 0;
//
//    if (currentTick - lastTick >= 1000) {
//        lastTick = currentTick;
//
//        Gmac_Ip_StatusType status = Gmac_Ip_MDIORead(0, LAN9646_MDIO_ADDR,
//                                                      LAN9646_CHIP_ID_REG,
//                                                      &chipId, FALSE);
//        if (status == GMAC_STATUS_SUCCESS) {
//            LOG_D(TAG, "Chip ID: 0x%04X", chipId);
//        } else {
//            LOG_E(TAG, "MDIO Read Error");
//        }
//    }
//}
/* src/test_lan9646.c */
//#include "Gmac_Ip.h"
//#include "Gmac_Ip_Cfg.h"
//#include "log_debug.h"
//#include "systick.h"
//
//static const char* TAG = "LAN9646";
//
//void Test_LAN9646_Init(void)
//{
//    Gmac_Ip_StatusType status;
//    uint16 data = 0;
//
//    LOG_I(TAG, "=== LAN9646 Test Start ===");
//
//    status = Gmac_Ip_Init(0, &Gmac_0_ConfigPB);
//    if (status == GMAC_STATUS_SUCCESS) {
//        LOG_I(TAG, "GMAC Init OK");
//    } else {
//        LOG_E(TAG, "GMAC Init FAIL: %d", status);
//        return;
//    }
//
//    Gmac_Ip_EnableController(0);
//    LOG_I(TAG, "GMAC Controller Enabled");
//
//    // Thử scan các PHY address từ 0-31
//    LOG_I(TAG, "Scanning MDIO addresses...");
//    for(uint8 addr = 0; addr < 32; addr++) {
//        status = Gmac_Ip_MDIORead(0, addr, 0x02, &data, FALSE); // Reg 0x02: PHY ID1
//        if (status == GMAC_STATUS_SUCCESS && data != 0x0000 && data != 0xFFFF) {
//            LOG_I(TAG, "Found PHY at addr %d: ID1=0x%04X", addr, data);
//
//            // Đọc thêm register 0x03
//            Gmac_Ip_MDIORead(0, addr, 0x03, &data, FALSE);
//            LOG_I(TAG, "  PHY ID2=0x%04X", data);
//        }
//    }
//
//    LOG_I(TAG, "=== Scan Complete ===");
//}
//
//void Test_LAN9646_PeriodicRead(void)
//{
//    static uint32 lastTick = 0;
//    uint32 currentTick = SysTick_GetTick();
//    uint16 data = 0;
//
//    if (currentTick - lastTick >= 2000) {
//        lastTick = currentTick;
//
//        // Thử đọc Basic Status Register (0x01)
//        Gmac_Ip_StatusType status = Gmac_Ip_MDIORead(0, 0, 0x01, &data, FALSE);
//        if (status == GMAC_STATUS_SUCCESS) {
//            LOG_I(TAG, "PHY Status: 0x%04X", data);
//        }
//    }
//}
///* src/test_lan9646.c */
//#include "Gmac_Ip.h"
//#include "Gmac_Ip_Cfg.h"
//#include "log_debug.h"
//#include "systick.h"
//
//static const char* TAG = "LAN9646";
//
//extern GMAC_Type* const Gmac_apxBases[FEATURE_GMAC_NUM_INSTANCES];
//
//void Test_LAN9646_Init(void)
//{
//    Gmac_Ip_StatusType status;
//    uint16 data = 0;
//
//    LOG_I(TAG, "=== LAN9646 Test Start ===");
//
//    status = Gmac_Ip_Init(0, &Gmac_0_ConfigPB);
//    LOG_I(TAG, "GMAC Init: %d", status);
//
//    Gmac_Ip_EnableController(0);
//    LOG_I(TAG, "GMAC Enabled");
//
//    // Kiểm tra MDIO registers
//    GMAC_Type *base = Gmac_apxBases[0];
//    uint32 mdio_addr = base->MAC_MDIO_ADDRESS;
//    uint32 mdio_data = base->MAC_MDIO_DATA;
//    LOG_I(TAG, "MDIO_ADDR: 0x%08lX", mdio_addr);
//    LOG_I(TAG, "MDIO_DATA: 0x%08lX", mdio_data);
//
//    // Test MDIO với các PHY address
//    LOG_I(TAG, "Scanning PHY addresses...");
//    for(uint8 addr = 0; addr < 32; addr++) {
//        status = Gmac_Ip_MDIORead(0, addr, 0x02, &data, FALSE);
//        if(status == GMAC_STATUS_SUCCESS && data != 0xFFFF && data != 0) {
//            LOG_I(TAG, "PHY found at addr %d: ID=0x%04X", addr, data);
//        }
//    }
//
//    LOG_I(TAG, "=== Test Complete ===");
//}
//
//void Test_LAN9646_PeriodicRead(void)
//{
//    // Empty
//}
/* src/test_lan9646.c */
#include "Gmac_Ip.h"
#include "Gmac_Ip_Cfg.h"
#include "log_debug.h"
#include "systick.h"

static const char* TAG = "LAN9646";

extern GMAC_Type* const Gmac_apxBases[FEATURE_GMAC_NUM_INSTANCES];

#define GMAC_MAC_MDIO_ADDRESS_CR_SHIFT   (8U)

void Test_LAN9646_Init(void)
{
    Gmac_Ip_StatusType status;
    uint16 data = 0;
    GMAC_Type *base = Gmac_apxBases[0];

    LOG_I(TAG, "=== LAN9646 Test ===");

    status = Gmac_Ip_Init(0, &Gmac_0_ConfigPB);
    LOG_I(TAG, "Init: %d", status);

    Gmac_Ip_EnableController(0);

    // MDIO = 3.6MHz với CSR=4
    // → GMAC_CLK = 3.6MHz * (divider)
    // CSR=4: div=102 → 3.6*102 = 367MHz (quá cao!)
    // Thử CSR=5 (div=124) để giảm xuống

    LOG_I(TAG, "Current MDIO = 3.6MHz (too fast!)");
    LOG_I(TAG, "Trying higher CSR for slower MDIO...");

    // Thử CSR từ cao xuống thấp
    uint8 csr_test[] = {5, 4, 3, 2, 1, 0};

    for(uint8 i = 0; i < 6; i++) {
        uint8 csr = csr_test[i];
        LOG_I(TAG, "");
        LOG_I(TAG, "Testing CSR=%d", csr);

        base->MAC_MDIO_ADDRESS = (csr << GMAC_MAC_MDIO_ADDRESS_CR_SHIFT);

        // Longer delay
        for(volatile uint32 d = 0; d < 100000; d++);

        for(uint8 addr = 0; addr < 8; addr++) {
            status = Gmac_Ip_MDIORead(0, addr, 0x02, &data, FALSE);

            if(status == GMAC_STATUS_SUCCESS && data != 0xFFFF && data != 0) {
                LOG_I(TAG, ">>> PHY at addr=%d, ID=0x%04X", addr, data);
                LOG_I(TAG, ">>> CSR=%d works!", csr);
                return; // Found!
            }
        }
    }

    LOG_E(TAG, "No PHY found with any CSR");
    LOG_I(TAG, "Check hardware: MDC/MDIO wiring, LAN9646 power");
}

void Test_LAN9646_PeriodicRead(void) {}
