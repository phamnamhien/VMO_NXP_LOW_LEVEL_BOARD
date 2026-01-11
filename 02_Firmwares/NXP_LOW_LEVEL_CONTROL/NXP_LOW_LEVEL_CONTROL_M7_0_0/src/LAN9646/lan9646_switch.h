/**
 * \file            lan9646_switch.h
 * \brief           LAN9646 Ethernet Switch - High Level API
 */

/*
 * Copyright (c) 2026 Pham Nam Hien
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LAN9646 library.
 *
 * Author:          Pham Nam Hien
 * Version:         v1.0.0
 */
#ifndef LAN9646_SWITCH_HDR_H
#define LAN9646_SWITCH_HDR_H

#include "lan9646.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*===========================================================================*/
/*                              PORT DEFINITIONS                              */
/*===========================================================================*/

#define LAN9646_PORT1               1       /*!< Port 1 (PHY) */
#define LAN9646_PORT2               2       /*!< Port 2 (PHY) */
#define LAN9646_PORT3               3       /*!< Port 3 (PHY) */
#define LAN9646_PORT4               4       /*!< Port 4 (PHY) */
#define LAN9646_PORT6               6       /*!< Port 6 (RGMII - CPU) */
#define LAN9646_PORT7               7       /*!< Port 7 (RGMII) */

#define LAN9646_PORT_MASK_ALL       0x7F    /*!< All ports mask */
#define LAN9646_PORT_MASK_PHY       0x0F    /*!< PHY ports (1-4) mask */
#define LAN9646_PORT_MASK_RGMII     0x60    /*!< RGMII ports (6-7) mask */

/*===========================================================================*/
/*                              DATA TYPES                                    */
/*===========================================================================*/

/**
 * \brief           Link speed enumeration
 */
typedef enum {
    LAN9646_SPEED_10M = 0,      /*!< 10 Mbps */
    LAN9646_SPEED_100M,         /*!< 100 Mbps */
    LAN9646_SPEED_1000M,        /*!< 1000 Mbps */
    LAN9646_SPEED_DOWN          /*!< Link down */
} lan9646_speed_t;

/**
 * \brief           Duplex mode enumeration
 */
typedef enum {
    LAN9646_DUPLEX_HALF = 0,    /*!< Half duplex */
    LAN9646_DUPLEX_FULL         /*!< Full duplex */
} lan9646_duplex_t;

/**
 * \brief           Port link status structure
 */
typedef struct {
    uint8_t         port;           /*!< Port number */
    bool            link_up;        /*!< Link status */
    lan9646_speed_t speed;          /*!< Link speed */
    lan9646_duplex_t duplex;        /*!< Duplex mode */
    bool            auto_mdix;      /*!< Auto MDI/MDIX (PHY ports only) */
} lan9646_port_status_t;

/**
 * \brief           MIB counters structure
 */
typedef struct {
    /* RX counters */
    uint32_t rx_unicast;            /*!< RX Unicast packets */
    uint32_t rx_broadcast;          /*!< RX Broadcast packets */
    uint32_t rx_multicast;          /*!< RX Multicast packets */
    uint64_t rx_bytes;              /*!< RX Bytes */
    uint32_t rx_crc_err;            /*!< RX CRC errors */
    uint32_t rx_undersize;          /*!< RX Undersize packets */
    uint32_t rx_oversize;           /*!< RX Oversize packets */
    uint32_t rx_discard;            /*!< RX Discarded packets */

    /* TX counters */
    uint32_t tx_unicast;            /*!< TX Unicast packets */
    uint32_t tx_broadcast;          /*!< TX Broadcast packets */
    uint32_t tx_multicast;          /*!< TX Multicast packets */
    uint64_t tx_bytes;              /*!< TX Bytes */
    uint32_t tx_collisions;         /*!< TX Collisions */
    uint32_t tx_discard;            /*!< TX Discarded packets */
} lan9646_mib_t;

/**
 * \brief           Simple MIB counters (packets only)
 */
typedef struct {
    uint32_t rx_packets;            /*!< Total RX packets */
    uint32_t tx_packets;            /*!< Total TX packets */
    uint32_t rx_bytes;              /*!< RX Bytes (low 32-bit) */
    uint32_t tx_bytes;              /*!< TX Bytes (low 32-bit) */
} lan9646_mib_simple_t;

/**
 * \brief           RGMII delay configuration
 */
typedef struct {
    bool tx_delay;                  /*!< Enable TX internal delay (2ns) */
    bool rx_delay;                  /*!< Enable RX internal delay (2ns) */
} lan9646_rgmii_delay_t;

