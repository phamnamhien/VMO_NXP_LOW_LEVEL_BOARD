/**
 * @file    rgmii_config_debug.h
 * @brief   RGMII Configuration Debug Module for S32K388 + LAN9646
 * @note    Comprehensive debug output for hardware/software validation
 *
 * This module provides detailed configuration dump and validation for:
 * - S32K388 GMAC0 registers (MAC, DMA, MMC counters)
 * - S32K388 DCM_GPR registers (RGMII mode, clock bypass)
 * - S32K388 MC_CGM clock configuration
 * - LAN9646 Port 6 XMII/RGMII configuration
 * - Speed/Duplex synchronization between both chips
 *
 * @author  Pham Nam Hien
 * @version 1.0.0
 */

#ifndef RGMII_CONFIG_DEBUG_H
#define RGMII_CONFIG_DEBUG_H

#include <stdint.h>
#include <stdbool.h>
#include "lan9646.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/*                          SPEED/DUPLEX DEFINITIONS                          */
/*===========================================================================*/

typedef enum {
    RGMII_SPEED_10M   = 0,
    RGMII_SPEED_100M  = 1,
    RGMII_SPEED_1000M = 2,
} rgmii_speed_t;

typedef enum {
    RGMII_DUPLEX_HALF = 0,
    RGMII_DUPLEX_FULL = 1,
} rgmii_duplex_t;

typedef enum {
    RGMII_DELAY_NONE    = 0,    /* No internal delay */
    RGMII_DELAY_TX_ONLY = 1,    /* TX delay only (~1.5-2ns) */
    RGMII_DELAY_RX_ONLY = 2,    /* RX delay only (~1.5-2ns) */
    RGMII_DELAY_BOTH    = 3,    /* Both TX and RX delay */
} rgmii_delay_mode_t;

/*===========================================================================*/
/*                          CONFIGURATION STRUCTURES                          */
/*===========================================================================*/

/**
 * @brief   S32K388 GMAC configuration snapshot
 */
typedef struct {
    /* DCM_GPR Registers */
    uint32_t dcmrwf1;           /* RGMII mode selection */
    uint32_t dcmrwf3;           /* TX_CLK_OUT_EN, RX_CLK_BYPASS */

    /* MC_CGM Clock Mux 8 (GMAC TX Clock) */
    uint32_t mux8_csc;          /* Clock Source Control */
    uint32_t mux8_css;          /* Clock Source Status */
    uint32_t mux8_dc0;          /* Divider Configuration */

    /* GMAC MAC Configuration */
    uint32_t mac_configuration; /* Main MAC config register */
    uint32_t mac_version;       /* MAC IP version */
    uint32_t mac_hw_feature0;   /* Hardware features */
    uint32_t mac_hw_feature1;
    uint32_t mac_hw_feature2;
    uint32_t mac_hw_feature3;

    /* MAC Address */
    uint32_t mac_addr_high;
    uint32_t mac_addr_low;

    /* MAC Status */
    uint32_t mac_debug;

    /* DMA Configuration */
    uint32_t dma_mode;
    uint32_t dma_sysbus_mode;
    uint32_t dma_ch0_control;
    uint32_t dma_ch0_tx_control;
    uint32_t dma_ch0_rx_control;

    /* MTL Configuration */
    uint32_t mtl_operation_mode;
    uint32_t mtl_txq0_operation_mode;
    uint32_t mtl_rxq0_operation_mode;

    /* Parsed values */
    rgmii_speed_t speed;
    rgmii_duplex_t duplex;
    bool tx_enable;
    bool rx_enable;
    bool loopback;
    uint8_t interface_mode;     /* 0=MII, 1=RMII, 2=RGMII */
    bool tx_clk_out_enable;
    bool rx_clk_bypass;
} s32k388_gmac_config_t;

/**
 * @brief   LAN9646 Port 6 configuration snapshot
 */
