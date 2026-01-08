/* src/test_lan9646.c */
#include "Gmac_Ip.h"
#include "Gmac_Ip_Cfg.h"
#include "log_debug.h"
#include "systick.h"

static const char* TAG = "LAN9646";

extern GMAC_Type* const Gmac_apxBases[FEATURE_GMAC_NUM_INSTANCES];

#define GMAC_MAC_MDIO_ADDRESS_CR_SHIFT   (8U)
#define GMAC_MAC_MDIO_ADDRESS_PA_SHIFT   (21U)
#define GMAC_MAC_MDIO_ADDRESS_RDA_SHIFT  (16U)
#define GMAC_MAC_MDIO_ADDRESS_GOC_SHIFT  (2U)
#define GMAC_MAC_MDIO_ADDRESS_GB_MASK    (0x00000001U)

boolean Custom_MDIO_Read(uint8 phy_addr, uint8 reg_addr, uint16* data)
{
    GMAC_Type *base = Gmac_apxBases[0];
    uint32 mdio_addr;
    uint32 timeout;

    mdio_addr = (2U << GMAC_MAC_MDIO_ADDRESS_CR_SHIFT) |
                (((uint32)phy_addr) << GMAC_MAC_MDIO_ADDRESS_PA_SHIFT) |
                (((uint32)reg_addr) << GMAC_MAC_MDIO_ADDRESS_RDA_SHIFT) |
                (3U << GMAC_MAC_MDIO_ADDRESS_GOC_SHIFT) |
                GMAC_MAC_MDIO_ADDRESS_GB_MASK;

    base->MAC_MDIO_ADDRESS = mdio_addr;

    timeout = 1000000;
    while((base->MAC_MDIO_ADDRESS & GMAC_MAC_MDIO_ADDRESS_GB_MASK) && (timeout > 0)) {
        timeout--;
    }

    if(timeout == 0) {
        return FALSE;
    }

    *data = (uint16)(base->MAC_MDIO_DATA & 0xFFFFU);
    return TRUE;
}

void Test_LAN9646_Init(void)
{
    Gmac_Ip_StatusType status;
    uint16 data = 0;
    GMAC_Type *base = Gmac_apxBases[0];

    LOG_I(TAG, "=== LAN9646 Init ===");

    status = Gmac_Ip_Init(0, &Gmac_0_ConfigPB);
    LOG_I(TAG, "GMAC Init: %d", status);

    Gmac_Ip_EnableController(0);

    base->MAC_MDIO_ADDRESS = (2 << GMAC_MAC_MDIO_ADDRESS_CR_SHIFT);
    SysTick_DelayMs(100);

    LOG_I(TAG, "LAN9646 PHY Status:");
    LOG_I(TAG, "");

    for(uint8 addr = 0; addr <= 5; addr++) {
        if(Custom_MDIO_Read(addr, 0x01, &data)) {
            boolean link = (data & 0x04) ? TRUE : FALSE;
            const char* port_name;

            if(addr == 0) port_name = "Switch";
            else port_name = "Port";

            LOG_I(TAG, "%s %d: %s", port_name, addr, link ? "UP ✓" : "DOWN");
        }
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "=== LAN9646 Ready ===");
}

void Test_LAN9646_PeriodicRead(void)
{
    static uint32 lastTick = 0;
    uint16 bmsr;

    if(SysTick_GetTick() - lastTick >= 2000) {
        lastTick = SysTick_GetTick();

        LOG_I(TAG, "Link Status:");
        for(uint8 addr = 1; addr <= 5; addr++) {
            if(Custom_MDIO_Read(addr, 0x01, &bmsr)) {
                LOG_I(TAG, "  Port %d: %s", addr, (bmsr & 0x04) ? "UP" : "DOWN");
            }
        }
    }
}
