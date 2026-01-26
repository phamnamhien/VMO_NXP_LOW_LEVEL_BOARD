# RGMII 1Gbps Configuration Notes
## S32K388 GMAC + LAN9646 Port 6

### Overview
This document describes the configuration required to establish RGMII 1Gbps communication between S32K388 GMAC and LAN9646 (KSZ9477 family) Port 6.

---

## 1. LAN9646 Switch Configuration

### 1.1 Port 6 RGMII Settings

| Register | Address | Value | Description |
|----------|---------|-------|-------------|
| XMII_CTRL0 | 0x6300 | 0x68 | Full duplex mode |
| XMII_CTRL1 | 0x6301 | 0x18 | 1Gbps + TX_ID + RX_ID (internal delay enabled) |

**Important:** TX_ID and RX_ID bits in XMII_CTRL1 enable internal 2ns delay for RGMII timing. This is **required** for proper data reception.

### 1.2 Switch Global Settings

| Register | Address | Value | Description |
|----------|---------|-------|-------------|
| Switch Operation | 0x0300 | 0x01 | Enable switch |

### 1.3 Port Membership (VLAN)

| Port | Register | Value | Description |
|------|----------|-------|-------------|
| Port 6 | 0x6A04 | 0x4F | Port 6 membership |
| Port 1 | 0x1A04 | 0x6E | Port 1 membership |
| Port 2 | 0x2A04 | 0x6D | Port 2 membership |
| Port 3 | 0x3A04 | 0x6B | Port 3 membership |
| Port 4 | 0x4A04 | 0x67 | Port 4 membership |

### 1.4 I2C Communication

- **I2C Address:** 0x5F
- **Memory Address Size:** 16-bit (2 bytes)
- **Interface:** Software I2C via GPIO

---

## 2. S32K388 GMAC Configuration

### 2.1 DCM_GPR Register Settings

#### DCMRWF1 - RGMII Mode Selection
```c
uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
dcmrwf1 = (dcmrwf1 & ~0x03U) | 0x01U;  // Bits 0-1: RGMII mode (0x01)
dcmrwf1 |= (1U << 6);                   // Bit 6: Enable
IP_DCM_GPR->DCMRWF1 = dcmrwf1;
```

#### DCMRWF3 - Clock Configuration
```c
uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
dcmrwf3 |= (1U << 13);  // Bit 13: RX_CLK bypass
dcmrwf3 |= (1U << 11);  // Bit 11: TX_CLK output enable
IP_DCM_GPR->DCMRWF3 = dcmrwf3;
```

### 2.2 MAC_CONFIGURATION Register

```c
uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
mac_cfg &= ~(1U << 15);  // PS = 0 (Port Select: Gigabit)
mac_cfg &= ~(1U << 14);  // FES = 0 (Speed: with PS=0 means 1Gbps)
mac_cfg |= (1U << 13);   // DM = 1 (Full Duplex Mode)
mac_cfg |= (1U << 0);    // RE = 1 (Receiver Enable)
mac_cfg |= (1U << 1);    // TE = 1 (Transmitter Enable)
IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;
```

**Speed Configuration Table:**

| PS | FES | Speed |
|----|-----|-------|
| 0  | 0   | 1 Gbps |
| 1  | 0   | 10 Mbps |
| 1  | 1   | 100 Mbps |

---

## 3. EB Tresos Configuration

### 3.1 Network Settings (tcp_stack_1)

| Parameter | Value |
|-----------|-------|
| IPv4 Address | 192.168.1.200 |
| Subnet Mask | 255.255.255.0 |
| Gateway | 192.168.1.1 |

### 3.2 MAC Address (Gmac_Ip_PBcfg)

| Parameter | Value |
|-----------|-------|
| MAC Address | 10:11:22:77:77:77 |

---

## 4. TX Buffer Configuration (Critical!)

### 4.1 Non-Cacheable Memory Section

TX buffer **MUST** be placed in non-cacheable memory for DMA access:

```c
/* TX buffer - place in non-cacheable section for DMA access */
#define ETH_43_GMAC_START_SEC_VAR_CLEARED_UNSPECIFIED_NO_CACHEABLE
#include "Eth_43_GMAC_MemMap.h"
static uint8_t g_tx_buffer[1536] __attribute__((aligned(8)));
#define ETH_43_GMAC_STOP_SEC_VAR_CLEARED_UNSPECIFIED_NO_CACHEABLE
#include "Eth_43_GMAC_MemMap.h"
```