typedef struct {
    /* Chip ID */
    uint16_t chip_id;
    uint8_t revision;

    /* Port 6 XMII Control */
    uint8_t xmii_ctrl0;
    uint8_t xmii_ctrl1;

    /* Port 6 Status */
    uint8_t port_status;

    /* Port 6 Operation Control */
    uint8_t op_ctrl0;
    uint8_t op_ctrl1;

    /* Port 6 MSTP State */
    uint8_t mstp_state;

    /* Port 6 Membership */
    uint32_t membership;

    /* Port 6 MAC Control */
    uint8_t mac_ctrl0;
    uint8_t mac_ctrl1;

    /* Switch Operation */
    uint8_t switch_op;

    /* Parsed values */
    rgmii_speed_t speed;
    rgmii_duplex_t duplex;
    bool tx_delay;
    bool rx_delay;
    bool tx_flow_ctrl;
    bool rx_flow_ctrl;
    bool tx_enable;
    bool rx_enable;
    bool learning_enable;
} lan9646_port6_config_t;

/**
 * @brief   Combined configuration for validation
 */
typedef struct {
    s32k388_gmac_config_t s32k388;
    lan9646_port6_config_t lan9646;

    /* Validation results */
    bool speed_match;
    bool duplex_match;
    bool delay_valid;           /* Delay configuration is complementary */
    bool interface_valid;       /* S32K388 in RGMII mode */
    bool clocks_valid;          /* TX/RX clocks configured correctly */
    bool overall_valid;         /* All checks passed */

    /* Recommended fixes */
    char recommendations[512];
} rgmii_config_snapshot_t;

/*===========================================================================*/
/*                          MIB COUNTER STRUCTURES                            */
/*===========================================================================*/

/**
 * @brief   S32K388 GMAC MMC counters
 */
typedef struct {
    /* TX Counters */
    uint32_t tx_octet_count_good_bad;
    uint32_t tx_packet_count_good_bad;
    uint32_t tx_broadcast_packets_good;
    uint32_t tx_multicast_packets_good;
    uint32_t tx_unicast_packets_good_bad;
    uint32_t tx_underflow_error_packets;
    uint32_t tx_single_collision_good_packets;
    uint32_t tx_multiple_collision_good_packets;
    uint32_t tx_deferred_packets;
    uint32_t tx_late_collision_packets;
    uint32_t tx_excessive_collision_packets;
    uint32_t tx_carrier_error_packets;
    uint32_t tx_packet_count_good;
    uint32_t tx_pause_packets;

    /* RX Counters */
    uint32_t rx_packets_count_good_bad;
    uint32_t rx_octet_count_good_bad;
    uint32_t rx_octet_count_good;
    uint32_t rx_broadcast_packets_good;
    uint32_t rx_multicast_packets_good;
    uint32_t rx_crc_error_packets;
    uint32_t rx_alignment_error_packets;
    uint32_t rx_runt_error_packets;
    uint32_t rx_jabber_error_packets;
    uint32_t rx_undersize_packets_good;
    uint32_t rx_oversize_packets_good;
    uint32_t rx_unicast_packets_good;
    uint32_t rx_length_error_packets;
    uint32_t rx_out_of_range_type_packets;
    uint32_t rx_pause_packets;
    uint32_t rx_fifo_overflow_packets;
    uint32_t rx_watchdog_error_packets;
} s32k388_mmc_counters_t;

/**
 * @brief   LAN9646 Port 6 MIB counters
 */