/**
 * \brief           Link status callback type
 */
typedef void (*lan9646_link_cb_t)(uint8_t port, bool link_up,
                                  lan9646_speed_t speed, lan9646_duplex_t duplex);

/**
 * \brief           SYNCLKO clock source
 */
typedef enum {
    LAN9646_CLK_SRC_XI = 0,         /*!< 25MHz XI crystal (default) */
    LAN9646_CLK_SRC_PORT1_RX,       /*!< Port 1 recovered RX clock */
    LAN9646_CLK_SRC_PORT2_RX,       /*!< Port 2 recovered RX clock */
    LAN9646_CLK_SRC_PORT3_RX,       /*!< Port 3 recovered RX clock */
    LAN9646_CLK_SRC_PORT4_RX,       /*!< Port 4 recovered RX clock */
} lan9646_clk_src_t;

/**
 * \brief           SYNCLKO output frequency
 */
typedef enum {
    LAN9646_CLK_FREQ_25MHZ = 0,     /*!< 25 MHz output */
    LAN9646_CLK_FREQ_125MHZ,        /*!< 125 MHz output */
} lan9646_clk_freq_t;

/**
 * \brief           Clock configuration structure
 */
typedef struct {
    lan9646_clk_src_t   source;         /*!< Clock source */
    lan9646_clk_freq_t  frequency;      /*!< Output frequency */
    bool                synclko_enable; /*!< SYNCLKO pin enable */
    uint8_t             raw_reg;        /*!< Raw register value (0x0024) */
} lan9646_clk_cfg_t;

/*===========================================================================*/
/*                         INITIALIZATION FUNCTIONS                           */
/*===========================================================================*/

/**
 * \brief           Initialize LAN9646 switch (verify chip only)
 * \note            Switch works with DEFAULT settings, no config needed
 * \param[in]       handle: Pointer to device handle
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_init(lan9646_t* handle);

/**
 * \brief           Get chip ID and revision
 * \param[in]       handle: Pointer to device handle
 * \param[out]      chip_id: Chip ID (should be 0x0094)
 * \param[out]      revision: Chip revision (can be NULL)
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_get_chip_id(lan9646_t* handle, uint16_t* chip_id, uint8_t* revision);

/*===========================================================================*/
/*                          PORT STATUS FUNCTIONS                             */
/*===========================================================================*/

/**
 * \brief           Get port link status
 * \param[in]       handle: Pointer to device handle
 * \param[in]       port: Port number (1-4, 6-7)
 * \param[out]      status: Port status structure
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_get_port_status(lan9646_t* handle, uint8_t port,
                                          lan9646_port_status_t* status);

/**
 * \brief           Get Port 6 (GMAC) link status
 * \param[in]       handle: Pointer to device handle
 * \param[out]      speed: Link speed (can be NULL)
 * \param[out]      duplex: Duplex mode (can be NULL)
 * \return          true if link is up
 */
bool lan9646_switch_get_gmac_link(lan9646_t* handle, lan9646_speed_t* speed,
                                  lan9646_duplex_t* duplex);

/**
 * \brief           Get all PHY ports (1-4) link status
 * \param[in]       handle: Pointer to device handle
 * \param[out]      status: Array of 4 port status structures
 * \return          Bitmask of ports with link up (bit 0 = port 1, etc)
 */
uint8_t lan9646_switch_get_all_phy_status(lan9646_t* handle, lan9646_port_status_t* status);

/*===========================================================================*/
/*                          PORT CONTROL FUNCTIONS                            */
/*===========================================================================*/

/**
 * \brief           Enable/Disable port TX and RX
 * \param[in]       handle: Pointer to device handle
 * \param[in]       port: Port number
 * \param[in]       tx_en: Enable TX
 * \param[in]       rx_en: Enable RX
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_set_port_enable(lan9646_t* handle, uint8_t port,
                                          bool tx_en, bool rx_en);

/**
 * \brief           Set Port 6 RGMII delay
 * \note            Only needed if default (TX=on, RX=off) doesn't work
 * \param[in]       handle: Pointer to device handle
 * \param[in]       delay: Delay configuration
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_set_rgmii_delay(lan9646_t* handle, const lan9646_rgmii_delay_t* delay);

/**
 * \brief           Get Port 6 RGMII delay configuration
 * \param[in]       handle: Pointer to device handle
 * \param[out]      delay: Current delay configuration
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_get_rgmii_delay(lan9646_t* handle, lan9646_rgmii_delay_t* delay);

/*===========================================================================*/
/*                          MIB COUNTER FUNCTIONS                             */
/*===========================================================================*/

