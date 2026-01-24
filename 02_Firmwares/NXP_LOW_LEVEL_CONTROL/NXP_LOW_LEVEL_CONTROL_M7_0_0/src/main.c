/**
 * @file    main.c
 * @brief   RGMII Network Application - S32K388 GMAC + LAN9646 Port 6
 *          - Broadcast every 5 seconds
 *          - Respond to ICMP ping
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
#include "CDD_Uart.h"
#include "log_debug.h"

/* External config symbols from generated PBcfg files */
extern const Eth_43_GMAC_ConfigType Eth_43_GMAC_xPredefinedConfig;

/* GPT notification stub - required by Gpt_PBcfg.c */
void SysTick_Custom_Handler(void) {
    /* Not used in baremetal mode */
}

#define TAG "NET"

/*===========================================================================*/
/*                          NETWORK CONFIGURATION                             */
/*===========================================================================*/

/* Our MAC address (as configured in EB Tresos) */
static const uint8_t g_our_mac[6] = {0x10, 0x11, 0x22, 0x77, 0x77, 0x77};

/* Our IP address: 192.168.1.200 (as configured in EB Tresos tcp_stack_1) */
static const uint8_t g_our_ip[4] = {192, 168, 1, 200};

/* Broadcast MAC */
static const uint8_t g_bcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* Broadcast IP */
static const uint8_t g_bcast_ip[4] = {255, 255, 255, 255};

/*===========================================================================*/
/*                          HARDWARE CONFIGURATION                            */
/*===========================================================================*/

#define LAN9646_SCL_CHANNEL     DioConf_DioChannel_SCL_CH
#define LAN9646_SDA_CHANNEL     DioConf_DioChannel_SDA_CH
#define LAN9646_I2C_SPEED       5U
#define ETH_CTRL_IDX            0U

/* Ethernet frame types */
#define ETH_TYPE_ARP            0x0806
#define ETH_TYPE_IP             0x0800

/* IP protocols */
#define IP_PROTO_ICMP           1
#define IP_PROTO_UDP            17

/* ICMP types */
#define ICMP_ECHO_REQUEST       8
#define ICMP_ECHO_REPLY         0

/*===========================================================================*/
/*                          GLOBAL VARIABLES                                  */
/*===========================================================================*/

static lan9646_t g_lan9646;
static softi2c_t g_i2c;

/* TX buffer */
static uint8_t g_tx_buffer[1536];

/* Statistics */
static uint32_t g_rx_count = 0;
static uint32_t g_tx_count = 0;
static uint32_t g_ping_count = 0;
static uint32_t g_arp_count = 0;

/*===========================================================================*/
/*                          DELAY FUNCTIONS                                   */
/*===========================================================================*/