typedef struct {
    /* RX Counters */
    uint32_t rx_hi_priority_bytes;
    uint32_t rx_undersize;
    uint32_t rx_fragments;
    uint32_t rx_oversize;
    uint32_t rx_jabbers;
    uint32_t rx_symbol_err;
    uint32_t rx_crc_err;
    uint32_t rx_align_err;
    uint32_t rx_control;
    uint32_t rx_pause;
    uint32_t rx_broadcast;
    uint32_t rx_multicast;
    uint32_t rx_unicast;
    uint32_t rx_64;
    uint32_t rx_65_127;
    uint32_t rx_128_255;
    uint32_t rx_256_511;
    uint32_t rx_512_1023;
    uint32_t rx_1024_1522;
    uint32_t rx_total;
    uint32_t rx_dropped;

    /* TX Counters */
    uint32_t tx_hi_priority_bytes;
    uint32_t tx_late_collision;
    uint32_t tx_pause;
    uint32_t tx_broadcast;
    uint32_t tx_multicast;
    uint32_t tx_unicast;
    uint32_t tx_deferred;
    uint32_t tx_total_collision;
    uint32_t tx_excess_collision;
    uint32_t tx_single_collision;
    uint32_t tx_multi_collision;
    uint32_t tx_total;
    uint32_t tx_dropped;
} lan9646_mib_counters_t;

/*===========================================================================*/
/*                          INITIALIZATION                                    */
/*===========================================================================*/

/**
 * @brief   Initialize the RGMII debug module
 * @param   lan         Pointer to LAN9646 handle
 * @param   delay_ms    Delay function (milliseconds)
 */
void rgmii_debug_init(lan9646_t* lan, void (*delay_ms)(uint32_t));

/*===========================================================================*/
/*                          CONFIGURATION DUMP                                */
/*===========================================================================*/

/**
 * @brief   Dump S32K388 GMAC0 complete configuration
 * @note    Prints all relevant registers with decoded values
 */
void rgmii_debug_dump_s32k388(void);

/**
 * @brief   Dump LAN9646 Port 6 complete configuration
 * @note    Prints all relevant registers with decoded values
 */
void rgmii_debug_dump_lan9646(void);

/**
 * @brief   Dump both S32K388 and LAN9646 configurations
 * @note    Side-by-side comparison format
 */
void rgmii_debug_dump_all(void);

/**
 * @brief   Quick summary dump (most important info only)
 */
void rgmii_debug_quick_summary(void);

/*===========================================================================*/
/*                          CONFIGURATION SNAPSHOT                            */
/*===========================================================================*/

/**
 * @brief   Read S32K388 GMAC configuration
 * @param   config      Pointer to configuration structure
 */
void rgmii_debug_read_s32k388_config(s32k388_gmac_config_t* config);

/**
 * @brief   Read LAN9646 Port 6 configuration
 * @param   config      Pointer to configuration structure
 */
void rgmii_debug_read_lan9646_config(lan9646_port6_config_t* config);

/**
 * @brief   Read complete configuration snapshot
 * @param   snapshot    Pointer to snapshot structure
 */
void rgmii_debug_read_snapshot(rgmii_config_snapshot_t* snapshot);

/*===========================================================================*/
/*                          VALIDATION                                        */
/*===========================================================================*/

/**
 * @brief   Validate configuration between S32K388 and LAN9646
 * @param   snapshot    Pointer to configuration snapshot
 * @return  true if all checks pass, false otherwise
 */
bool rgmii_debug_validate(rgmii_config_snapshot_t* snapshot);

/**
 * @brief   Print validation report with recommendations
 * @param   snapshot    Pointer to configuration snapshot
 */
void rgmii_debug_print_validation(const rgmii_config_snapshot_t* snapshot);

/*===========================================================================*/
/*                          SPEED/DUPLEX CONFIGURATION                        */
/*===========================================================================*/

/**
 * @brief   Configure RGMII speed on both S32K388 and LAN9646
 * @param   speed       Target speed (10/100/1000 Mbps)
 * @param   duplex      Target duplex mode
 * @return  true on success
 */
bool rgmii_debug_set_speed(rgmii_speed_t speed, rgmii_duplex_t duplex);

/**
 * @brief   Configure S32K388 GMAC speed/duplex only
 * @param   speed       Target speed
 * @param   duplex      Target duplex
 */
void rgmii_debug_set_s32k388_speed(rgmii_speed_t speed, rgmii_duplex_t duplex);

/**
 * @brief   Configure LAN9646 Port 6 speed/duplex only
 * @param   speed       Target speed
 * @param   duplex      Target duplex
 */
