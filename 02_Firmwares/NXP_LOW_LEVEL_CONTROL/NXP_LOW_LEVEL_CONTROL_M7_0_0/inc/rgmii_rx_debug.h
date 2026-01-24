/**
 * @file    rgmii_rx_debug.h
 * @brief   RGMII RX Path Debug Module for S32K388 + LAN9646
 * @note    Specialized debugging for RX path issues
 *
 * This module focuses on debugging the RX path:
 * - LAN9646 Port 6 TX -> RGMII -> S32K388 GMAC RX
 *
 * Key areas analyzed:
 * - RX_CLK signal (from LAN9646 to S32K388)
 * - RXD0-3 data lines
 * - RX_CTL control signal
 * - SIUL2 IMCR input mux configuration
 * - GMAC RX DMA and FIFO status
 * - RGMII timing/delay on RX path
 */

#ifndef RGMII_RX_DEBUG_H
#define RGMII_RX_DEBUG_H

#include <stdint.h>
#include <stdbool.h>
#include "lan9646.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/*                          RX PATH STATUS STRUCTURES                         */
/*===========================================================================*/

/**
 * @brief   S32K388 GMAC RX path status
 */
typedef struct {
    /* DCM_GPR RX Clock Configuration */
    uint32_t dcmrwf3;
    bool rx_clk_bypass_enabled;     /* DCMRWF3[13] = 1 means bypass */

    /* MC_CGM MUX_7 (GMAC0_RX_CLK - should be bypassed) */
    uint32_t mux7_csc;
    uint32_t mux7_css;
    uint32_t mux7_dc0;

    /* SIUL2 IMCR for RX pins */
    uint8_t imcr_rx_clk;            /* IMCR[300] */
    uint8_t imcr_rx_ctl;            /* IMCR[292] */
    uint8_t imcr_rxd0;              /* IMCR[294] */
    uint8_t imcr_rxd1;              /* IMCR[295] */
    uint8_t imcr_rxd2;              /* IMCR[301] */
    uint8_t imcr_rxd3;              /* IMCR[302] */

    /* GMAC MAC RX Status */
    uint32_t mac_configuration;
    uint32_t mac_debug;
    uint32_t mac_phyif_status;
    bool rx_enabled;
    bool rgmii_rx_active;           /* MAC_DEBUG RPESTS */
    bool link_status;               /* MAC_PHYIF_STATUS LNKSTS */

    /* GMAC DMA RX Status */
    uint32_t dma_ch0_status;
    uint32_t dma_ch0_rx_control;
    uint32_t dma_debug_status0;
    uint8_t rx_dma_state;           /* RPS from DMA_DEBUG_STATUS0 */

    /* GMAC MTL RX Status */
    uint32_t mtl_rxq0_debug;
    uint8_t rx_queue_state;         /* RXQSTS */
    uint8_t packets_in_queue;       /* PRXQ */

    /* GMAC RX Counters */
    uint32_t rx_packets_good_bad;
    uint32_t rx_octets_good;
    uint32_t rx_broadcast;
    uint32_t rx_multicast;
    uint32_t rx_unicast;
    uint32_t rx_crc_errors;
    uint32_t rx_align_errors;
    uint32_t rx_runt_errors;
    uint32_t rx_jabber_errors;
    uint32_t rx_fifo_overflow;
} s32k388_rx_status_t;

/**
 * @brief   LAN9646 Port 6 TX status (TX from LAN9646 = RX for GMAC)
 */
typedef struct {
    /* XMII Control */
    uint8_t xmii_ctrl0;
    uint8_t xmii_ctrl1;
    bool tx_delay_enabled;          /* XMII_CTRL1[3] */

    /* Port Status */
    uint8_t port_status;
    uint8_t speed_status;           /* 0=10M, 1=100M, 2=1000M */
    bool duplex_full;

    /* MSTP State */
    uint8_t mstp_state;
    bool tx_enabled;                /* Port 6 TX to GMAC RX */

    /* TX Counters (LAN9646 TX = GMAC RX) */
    uint32_t tx_broadcast;
    uint32_t tx_multicast;
    uint32_t tx_unicast;
    uint32_t tx_total_bytes;
    uint32_t tx_late_collision;
    uint32_t tx_excess_collision;
    uint32_t tx_dropped;
} lan9646_tx_status_t;

/**
 * @brief   Combined RX path analysis
 */
typedef struct {
    s32k388_rx_status_t gmac;
    lan9646_tx_status_t lan9646;

    /* Analysis results */
    bool rx_clk_config_ok;
    bool rx_pin_config_ok;
    bool rx_dma_ready;
    bool rx_path_active;
    bool lan9646_tx_ok;

    /* Diagnosis */
    char diagnosis[512];
} rx_path_analysis_t;

/*===========================================================================*/
/*                          INITIALIZATION                                    */
/*===========================================================================*/

/**
 * @brief   Initialize the RX debug module
 * @param   lan         Pointer to LAN9646 handle
 * @param   delay_ms    Delay function (milliseconds)
 */
void rx_debug_init(lan9646_t* lan, void (*delay_ms)(uint32_t));