### 4.2 Direct SendFrame (No GetTxBuff)

Driver is configured **without** internal TX buffers. Must use static buffer directly:

```c
static Gmac_Ip_StatusType send_packet_data(const uint8_t* data, uint16_t len) {
    Gmac_Ip_BufferType buf;

    /* Copy to DMA-accessible buffer */
    if (data != g_tx_buffer) {
        memcpy(g_tx_buffer, data, len);
    }

    buf.Data = g_tx_buffer;
    buf.Length = len;

    /* Retry if queue full */
    int retries = 20;
    while (retries > 0) {
        Gmac_Ip_StatusType status = Gmac_Ip_SendFrame(0, 0, &buf, NULL);
        if (status == GMAC_STATUS_SUCCESS) {
            return status;
        }
        if (status != GMAC_STATUS_TX_QUEUE_FULL) {
            return status;  // Other error
        }
        delay_ms(1);
        retries--;
    }
    return GMAC_STATUS_TX_QUEUE_FULL;
}
```

---

## 5. RX Buffer Configuration

RX uses driver's internal ring buffers:

```c
static void poll_rx(void) {
    Gmac_Ip_BufferType buf;
    Gmac_Ip_RxInfoType rx_info;

    Gmac_Ip_StatusType status = Gmac_Ip_ReadFrame(0, 0, &buf, &rx_info);

    if (status == GMAC_STATUS_SUCCESS) {
        /* Process packet from buf.Data with length rx_info.PktLen */
        process_rx_packet(buf.Data, rx_info.PktLen);

        /* Return buffer to driver */
        Gmac_Ip_ProvideRxBuff(0, 0, &buf);
    }
}
```

---

## 6. Initialization Sequence

```c
int main(void) {
    /* 1. MCU Init */
    OsIf_Init(NULL_PTR);
    Port_Init(NULL_PTR);
    Mcu_Init(NULL_PTR);
    Mcu_InitClock(McuClockSettingConfig_0);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {}
    Mcu_DistributePllClock();
    Mcu_SetMode(McuModeSettingConf_0);
    Platform_Init(NULL_PTR);

    /* 2. GPT for timing */
    Gpt_Init(NULL_PTR);
    Gpt_StartTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0, 40000U);
    Gpt_EnableNotification(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);

    /* 3. UART (optional for debug) */
    Uart_Init(NULL_PTR);

    /* 4. LAN9646 Init via I2C */
    lan9646_init(&g_lan9646, &cfg);
    lan9646_write_reg8(&g_lan9646, 0x6300, 0x68);  // XMII_CTRL0
    lan9646_write_reg8(&g_lan9646, 0x6301, 0x18);  // XMII_CTRL1
    lan9646_write_reg8(&g_lan9646, 0x0300, 0x01);  // Enable switch

    /* 5. GMAC Init */
    Eth_43_GMAC_Init(&Eth_43_GMAC_xPredefinedConfig);
    configure_gmac_1gbps();  // Set MAC_CONFIGURATION
    Eth_43_GMAC_SetControllerMode(0, ETH_MODE_ACTIVE);
    configure_s32k388_rgmii();  // Set DCM_GPR registers

    /* 6. Wait for link */
    delay_ms(100);

    /* 7. Main loop */
    while (1) {
        poll_rx();
        // ... other tasks
        delay_ms(1);
    }
}
```

---

## 7. Troubleshooting

### 7.1 TX Not Working
- Check TX buffer is in non-cacheable memory section
- Don't use `Gmac_Ip_GetTxBuff` - driver has no internal TX buffers
- Add retry logic for TX_QUEUE_FULL

### 7.2 RX Not Working
- Verify LAN9646 XMII_CTRL1 has TX_ID and RX_ID bits set (0x18)
- Check DCM_GPR DCMRWF3 settings

### 7.3 Invalid Buffer Address (e.g., 0xC701A8C0)
- This indicates memory corruption or TX ring overflow
- Validate buffer address is in SRAM range (0x20400000 - 0x2047FFFF)
- Add delay between TX packets

---

## 8. Test Results

```
Ping statistics for 192.168.1.200:
    Packets: Sent = 4, Received = 4, Lost = 0 (0% loss)
    Minimum = 10ms, Maximum = 21ms, Average = 13ms
```

---

## Version History

| Date | Author | Description |
|------|--------|-------------|
| 2025-01-26 | Claude AI | Initial documentation |