void rgmii_debug_set_lan9646_speed(rgmii_speed_t speed, rgmii_duplex_t duplex);

/*===========================================================================*/
/*                          RGMII DELAY CONFIGURATION                         */
/*===========================================================================*/

/**
 * @brief   Configure RGMII internal delays on LAN9646
 * @param   tx_delay    Enable TX internal delay
 * @param   rx_delay    Enable RX internal delay
 */
void rgmii_debug_set_lan9646_delay(bool tx_delay, bool rx_delay);

/**
 * @brief   Try all delay combinations and report results
 * @note    Uses loopback test to find working configuration
 */
void rgmii_debug_delay_sweep(void);

/*===========================================================================*/
/*                          MIB COUNTERS                                      */
/*===========================================================================*/

/**
 * @brief   Dump S32K388 GMAC MMC counters
 */
void rgmii_debug_dump_s32k388_mmc(void);

/**
 * @brief   Dump LAN9646 Port 6 MIB counters
 */
void rgmii_debug_dump_lan9646_mib(void);

/**
 * @brief   Dump both MMC/MIB counters side-by-side
 */
void rgmii_debug_dump_all_counters(void);

/**
 * @brief   Read S32K388 MMC counters
 * @param   counters    Pointer to counter structure
 */
void rgmii_debug_read_s32k388_mmc(s32k388_mmc_counters_t* counters);

/**
 * @brief   Read LAN9646 Port 6 MIB counters
 * @param   counters    Pointer to counter structure
 */
void rgmii_debug_read_lan9646_mib(lan9646_mib_counters_t* counters);

/**
 * @brief   Clear/reset S32K388 MMC counters
 */
void rgmii_debug_clear_s32k388_mmc(void);

/**
 * @brief   Clear/reset LAN9646 Port 6 MIB counters
 */
void rgmii_debug_clear_lan9646_mib(void);

/*===========================================================================*/
/*                          CLOCK DIAGNOSTICS                                 */
/*===========================================================================*/

/**
 * @brief   Dump complete clock configuration for GMAC
 */
void rgmii_debug_dump_clocks(void);

/**
 * @brief   Verify TX clock is properly configured and output
 * @return  true if TX clock appears working
 */
bool rgmii_debug_verify_tx_clock(void);

/**
 * @brief   Verify RX clock bypass is configured correctly
 * @return  true if RX clock bypass is correct
 */
bool rgmii_debug_verify_rx_clock_bypass(void);

/*===========================================================================*/
/*                          SIGNAL INTEGRITY TEST                             */
/*===========================================================================*/

/**
 * @brief   Test TX path by sending packets
 * @param   count       Number of packets to send
 * @return  Number of packets successfully sent
 */
uint32_t rgmii_debug_test_tx_path(uint32_t count);

/**
 * @brief   Test RX path using loopback
 * @param   count       Number of packets to send/expect
 * @return  Number of packets received
 */
uint32_t rgmii_debug_test_loopback(uint32_t count);

/*===========================================================================*/
/*                          TROUBLESHOOTING GUIDE                             */
/*===========================================================================*/

/**
 * @brief   Print troubleshooting guide based on current state
 */
void rgmii_debug_print_troubleshooting(void);

/**
 * @brief   Run full diagnostic and print recommendations
 */
void rgmii_debug_full_diagnostic(void);

/*===========================================================================*/
/*                          UTILITY FUNCTIONS                                 */
/*===========================================================================*/

/**
 * @brief   Get speed string from enum
 */
const char* rgmii_debug_speed_str(rgmii_speed_t speed);

/**
 * @brief   Get duplex string from enum
 */
const char* rgmii_debug_duplex_str(rgmii_duplex_t duplex);

/**
 * @brief   Get interface mode string (MII/RMII/RGMII)
 */
const char* rgmii_debug_interface_str(uint8_t mode);

/**
 * @brief   Get delay mode string
 */
const char* rgmii_debug_delay_str(rgmii_delay_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* RGMII_CONFIG_DEBUG_H */