/*===========================================================================*/
/*                          RX PATH ANALYSIS                                  */
/*===========================================================================*/

/**
 * @brief   Run complete RX path analysis
 */
void rx_debug_full_analysis(void);

/**
 * @brief   Analyze and dump S32K388 GMAC RX configuration
 */
void rx_debug_dump_gmac_rx_config(void);

/**
 * @brief   Analyze and dump LAN9646 Port 6 TX configuration
 */
void rx_debug_dump_lan9646_tx_config(void);

/**
 * @brief   Read complete RX path status
 * @param   analysis    Pointer to analysis structure
 */
void rx_debug_read_status(rx_path_analysis_t* analysis);

/*===========================================================================*/
/*                          RX_CLK SPECIFIC DEBUGGING                         */
/*===========================================================================*/

/**
 * @brief   Deep analysis of RX_CLK signal path
 * @note    This is the most critical signal for RX path
 */
void rx_debug_analyze_rx_clk(void);

/**
 * @brief   Verify RX_CLK is coming from LAN9646
 * @return  true if RX_CLK appears to be working
 */
bool rx_debug_verify_rx_clk(void);

/**
 * @brief   Try different RX_CLK configurations
 */
void rx_debug_try_rx_clk_options(void);

/*===========================================================================*/
/*                          IMCR/PIN CONFIGURATION                            */
/*===========================================================================*/

/**
 * @brief   Dump all SIUL2 IMCR registers for GMAC0 RX
 */
void rx_debug_dump_imcr(void);

/**
 * @brief   Dump MSCR (output) configuration for RX pins
 */
void rx_debug_dump_mscr(void);

/**
 * @brief   Verify all RX pin configurations
 * @return  true if all pins appear correctly configured
 */
bool rx_debug_verify_rx_pins(void);

/*===========================================================================*/
/*                          GMAC RX DMA/FIFO STATUS                           */
/*===========================================================================*/

/**
 * @brief   Dump GMAC DMA RX status
 */
void rx_debug_dump_dma_status(void);

/**
 * @brief   Dump GMAC MTL RX queue status
 */
void rx_debug_dump_mtl_status(void);

/**
 * @brief   Check if GMAC RX DMA is ready to receive
 * @return  true if DMA appears ready
 */
bool rx_debug_check_dma_ready(void);

/**
 * @brief   Check RX descriptor ring status
 */
void rx_debug_dump_rx_descriptors(void);

/*===========================================================================*/
/*                          RX COUNTER MONITORING                             */
/*===========================================================================*/

/**
 * @brief   Dump current GMAC RX counters
 */
void rx_debug_dump_gmac_counters(void);

/**
 * @brief   Dump current LAN9646 Port 6 TX counters
 */
void rx_debug_dump_lan9646_tx_counters(void);

/**
 * @brief   Clear all counters and monitor for changes
 * @param   duration_ms     How long to monitor
 */
void rx_debug_monitor_counters(uint32_t duration_ms);

/**
 * @brief   Continuous counter display (call in loop)
 */
void rx_debug_live_counters(void);

/*===========================================================================*/
/*                          RX PATH TESTS                                     */
/*===========================================================================*/

/**
 * @brief   Test RX by sending from LAN9646 Port 6 loopback
 * @param   count       Number of packets
 * @return  Number received by GMAC
 */
uint32_t rx_debug_test_loopback(uint32_t count);

/**
 * @brief   Test RX by injecting traffic from LAN9646 Port 1-4
 * @param   src_port    Source port (1-4)
 * @param   count       Number of packets
 * @return  Number received by GMAC
 */
uint32_t rx_debug_test_from_port(uint8_t src_port, uint32_t count);

/**
 * @brief   Sweep all delay combinations and test RX
 */
void rx_debug_delay_sweep(void);

/**
 * @brief   Try specific LAN9646 TX delay setting
 * @param   tx_delay    Enable TX delay (affects S32K388 RX timing)
 */
void rx_debug_set_lan9646_tx_delay(bool tx_delay);

/*===========================================================================*/
/*                          ADVANCED DEBUGGING                                */
/*===========================================================================*/

/**
 * @brief   Probe RX_CLK pin directly using GPIO
 * @note    Temporarily reconfigures pin as GPIO input
 */
void rx_debug_probe_rx_clk_pin(void);

/**
 * @brief   Check GMAC internal state machines
 */
void rx_debug_dump_state_machines(void);

/**
 * @brief   Reset GMAC RX path and reinitialize
 */
void rx_debug_reset_rx_path(void);

/**
 * @brief   Force enable GMAC RX with specific settings
 */
void rx_debug_force_enable_rx(void);

/*===========================================================================*/
/*                          TROUBLESHOOTING                                   */
/*===========================================================================*/

/**
 * @brief   Print RX-specific troubleshooting guide
 */
void rx_debug_print_troubleshooting(void);

/**
 * @brief   Suggest fixes based on current RX path state
 */
void rx_debug_suggest_fixes(void);

/**
 * @brief   Run automated RX debug sequence
 * @note    Tries various configurations to find working setup
 */
void rx_debug_auto_diagnose(void);

#ifdef __cplusplus
}
#endif

#endif /* RGMII_RX_DEBUG_H */