static void delay_ms(uint32_t ms) {
    volatile uint32_t count;
    while (ms > 0) {
        count = 40000U;
        while (count > 0) {
            count--;
        }
        ms--;
    }
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
/*                          LAN9646 HELPERS                                   */
/*===========================================================================*/

static void lan_write8(uint16_t addr, uint8_t val) {
    lan9646_write_reg8(&g_lan9646, addr, val);
}

static void lan_write32(uint16_t addr, uint32_t val) {
    lan9646_write_reg32(&g_lan9646, addr, val);
}

/*===========================================================================*/
/*                          CHECKSUM HELPERS                                  */
/*===========================================================================*/

static uint16_t ip_checksum(const uint8_t* data, uint16_t len) {
    uint32_t sum = 0;

    while (len > 1) {
        sum += ((uint16_t)data[0] << 8) | data[1];
        data += 2;
        len -= 2;
    }

    if (len == 1) {
        sum += (uint16_t)data[0] << 8;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)(~sum);
}

/*===========================================================================*/
/*                          PACKET SEND FUNCTIONS                             */
/*===========================================================================*/

static void send_packet(uint8_t* data, uint16_t len) {
    Gmac_Ip_BufferType buf;
    buf.Data = data;
    buf.Length = len;

    Gmac_Ip_SendFrame(0, 0, &buf, NULL);
    g_tx_count++;
}

/* Send UDP broadcast packet */
static void send_broadcast(void) {
    static uint32_t seq = 0;
    uint8_t* pkt = g_tx_buffer;
    uint16_t len = 0;

    seq++;

    /* Ethernet header (14 bytes) */
    memcpy(&pkt[0], g_bcast_mac, 6);      /* Dest MAC: broadcast */
    memcpy(&pkt[6], g_our_mac, 6);        /* Src MAC: our MAC */
    pkt[12] = 0x08; pkt[13] = 0x00;       /* EtherType: IP */
    len = 14;

    /* IP header (20 bytes) */
    uint8_t* ip = &pkt[14];
    ip[0] = 0x45;                         /* Version 4, IHL 5 */
    ip[1] = 0x00;                         /* DSCP/ECN */
    ip[2] = 0x00; ip[3] = 50;             /* Total length: 20 + 8 + 22 = 50 */
    ip[4] = (seq >> 8); ip[5] = seq;      /* ID */
    ip[6] = 0x00; ip[7] = 0x00;           /* Flags/Fragment */
    ip[8] = 64;                           /* TTL */
    ip[9] = IP_PROTO_UDP;                 /* Protocol: UDP */
    ip[10] = 0; ip[11] = 0;               /* Checksum (calculate later) */
    memcpy(&ip[12], g_our_ip, 4);         /* Src IP */
    memcpy(&ip[16], g_bcast_ip, 4);       /* Dst IP: broadcast */

    /* IP checksum */
    uint16_t ip_csum = ip_checksum(ip, 20);
    ip[10] = ip_csum >> 8;
    ip[11] = ip_csum & 0xFF;
    len += 20;

    /* UDP header (8 bytes) */
    uint8_t* udp = &pkt[34];
    udp[0] = 0x13; udp[1] = 0x88;         /* Src port: 5000 */
    udp[2] = 0x13; udp[3] = 0x88;         /* Dst port: 5000 */
    udp[4] = 0x00; udp[5] = 30;           /* Length: 8 + 22 = 30 */
    udp[6] = 0x00; udp[7] = 0x00;         /* Checksum: 0 (optional for IPv4) */
    len += 8;

    /* Payload: "S32K388 Hello #XXXX" */
    uint8_t* payload = &pkt[42];
    int plen = snprintf((char*)payload, 100, "S32K388 Hello #%lu", (unsigned long)seq);
    len += plen;

    /* Pad to minimum 60 bytes */
    while (len < 60) {
        pkt[len++] = 0;
    }

    send_packet(pkt, len);
    LOG_I(TAG, "TX Broadcast #%lu", (unsigned long)seq);
}

/*===========================================================================*/
/*                          ARP HANDLER                                       */
/*===========================================================================*/

static void handle_arp(uint8_t* pkt, uint16_t len) {
    if (len < 42) return;

    uint8_t* arp = &pkt[14];
    uint16_t opcode = ((uint16_t)arp[6] << 8) | arp[7];

    /* Only handle ARP request (opcode 1) */
    if (opcode != 1) return;

    /* Check if target IP is our IP */
    if (memcmp(&arp[24], g_our_ip, 4) != 0) return;

    g_arp_count++;

    /* Get sender info */
    uint8_t sender_mac[6];
    uint8_t sender_ip[4];
    memcpy(sender_mac, &arp[8], 6);
    memcpy(sender_ip, &arp[14], 4);

    LOG_I(TAG, "ARP Request from %d.%d.%d.%d",
          sender_ip[0], sender_ip[1], sender_ip[2], sender_ip[3]);

    /* Build ARP reply */
    uint8_t* reply = g_tx_buffer;

    /* Ethernet header */
    memcpy(&reply[0], sender_mac, 6);     /* Dest MAC */
    memcpy(&reply[6], g_our_mac, 6);      /* Src MAC */
    reply[12] = 0x08; reply[13] = 0x06;   /* EtherType: ARP */

    /* ARP header */
    reply[14] = 0x00; reply[15] = 0x01;   /* Hardware type: Ethernet */
    reply[16] = 0x08; reply[17] = 0x00;   /* Protocol type: IP */
    reply[18] = 6;                         /* Hardware size */
    reply[19] = 4;                         /* Protocol size */
    reply[20] = 0x00; reply[21] = 0x02;   /* Opcode: Reply */
    memcpy(&reply[22], g_our_mac, 6);     /* Sender MAC (our MAC) */
    memcpy(&reply[28], g_our_ip, 4);      /* Sender IP (our IP) */
    memcpy(&reply[32], sender_mac, 6);    /* Target MAC */
    memcpy(&reply[38], sender_ip, 4);     /* Target IP */

    /* Pad to 60 bytes */
    memset(&reply[42], 0, 18);

    send_packet(reply, 60);
    LOG_I(TAG, "ARP Reply sent");
}

/*===========================================================================*/
/*                          ICMP PING HANDLER                                 */
/*===========================================================================*/

static void handle_icmp(uint8_t* pkt, uint16_t len) {
    if (len < 42) return;  /* Min: 14 eth + 20 ip + 8 icmp */

    uint8_t* ip = &pkt[14];
    uint8_t ip_hdr_len = (ip[0] & 0x0F) * 4;
    uint8_t* icmp = &pkt[14 + ip_hdr_len];

    /* Check ICMP type */
    if (icmp[0] != ICMP_ECHO_REQUEST) return;

    /* Check if destination is our IP */
    if (memcmp(&ip[16], g_our_ip, 4) != 0) return;

    g_ping_count++;

    /* Get source info */
    uint8_t src_mac[6];
    uint8_t src_ip[4];
    memcpy(src_mac, &pkt[6], 6);
    memcpy(src_ip, &ip[12], 4);

    LOG_I(TAG, "PING from %d.%d.%d.%d",
          src_ip[0], src_ip[1], src_ip[2], src_ip[3]);

    /* Build ICMP echo reply */
    uint8_t* reply = g_tx_buffer;
    uint16_t total_len = len;

    /* Copy original packet */
    memcpy(reply, pkt, len);

    /* Swap MAC addresses */
    memcpy(&reply[0], src_mac, 6);
    memcpy(&reply[6], g_our_mac, 6);

    /* Swap IP addresses */
    uint8_t* reply_ip = &reply[14];
    memcpy(&reply_ip[12], g_our_ip, 4);   /* Src IP: our IP */
    memcpy(&reply_ip[16], src_ip, 4);     /* Dst IP: sender's IP */

    /* Recalculate IP checksum */
    reply_ip[10] = 0;
    reply_ip[11] = 0;
    uint16_t ip_csum = ip_checksum(reply_ip, ip_hdr_len);
    reply_ip[10] = ip_csum >> 8;
    reply_ip[11] = ip_csum & 0xFF;

    /* Change ICMP type to echo reply */
    uint8_t* reply_icmp = &reply[14 + ip_hdr_len];
    reply_icmp[0] = ICMP_ECHO_REPLY;

    /* Recalculate ICMP checksum */
    uint16_t icmp_len = len - 14 - ip_hdr_len;
    reply_icmp[2] = 0;
    reply_icmp[3] = 0;
    uint16_t icmp_csum = ip_checksum(reply_icmp, icmp_len);
    reply_icmp[2] = icmp_csum >> 8;
    reply_icmp[3] = icmp_csum & 0xFF;

    send_packet(reply, total_len);
    LOG_I(TAG, "PONG sent");
}

/*===========================================================================*/
/*                          PACKET RECEIVE HANDLER                            */
/*===========================================================================*/

static void process_rx_packet(uint8_t* pkt, uint16_t len) {
    if (len < 14) return;

    g_rx_count++;

    /* Get EtherType */
    uint16_t eth_type = ((uint16_t)pkt[12] << 8) | pkt[13];

    switch (eth_type) {
        case ETH_TYPE_ARP:
            handle_arp(pkt, len);
            break;

        case ETH_TYPE_IP:
            /* Check IP protocol */
            if (len >= 34) {
                uint8_t proto = pkt[23];  /* IP protocol field */
                if (proto == IP_PROTO_ICMP) {
                    handle_icmp(pkt, len);
                }
            }
            break;

        default:
            /* Ignore other packet types */
            break;
    }
}

static void poll_rx(void) {
    Gmac_Ip_BufferType buf;
    Gmac_Ip_RxInfoType rx_info;
    Gmac_Ip_StatusType status;

    /* Try to read a frame - driver returns buffer from internal ring */
    status = Gmac_Ip_ReadFrame(0, 0, &buf, &rx_info);

    if (status == GMAC_STATUS_SUCCESS) {
        /* Process the received packet */
        process_rx_packet(buf.Data, rx_info.PktLen);

        /* Return buffer to driver */
        Gmac_Ip_ProvideRxBuff(0, 0, &buf);
    }
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
        LOG_E(TAG, "LAN9646 init FAILED!");
        return lan9646ERR;
    }

    uint16_t chip_id;
    uint8_t revision;
    lan9646_get_chip_id(&g_lan9646, &chip_id, &revision);
    LOG_I(TAG, "  Chip ID: 0x%04X", chip_id);

    /* Configure Port 6 for RGMII 1Gbps */
    lan_write8(0x6300, 0x68);  /* XMII_CTRL0: Full duplex */
    lan_write8(0x6301, 0x18);  /* XMII_CTRL1: 1Gbps + TX_ID + RX_ID */

    /* Enable switch */
    lan_write8(0x0300, 0x01);

    /* Port membership */
    lan_write32(0x6A04, 0x4F);
    lan_write32(0x1A04, 0x6E);
    lan_write32(0x2A04, 0x6D);
    lan_write32(0x3A04, 0x6B);
    lan_write32(0x4A04, 0x67);

    LOG_I(TAG, "LAN9646 OK");
    return lan9646OK;
}

