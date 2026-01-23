/**
 * @file    rgmii_diag.h
 * @brief   RGMII Hardware Diagnostic for S32K388 + LAN9646
 * @note    Test RGMII signal integrity between S32K388 GMAC and LAN9646 Port 6
 */

#ifndef RGMII_DIAG_H
#define RGMII_DIAG_H

#include <stdint.h>
#include <stdbool.h>
#include "lan9646.h"

/*===========================================================================*/
/*                           TEST RESULT CODES                                */
/*===========================================================================*/

typedef enum {
    RGMII_TEST_PASS = 0,
    RGMII_TEST_FAIL_TX_CLK,      /* S32K388 TX_CLK not working */
    RGMII_TEST_FAIL_TX_DATA,     /* S32K388 TXD[0:3] problem */
    RGMII_TEST_FAIL_TX_CTL,      /* S32K388 TX_CTL problem */
    RGMII_TEST_FAIL_RX_CLK,      /* LAN9646 RX_CLK not working */
    RGMII_TEST_FAIL_RX_DATA,     /* LAN9646 RXD[0:3] problem */
    RGMII_TEST_FAIL_RX_CTL,      /* LAN9646 RX_CTL problem */
    RGMII_TEST_FAIL_TIMING,      /* Timing/delay mismatch */
    RGMII_TEST_FAIL_UNKNOWN,     /* Unknown failure */
} rgmii_test_result_t;

/*===========================================================================*/
/*                           TEST STATISTICS                                  */
/*===========================================================================*/

typedef struct {
    /* GMAC counters */
    uint32_t gmac_tx_good;
    uint32_t gmac_tx_underflow;
    uint32_t gmac_rx_good;
    uint32_t gmac_rx_crc_err;
    uint32_t gmac_rx_align_err;
    uint32_t gmac_rx_runt;
    uint32_t gmac_rx_oversize;

    /* LAN9646 Port 6 counters */
    uint32_t lan_rx_good;
    uint32_t lan_rx_crc_err;
    uint32_t lan_rx_symbol_err;
    uint32_t lan_rx_undersize;
    uint32_t lan_rx_oversize;
    uint32_t lan_tx_good;
    uint32_t lan_tx_late_col;
    uint32_t lan_tx_excess_col;
} rgmii_stats_t;

/*===========================================================================*/
/*                              API FUNCTIONS                                 */
/*===========================================================================*/

/**
 * @brief   Initialize RGMII diagnostic module
 * @param   lan     Pointer to LAN9646 handle
 * @param   delay   Delay function (ms)
 */
void rgmii_diag_init(lan9646_t* lan, void (*delay_ms)(uint32_t));

/**
 * @brief   Run full RGMII diagnostic suite
 * @return  Overall test result
 */
rgmii_test_result_t rgmii_diag_run_all(void);

/**
 * @brief   Test 1: Check clocks are running
 */
rgmii_test_result_t rgmii_diag_test_clocks(void);

/**
 * @brief   Test 2: MAC Loopback (test switch internal path)
 *          Data: GMAC TX -> LAN9646 Port6 -> Loopback -> GMAC RX
 */
rgmii_test_result_t rgmii_diag_test_mac_loopback(void);

/**
 * @brief   Test 3: TX Path only (GMAC -> LAN9646)
 *          Send packets and check LAN9646 MIB counters
 */
rgmii_test_result_t rgmii_diag_test_tx_path(void);

/**
 * @brief   Test 4: RX Path test (LAN9646 -> GMAC)
 *          Requires external traffic or loopback
 */
rgmii_test_result_t rgmii_diag_test_rx_path(void);

/**
 * @brief   Test 5: Timing sweep - try all delay combinations
 */
void rgmii_diag_timing_sweep(void);

/**
 * @brief   Print detailed diagnostic report
 */
void rgmii_diag_print_report(void);

/**
 * @brief   Get human-readable result string
 */
const char* rgmii_diag_result_str(rgmii_test_result_t result);

#endif /* RGMII_DIAG_H */