/**
 * \brief           Read MIB counters for a port
 * \note            MIB counters are READ-CLEAR!
 * \param[in]       handle: Pointer to device handle
 * \param[in]       port: Port number
 * \param[out]      mib: MIB counters structure
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_read_mib(lan9646_t* handle, uint8_t port, lan9646_mib_t* mib);

/**
 * \brief           Read simple MIB counters (packets only)
 * \note            MIB counters are READ-CLEAR!
 * \param[in]       handle: Pointer to device handle
 * \param[in]       port: Port number
 * \param[out]      mib: Simple MIB counters
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_read_mib_simple(lan9646_t* handle, uint8_t port,
                                          lan9646_mib_simple_t* mib);

/**
 * \brief           Flush (clear) MIB counters for a port
 * \param[in]       handle: Pointer to device handle
 * \param[in]       port: Port number (0 = all ports)
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_flush_mib(lan9646_t* handle, uint8_t port);

/*===========================================================================*/
/*                         PORT MIRRORING FUNCTIONS                           */
/*===========================================================================*/

/**
 * \brief           Enable port mirroring (for debug)
 * \note            Mirrors RX traffic from source ports to sniffer port
 * \param[in]       handle: Pointer to device handle
 * \param[in]       sniffer_port: Port to receive mirrored traffic (usually 6)
 * \param[in]       source_mask: Bitmask of ports to mirror (bit 0 = port 1)
 * \param[in]       mirror_rx: Mirror received packets
 * \param[in]       mirror_tx: Mirror transmitted packets
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_set_port_mirror(lan9646_t* handle, uint8_t sniffer_port,
                                          uint8_t source_mask, bool mirror_rx, bool mirror_tx);

/**
 * \brief           Disable all port mirroring
 * \param[in]       handle: Pointer to device handle
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_disable_port_mirror(lan9646_t* handle);

/*===========================================================================*/
/*                            VLAN FUNCTIONS                                  */
/*===========================================================================*/

/**
 * \brief           Set port VLAN membership
 * \param[in]       handle: Pointer to device handle
 * \param[in]       port: Port number
 * \param[in]       membership: Bitmask of ports this port can forward to
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_set_port_membership(lan9646_t* handle, uint8_t port,
                                              uint8_t membership);

/**
 * \brief           Get port VLAN membership
 * \param[in]       handle: Pointer to device handle
 * \param[in]       port: Port number
 * \param[out]      membership: Current membership mask
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_get_port_membership(lan9646_t* handle, uint8_t port,
                                              uint8_t* membership);

/*===========================================================================*/
/*                         LINK CALLBACK FUNCTIONS                            */
/*===========================================================================*/

/**
 * \brief           Set link status change callback
 * \param[in]       handle: Pointer to device handle
 * \param[in]       callback: Callback function (NULL to disable)
 */
void lan9646_switch_set_link_callback(lan9646_t* handle, lan9646_link_cb_t callback);

/**
 * \brief           Poll link status (call periodically)
 * \note            Will trigger callback if link status changed
 * \param[in]       handle: Pointer to device handle
 */
void lan9646_switch_poll_link(lan9646_t* handle);

/*===========================================================================*/
/*                           DEBUG FUNCTIONS                                  */
/*===========================================================================*/

/**
 * \brief           Dump switch registers (debug)
 * \param[in]       handle: Pointer to device handle
 */
void lan9646_switch_dump_regs(lan9646_t* handle);

/**
 * \brief           Print port status (debug)
 * \param[in]       handle: Pointer to device handle
 * \param[in]       port: Port number (0 = all ports)
 */
void lan9646_switch_print_status(lan9646_t* handle, uint8_t port);

/*===========================================================================*/
/*                          CLOCK FUNCTIONS                                   */
/*===========================================================================*/

/**
 * \brief           Get clock configuration
 * \param[in]       handle: Pointer to device handle
 * \param[out]      cfg: Clock configuration structure
 * \return          \ref lan9646OK on success
 */
lan9646r_t lan9646_switch_get_clock_config(lan9646_t* handle, lan9646_clk_cfg_t* cfg);

/**
 * \brief           Print clock configuration (debug)
 * \param[in]       handle: Pointer to device handle
 */
void lan9646_switch_print_clock_config(lan9646_t* handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LAN9646_SWITCH_HDR_H */