/*===========================================================================*/
/*                          S32K388 RGMII CONFIG                              */
/*===========================================================================*/

static void configure_s32k388_rgmii(void) {
    /* DCMRWF1: RGMII mode */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    dcmrwf1 = (dcmrwf1 & ~0x03U) | 0x01U;
    dcmrwf1 |= (1U << 6);
    IP_DCM_GPR->DCMRWF1 = dcmrwf1;

    /* DCMRWF3: Clock bypass */
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;
    dcmrwf3 |= (1U << 13);  /* RX_CLK bypass */
    dcmrwf3 |= (1U << 11);  /* TX_CLK output */
    IP_DCM_GPR->DCMRWF3 = dcmrwf3;
}

static void configure_gmac_1gbps(void) {
    uint32_t mac_cfg = IP_GMAC_0->MAC_CONFIGURATION;
    mac_cfg &= ~(1U << 15);  /* PS = 0 (1Gbps) */
    mac_cfg &= ~(1U << 14);  /* FES = 0 */
    mac_cfg |= (1U << 13);   /* DM = 1 (Full Duplex) */
    mac_cfg |= (1U << 0);    /* RE = 1 (RX Enable) */
    mac_cfg |= (1U << 1);    /* TE = 1 (TX Enable) */
    IP_GMAC_0->MAC_CONFIGURATION = mac_cfg;
}

