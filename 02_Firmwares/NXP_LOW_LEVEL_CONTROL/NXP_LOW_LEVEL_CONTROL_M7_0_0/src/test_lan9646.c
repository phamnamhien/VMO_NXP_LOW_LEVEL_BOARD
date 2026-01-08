/* src/test_lan9646.c */
#include "Gmac_Ip.h"
#include "Gmac_Ip_Cfg.h"
#include "lan9646_tx.h"
#include "log_debug.h"
#include "systick.h"
#include "string.h"

static const char* TAG = "LAN9646";

extern GMAC_Type* const Gmac_apxBases[FEATURE_GMAC_NUM_INSTANCES];

#define GMAC_MAC_MDIO_ADDRESS_CR_SHIFT   (8U)
#define GMAC_MAC_MDIO_ADDRESS_PA_SHIFT   (21U)
#define GMAC_MAC_MDIO_ADDRESS_RDA_SHIFT  (16U)
#define GMAC_MAC_MDIO_ADDRESS_GOC_SHIFT  (2U)

// MAC address for this device
static uint8 MyMAC[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
static uint8 MyIP[4] = {192, 168, 1, 100};

//// RX/TX buffers
//static uint8 RxBuffer[1536];
//static uint8 TxBuffer[1536];




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

boolean Custom_MDIO_Write(uint8 phy_addr, uint8 reg_addr, uint16 data)
{
    GMAC_Type *base = Gmac_apxBases[0];
    uint32 mdio_addr;
    uint32 timeout;

    base->MAC_MDIO_DATA = data;

    mdio_addr = (2U << GMAC_MAC_MDIO_ADDRESS_CR_SHIFT) |
                (((uint32)phy_addr) << GMAC_MAC_MDIO_ADDRESS_PA_SHIFT) |
                (((uint32)reg_addr) << GMAC_MAC_MDIO_ADDRESS_RDA_SHIFT) |
                (1U << GMAC_MAC_MDIO_ADDRESS_GOC_SHIFT) |
                GMAC_MAC_MDIO_ADDRESS_GB_MASK;

    base->MAC_MDIO_ADDRESS = mdio_addr;

    timeout = 1000000;
    while((base->MAC_MDIO_ADDRESS & GMAC_MAC_MDIO_ADDRESS_GB_MASK) && (timeout > 0)) {
        timeout--;
    }

    return (timeout != 0);
}

void LAN9646_Configure(void)
{
    uint16 data;

    LOG_I(TAG, "Configuring LAN9646...");

    if(Custom_MDIO_Read(0, 0x00, &data)) {
        LOG_I(TAG, "  Chip ID: 0x%04X", data);
    }

    for(uint8 port = 1; port <= 5; port++) {
        if(Custom_MDIO_Read(port, 0x00, &data)) {
            data |= 0x1000;
            Custom_MDIO_Write(port, 0x00, data);
        }
    }

    LOG_I(TAG, "  Switch configured");
}

uint16 CalculateChecksum(uint16* data, uint16 len)
{
    uint32 sum = 0;

    while(len > 1) {
        sum += *data++;
        len -= 2;
    }

    if(len == 1) {
        sum += *(uint8*)data;
    }

    while(sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16)(~sum);
}

void SendARPReply(uint8* targetMAC, uint8* targetIP)
{
	static uint8 TxBuffer[1536];
    uint16 len = 0;
    Gmac_Ip_BufferType txBuff;
    Gmac_Ip_TxOptionsType txOpt = {0};

    memcpy(&TxBuffer[0], targetMAC, 6);
    memcpy(&TxBuffer[6], MyMAC, 6);
    TxBuffer[12] = 0x08; TxBuffer[13] = 0x06;

    TxBuffer[14] = 0x00; TxBuffer[15] = 0x01;
    TxBuffer[16] = 0x08; TxBuffer[17] = 0x00;
    TxBuffer[18] = 0x06;
    TxBuffer[19] = 0x04;
    TxBuffer[20] = 0x00; TxBuffer[21] = 0x02;

    memcpy(&TxBuffer[22], MyMAC, 6);
    memcpy(&TxBuffer[28], MyIP, 4);
    memcpy(&TxBuffer[32], targetMAC, 6);
    memcpy(&TxBuffer[38], targetIP, 4);

    len = 42;
    while(len < 60) {
        TxBuffer[len++] = 0x00;
    }

    txBuff.Data = TxBuffer;
    txBuff.Length = len;

    Gmac_Ip_SendFrame(0, 0, &txBuff, &txOpt);

    LOG_I(TAG, "ARP reply -> %d.%d.%d.%d",
          targetIP[0], targetIP[1], targetIP[2], targetIP[3]);
}

void SendPingReply(uint8* destMAC, uint8* destIP, uint8* icmpData, uint16 icmpLen)
{
	static uint8 TxBuffer[1536];
    uint16 len = 0;
    uint16 ipLen = 20 + icmpLen;
    Gmac_Ip_BufferType txBuff;
    Gmac_Ip_TxOptionsType txOpt = {0};

    memcpy(&TxBuffer[0], destMAC, 6);
    memcpy(&TxBuffer[6], MyMAC, 6);
    TxBuffer[12] = 0x08; TxBuffer[13] = 0x00;

    TxBuffer[14] = 0x45;
    TxBuffer[15] = 0x00;
    TxBuffer[16] = (ipLen >> 8) & 0xFF;
    TxBuffer[17] = ipLen & 0xFF;
    TxBuffer[18] = 0x00; TxBuffer[19] = 0x00;
    TxBuffer[20] = 0x00; TxBuffer[21] = 0x00;
    TxBuffer[22] = 0x40;
    TxBuffer[23] = 0x01;
    TxBuffer[24] = 0x00; TxBuffer[25] = 0x00;
    memcpy(&TxBuffer[26], MyIP, 4);
    memcpy(&TxBuffer[30], destIP, 4);

    uint16 ipChecksum = CalculateChecksum((uint16*)&TxBuffer[14], 20);
    TxBuffer[24] = (ipChecksum >> 8) & 0xFF;
    TxBuffer[25] = ipChecksum & 0xFF;

    TxBuffer[34] = 0x00;
    TxBuffer[35] = 0x00;
    TxBuffer[36] = 0x00; TxBuffer[37] = 0x00;

    memcpy(&TxBuffer[38], icmpData, icmpLen - 4);

    uint16 icmpChecksum = CalculateChecksum((uint16*)&TxBuffer[34], icmpLen);
    TxBuffer[36] = (icmpChecksum >> 8) & 0xFF;
    TxBuffer[37] = icmpChecksum & 0xFF;

    len = 14 + ipLen;
    while(len < 60) {
        TxBuffer[len++] = 0x00;
    }

    txBuff.Data = TxBuffer;
    txBuff.Length = len;

    Gmac_Ip_SendFrame(0, 0, &txBuff, &txOpt);

    LOG_I(TAG, "PING reply -> %d.%d.%d.%d",
          destIP[0], destIP[1], destIP[2], destIP[3]);
}

void ProcessRxFrame(uint8* frame, uint16 len)
{
    uint16 etherType;

    if(len < 14) return;

    etherType = (frame[12] << 8) | frame[13];

    boolean forUs = (frame[0] == 0xFF) || (memcmp(&frame[0], MyMAC, 6) == 0);

    if(!forUs) return;

    if(etherType == 0x0806) {
        if(frame[20] == 0x00 && frame[21] == 0x01) {
            if(memcmp(&frame[38], MyIP, 4) == 0) {
                uint8 senderMAC[6], senderIP[4];
                memcpy(senderMAC, &frame[22], 6);
                memcpy(senderIP, &frame[28], 4);

                LOG_I(TAG, "ARP from %d.%d.%d.%d",
                      senderIP[0], senderIP[1], senderIP[2], senderIP[3]);

                SendARPReply(senderMAC, senderIP);
            }
        }
    }
    else if(etherType == 0x0800) {
        uint8 protocol = frame[23];

        if(memcmp(&frame[30], MyIP, 4) != 0) return;

        if(protocol == 0x01) {
            uint8 icmpType = frame[34];

            if(icmpType == 0x08) {
                uint8 srcMAC[6], srcIP[4];
                uint16 ipLen = (frame[16] << 8) | frame[17];
                uint16 icmpLen = ipLen - 20;

                memcpy(srcMAC, &frame[6], 6);
                memcpy(srcIP, &frame[26], 4);

                LOG_I(TAG, "PING from %d.%d.%d.%d",
                      srcIP[0], srcIP[1], srcIP[2], srcIP[3]);

                SendPingReply(srcMAC, srcIP, &frame[38], icmpLen - 4);
            }
        }
    }
}

void Test_LAN9646_SendTestPacket(void)
{
    static uint8_t testPacket[64]; // Minimum Ethernet frame

    // Dest MAC (broadcast)
    testPacket[0] = 0xFF; testPacket[1] = 0xFF;
    testPacket[2] = 0xFF; testPacket[3] = 0xFF;
    testPacket[4] = 0xFF; testPacket[5] = 0xFF;

    // Source MAC
    testPacket[6] = 0x00; testPacket[7] = 0x11;
    testPacket[8] = 0x22; testPacket[9] = 0x33;
    testPacket[10] = 0x44; testPacket[11] = 0x55;

    // EtherType
    testPacket[12] = 0x08; testPacket[13] = 0x00;

    // Payload (50 bytes)
    for(int i = 14; i < 64; i++) {
        testPacket[i] = i;
    }

    Gmac_Ip_BufferType txBuff;
    Gmac_Ip_TxOptionsType txOpt = {0};

    txBuff.Data = testPacket;
    txBuff.Length = 64;

    Gmac_Ip_StatusType status = Gmac_Ip_SendFrame(0, 0, &txBuff, &txOpt);

    LOG_I(TAG, "NXP TX: %d", status);
}

void LAN9646_ConfigurePHY(void)
{
    uint16_t data;

    LOG_I(TAG, "=== Configure PHY ===");

    // ĐỪNG GHI GÌ - chỉ đọc
    Custom_MDIO_Read(1, 0x01, &data);
    LOG_I(TAG, "Port 1 BMSR: 0x%04X", data);
    LOG_I(TAG, "  Link: %s", (data & 0x04) ? "UP" : "DOWN");

    Custom_MDIO_Read(2, 0x01, &data);
    LOG_I(TAG, "Port 2 BMSR: 0x%04X", data);
    LOG_I(TAG, "  Link: %s", (data & 0x04) ? "UP" : "DOWN");
}


void LAN9646_CheckPHYStatus(void)
{
    uint16_t data;

    LOG_I(TAG, "=== PHY Status Check ===");

    // Port 1
    if(Custom_MDIO_Read(1, 0x01, &data)) {
        LOG_I(TAG, "Port 1 BMSR: 0x%04X", data);
        LOG_I(TAG, "  Link: %s", (data & 0x04) ? "UP" : "DOWN");
        LOG_I(TAG, "  AutoNeg: %s", (data & 0x20) ? "Complete" : "Incomplete");
    }

    // Port 2
    if(Custom_MDIO_Read(2, 0x01, &data)) {
        LOG_I(TAG, "Port 2 BMSR: 0x%04X", data);
        LOG_I(TAG, "  Link: %s", (data & 0x04) ? "UP" : "DOWN");
    }

    // Check GMAC MAC config
    GMAC_Type *base = Gmac_apxBases[0];
    LOG_I(TAG, "MAC_CONFIG: 0x%08X", base->MAC_CONFIGURATION);
    LOG_I(TAG, "  TX Enable: %s", (base->MAC_CONFIGURATION & (1<<0)) ? "YES" : "NO");
    LOG_I(TAG, "  Speed: %s", (base->MAC_CONFIGURATION & (1<<14)) ? "1000M" :
                               (base->MAC_CONFIGURATION & (1<<15)) ? "10M" : "100M");
    LOG_I(TAG, "  Duplex: %s", (base->MAC_CONFIGURATION & (1<<13)) ? "FULL" : "HALF");
}
void LAN9646_ConfigureSwitch(void)
{
    uint16_t data;

    LOG_I(TAG, "=== Configure LAN9646 Switch ===");

    if(Custom_MDIO_Read(0, 0x02, &data)) {
        LOG_I(TAG, "Switch ID1: 0x%04X", data);
    }
    if(Custom_MDIO_Read(0, 0x03, &data)) {
        LOG_I(TAG, "Switch ID2: 0x40FE", data);
    }

    Custom_MDIO_Read(1, 0x00, &data);
    LOG_I(TAG, "Port 1 BMCR: 0x%04X", data);

    Custom_MDIO_Read(1, 0x01, &data);
    LOG_I(TAG, "Port 1 BMSR: 0x%04X", data);

    // THÊM: Đọc Port Control registers
    Custom_MDIO_Read(1, 0x04, &data);
    LOG_I(TAG, "Port 1 Reg 0x04: 0x%04X", data);

    Custom_MDIO_Read(1, 0x05, &data);
    LOG_I(TAG, "Port 1 Reg 0x05: 0x%04X", data);

    // Switch control
    Custom_MDIO_Read(0, 0x00, &data);
    LOG_I(TAG, "Switch Reg 0x00: 0x%04X", data);

    Custom_MDIO_Read(0, 0x01, &data);
    LOG_I(TAG, "Switch Reg 0x01: 0x%04X", data);
}

void Test_LAN9646_Init(void)
{
    Gmac_Ip_StatusType status;
    GMAC_Type *base = Gmac_apxBases[0];

    LOG_I(TAG, "=== Init START ===");

    status = Gmac_Ip_Init(0, &Gmac_0_ConfigPB);
    LOG_I(TAG, "GMAC Init: %d", status);

    Gmac_Ip_EnableController(0);
    base->MAC_MDIO_ADDRESS = (2 << 8);

    LOG_I(TAG, "Wait 5s for PHY...");
    SysTick_DelayMs(5000);  // ← Tăng từ 3s lên 5s

    LAN9646_ConfigureSwitch();
    LAN9646_ConfigurePHY();

    LAN9646_TX_Init();

    LOG_I(TAG, "=== Init DONE ===");
}

void Test_LAN9646_PeriodicRead(void)
{
    static uint32_t lastTx = 0;
    static uint32_t count = 0;
    GMAC_Type *gmac = Gmac_apxBases[0];

    if(SysTick_GetTick() - lastTx >= 2000) {
        lastTx = SysTick_GetTick();

        LOG_I(TAG, "--- Loop %d ---", count);

        // CHECK RX COUNTER
        uint32_t rx_pkt = gmac->RX_PACKETS_COUNT_GOOD_BAD;
        LOG_I(TAG, "RX counter: %d", rx_pkt);
        // THÊM: Đọc Port Control registers
        uint16_t data;
        Custom_MDIO_Read(4, 0x80, &data);
        LOG_I(TAG, "Port 4 Reg 0x80: 0x%04X", data);

        // TX test
        uint32_t tx_pkt_before = gmac->TX_PACKET_COUNT_GOOD_BAD;
        Test_LAN9646_SendTestPacket();
        SysTick_DelayMs(10);
        uint32_t tx_pkt_after = gmac->TX_PACKET_COUNT_GOOD_BAD;

        if(tx_pkt_after > tx_pkt_before) {
            LOG_I(TAG, "TX counter: %d -> %d", tx_pkt_before, tx_pkt_after);
        }

        count++;
    }
}