/*===========================================================================*/
/*                          MAIN                                              */
/*===========================================================================*/

int main(void) {
    /* MCU Init */
    OsIf_Init(NULL_PTR);
    Port_Init(NULL_PTR);

    Mcu_Init(NULL_PTR);
    Mcu_InitClock(McuClockSettingConfig_0);
    while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) {}
    Mcu_DistributePllClock();
    Mcu_SetMode(McuModeSettingConf_0);

    Platform_Init(NULL_PTR);

    /* GPT for OsIf timing */
    Gpt_Init(NULL_PTR);
    Gpt_StartTimer(GptConf_GptChannelConfiguration_GptChannelConfiguration_0, 40000U);
    Gpt_EnableNotification(GptConf_GptChannelConfiguration_GptChannelConfiguration_0);

    /* UART & Log */
    Uart_Init(NULL_PTR);
    log_init();

    /* Banner */
    LOG_I(TAG, "");
    LOG_I(TAG, "============================================");
    LOG_I(TAG, "  S32K388 Network Application");
    LOG_I(TAG, "  IP: %d.%d.%d.%d", g_our_ip[0], g_our_ip[1], g_our_ip[2], g_our_ip[3]);
    LOG_I(TAG, "  MAC: %02X:%02X:%02X:%02X:%02X:%02X",
          g_our_mac[0], g_our_mac[1], g_our_mac[2],
          g_our_mac[3], g_our_mac[4], g_our_mac[5]);
    LOG_I(TAG, "============================================");
    LOG_I(TAG, "");

    /* LAN9646 Init */
    if (init_lan9646() != lan9646OK) {
        LOG_E(TAG, "FATAL: LAN9646 init failed!");
        while (1) { delay_ms(1000); }
    }

    /* GMAC Init */
    LOG_I(TAG, "Initializing GMAC...");
    Eth_43_GMAC_Init(&Eth_43_GMAC_xPredefinedConfig);
    configure_gmac_1gbps();
    Eth_43_GMAC_SetControllerMode(ETH_CTRL_IDX, ETH_MODE_ACTIVE);
    configure_s32k388_rgmii();
    LOG_I(TAG, "GMAC OK");

    /* Wait for link */
    delay_ms(100);

    LOG_I(TAG, "");
    LOG_I(TAG, "Ready! Broadcast every 5s, responding to ping...");
    LOG_I(TAG, "");

    /*=======================================================================*/
    /*                          MAIN LOOP                                    */
    /*=======================================================================*/

    uint32_t loop = 0;
    uint32_t last_bcast = 0;

    for (;;) {
        /* Poll for received packets */
        poll_rx();

        /* Broadcast every 5 seconds (5000 iterations * ~1ms delay = 5s) */
        loop++;
        if (loop - last_bcast >= 5000) {
            send_broadcast();
            last_bcast = loop;

            /* Print status */
            LOG_I(TAG, "Status: RX=%lu TX=%lu PING=%lu ARP=%lu",
                  (unsigned long)g_rx_count,
                  (unsigned long)g_tx_count,
                  (unsigned long)g_ping_count,
                  (unsigned long)g_arp_count);
        }

        /* Small delay to prevent tight loop */
        delay_ms(1);
    }

    return 0;
}
