/**
 * @file    rgmii_diag.c
 * @brief   RGMII Hardware Diagnostic for S32K388 + LAN9646
 */

#include "rgmii_diag.h"
#include "log.h"
#include "Gmac_Ip.h"

#define TAG "RGMII_DIAG"

/*===========================================================================*/
/*                              PRIVATE DATA                                  */
/*===========================================================================*/

static lan9646_t* g_lan = NULL;
static void (*g_delay)(uint32_t) = NULL;
static rgmii_stats_t g_stats_before;
static rgmii_stats_t g_stats_after;

/* Test packet - 64 bytes minimum */
static uint8_t g_test_packet[64] = {
    /* Dest MAC: Broadcast */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* Src MAC */
    0x10, 0x11, 0x22, 0x77, 0x77, 0x77,
    /* EtherType: Custom test */
    0x88, 0xB5,
    /* Payload: Pattern for bit detection */
    0x00, 0x00, 0x00, 0x00,  /* All zeros */
    0xFF, 0xFF, 0xFF, 0xFF,  /* All ones */
    0xAA, 0xAA, 0xAA, 0xAA,  /* 10101010 */
    0x55, 0x55, 0x55, 0x55,  /* 01010101 */
    0x0F, 0x0F, 0x0F, 0x0F,  /* 00001111 */
    0xF0, 0xF0, 0xF0, 0xF0,  /* 11110000 */
    0x33, 0x33, 0x33, 0x33,  /* 00110011 */
    0xCC, 0xCC, 0xCC, 0xCC,  /* 11001100 */
    /* Sequence number (will be modified) */
    0x00, 0x00, 0x00, 0x00,
    /* Padding */
    0xDE, 0xAD, 0xBE, 0xEF,
    0xCA, 0xFE, 0xBA, 0xBE,
    0x12, 0x34, 0x56, 0x78,
};

/*===========================================================================*/
/*                          HELPER FUNCTIONS                                  */
/*===========================================================================*/

static void read_gmac_stats(rgmii_stats_t* s) {
    s->gmac_tx_good     = IP_GMAC_0->TX_PACKET_COUNT_GOOD;
    s->gmac_tx_underflow = IP_GMAC_0->TX_UNDERFLOW_ERROR_PACKETS;
    s->gmac_rx_good     = IP_GMAC_0->RX_PACKETS_COUNT_GOOD_BAD;
    s->gmac_rx_crc_err  = IP_GMAC_0->RX_CRC_ERROR_PACKETS;
    s->gmac_rx_align_err = IP_GMAC_0->RX_ALIGNMENT_ERROR_PACKETS;
    s->gmac_rx_runt     = IP_GMAC_0->RX_RUNT_ERROR_PACKETS;
    s->gmac_rx_oversize = IP_GMAC_0->RX_OVERSIZE_PACKETS_GOOD;
}

/* LAN9646 MIB counter offsets for Port 6 */
#define MIB_BASE(port)      (0x0500 + ((port) * 0x80))
#define MIB_RX_TOTAL        0x08
#define MIB_RX_CRC          0x34
#define MIB_RX_SYMBOL       0x3C
#define MIB_RX_UNDERSIZE    0x24
#define MIB_RX_OVERSIZE     0x30
#define MIB_TX_TOTAL        0x50
#define MIB_TX_LATE_COL     0x64
#define MIB_TX_EXCESS_COL   0x68

static uint32_t read_mib(uint8_t port, uint8_t offset) {
    uint32_t val;
    lan9646_read_reg32(g_lan, MIB_BASE(port) | offset, &val);
    return val;
}

static void read_lan_stats(rgmii_stats_t* s, uint8_t port) {
    s->lan_rx_good      = read_mib(port, MIB_RX_TOTAL);
    s->lan_rx_crc_err   = read_mib(port, MIB_RX_CRC);
    s->lan_rx_symbol_err = read_mib(port, MIB_RX_SYMBOL);
    s->lan_rx_undersize = read_mib(port, MIB_RX_UNDERSIZE);
    s->lan_rx_oversize  = read_mib(port, MIB_RX_OVERSIZE);
    s->lan_tx_good      = read_mib(port, MIB_TX_TOTAL);
    s->lan_tx_late_col  = read_mib(port, MIB_TX_LATE_COL);
    s->lan_tx_excess_col = read_mib(port, MIB_TX_EXCESS_COL);
}

static void flush_mib(uint8_t port) {
    /* Read all counters to clear them (read-to-clear) */
    uint16_t base = MIB_BASE(port);
    uint32_t dummy;
    for (uint16_t i = 0; i < 0x80; i += 4) {
        lan9646_read_reg32(g_lan, base | i, &dummy);
    }
}

static void read_all_stats(rgmii_stats_t* s) {
    read_gmac_stats(s);
    read_lan_stats(s, 6);
}

static void print_stats(const char* title, const rgmii_stats_t* s) {
    LOG_I(TAG, "");
    LOG_I(TAG, "=== %s ===", title);
    LOG_I(TAG, "GMAC (S32K388):");
    LOG_I(TAG, "  TX Good:      %lu", (unsigned long)s->gmac_tx_good);
    LOG_I(TAG, "  TX Underflow: %lu", (unsigned long)s->gmac_tx_underflow);
    LOG_I(TAG, "  RX Good:      %lu", (unsigned long)s->gmac_rx_good);
    LOG_I(TAG, "  RX CRC Err:   %lu", (unsigned long)s->gmac_rx_crc_err);
    LOG_I(TAG, "  RX Align Err: %lu", (unsigned long)s->gmac_rx_align_err);
    LOG_I(TAG, "LAN9646 Port 6:");
    LOG_I(TAG, "  RX Good:      %lu", (unsigned long)s->lan_rx_good);
    LOG_I(TAG, "  RX CRC Err:   %lu", (unsigned long)s->lan_rx_crc_err);
    LOG_I(TAG, "  RX Symbol Err:%lu", (unsigned long)s->lan_rx_symbol_err);
    LOG_I(TAG, "  TX Good:      %lu", (unsigned long)s->lan_tx_good);
    LOG_I(TAG, "  TX Late Col:  %lu", (unsigned long)s->lan_tx_late_col);
}

static Gmac_Ip_StatusType send_test_packet(uint32_t seq) {
    /* Update sequence number */
    g_test_packet[46] = (seq >> 24) & 0xFF;
    g_test_packet[47] = (seq >> 16) & 0xFF;
    g_test_packet[48] = (seq >> 8) & 0xFF;
    g_test_packet[49] = seq & 0xFF;

    Gmac_Ip_BufferType buf = {
        .Data = g_test_packet,
        .Length = sizeof(g_test_packet)
    };

    return Gmac_Ip_SendFrame(0, 0, &buf, NULL);
}

/*===========================================================================*/
/*                          PUBLIC API                                        */
/*===========================================================================*/

void rgmii_diag_init(lan9646_t* lan, void (*delay_ms)(uint32_t)) {
    g_lan = lan;
    g_delay = delay_ms;
}

const char* rgmii_diag_result_str(rgmii_test_result_t result) {
    switch (result) {
        case RGMII_TEST_PASS:           return "PASS";
        case RGMII_TEST_FAIL_TX_CLK:    return "FAIL: TX_CLK not working";
        case RGMII_TEST_FAIL_TX_DATA:   return "FAIL: TXD[0:3] problem";
        case RGMII_TEST_FAIL_TX_CTL:    return "FAIL: TX_CTL problem";
        case RGMII_TEST_FAIL_RX_CLK:    return "FAIL: RX_CLK not working";
        case RGMII_TEST_FAIL_RX_DATA:   return "FAIL: RXD[0:3] problem";
        case RGMII_TEST_FAIL_RX_CTL:    return "FAIL: RX_CTL problem";
        case RGMII_TEST_FAIL_TIMING:    return "FAIL: Timing mismatch";
        default:                        return "FAIL: Unknown";
    }
}

/*===========================================================================*/
/*                          TEST 1: CLOCKS                                    */
/*===========================================================================*/

rgmii_test_result_t rgmii_diag_test_clocks(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "#              TEST 1: CLOCK VERIFICATION                      #");
    LOG_I(TAG, "################################################################");

    /* Check S32K388 GMAC TX clock source - use SDK register definitions */
    uint32_t csc = IP_MC_CGM->MUX_8_CSC;
    uint32_t css = IP_MC_CGM->MUX_8_CSS;
    uint32_t dc0 = IP_MC_CGM->MUX_8_DC_0;

    LOG_I(TAG, "S32K388 GMAC0_TX_CLK (MUX_8):");
    LOG_I(TAG, "  CSC=0x%08lX CSS=0x%08lX DC_0=0x%08lX",
          (unsigned long)csc, (unsigned long)css, (unsigned long)dc0);

    uint8_t sel = (css >> 24) & 0x3F;
    bool div_en = (dc0 >> 31) & 1;
    uint8_t div_val = (dc0 >> 16) & 0xFF;

    LOG_I(TAG, "  Clock source: %d, Divider: %s (value=%d)",
          sel, div_en ? "ON" : "OFF", div_val + 1);

    /* Check DCM_GPR for RGMII mode */
    uint32_t dcmrwf1 = IP_DCM_GPR->DCMRWF1;
    uint32_t dcmrwf3 = IP_DCM_GPR->DCMRWF3;

    LOG_I(TAG, "S32K388 DCM_GPR:");
    LOG_I(TAG, "  DCMRWF1=0x%08lX (MAC mode=%lu)",
          (unsigned long)dcmrwf1, (unsigned long)(dcmrwf1 & 0x03));
    LOG_I(TAG, "  DCMRWF3=0x%08lX", (unsigned long)dcmrwf3);
    LOG_I(TAG, "    TX_CLK_OUT_EN=%d RX_CLK_BYPASS=%d",
          (dcmrwf3 >> 3) & 1, dcmrwf3 & 1);

    /* Check LAN9646 Port 6 status */
    uint8_t ctrl0, ctrl1, status;
    lan9646_read_reg8(g_lan, 0x6300, &ctrl0);
    lan9646_read_reg8(g_lan, 0x6301, &ctrl1);
    lan9646_read_reg8(g_lan, 0x6030, &status);

    LOG_I(TAG, "LAN9646 Port 6:");
    LOG_I(TAG, "  XMII_CTRL0=0x%02X XMII_CTRL1=0x%02X STATUS=0x%02X",
          ctrl0, ctrl1, status);

    uint8_t speed = (status >> 3) & 0x03;
    const char* speed_str[] = {"10M", "100M", "1000M", "???"};
    LOG_I(TAG, "  Speed: %s, Duplex: %s",
          speed_str[speed], (status & 0x04) ? "Full" : "Half");
    LOG_I(TAG, "  TX Delay: %s, RX Delay: %s",
          (ctrl1 & 0x08) ? "+1.3ns" : "None",
          (ctrl1 & 0x10) ? "+1.3ns" : "None");

    /* Basic validation */
    if ((dcmrwf1 & 0x03) != 2) {
        LOG_E(TAG, "ERROR: S32K388 not in RGMII mode!");
        return RGMII_TEST_FAIL_TX_CLK;
    }

    if (!(dcmrwf3 & 0x08)) {
        LOG_E(TAG, "ERROR: TX_CLK output not enabled!");
        return RGMII_TEST_FAIL_TX_CLK;
    }

    LOG_I(TAG, "RESULT: Clock configuration OK");
    return RGMII_TEST_PASS;
}

/*===========================================================================*/
/*                     TEST 2: MAC LOOPBACK                                   */
/*===========================================================================*/

rgmii_test_result_t rgmii_diag_test_mac_loopback(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "#              TEST 2: MAC LOOPBACK                            #");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "");
    LOG_I(TAG, "This test enables loopback in LAN9646 Port 6 MAC.");
    LOG_I(TAG, "Path: GMAC_TX -> RGMII -> LAN9646 -> Loopback -> RGMII -> GMAC_RX");
    LOG_I(TAG, "");

    /* Clear counters */
    flush_mib(6);
    read_all_stats(&g_stats_before);

    /* Enable Remote MAC Loopback on Port 6 */
    /* Register 0x6020 bit 6 = Remote MAC Loopback */
    uint8_t ctrl;
    lan9646_read_reg8(g_lan, 0x6020, &ctrl);
    ctrl |= 0x40;  /* Set bit 6 */
    lan9646_write_reg8(g_lan, 0x6020, ctrl);

    lan9646_read_reg8(g_lan, 0x6020, &ctrl);
    LOG_I(TAG, "Port 6 Operation Control 0: 0x%02X (Loopback=%d)",
          ctrl, (ctrl >> 6) & 1);

    /* Send test packets */
    LOG_I(TAG, "");
    LOG_I(TAG, "Sending 10 test packets...");

    uint32_t sent = 0;
    for (int i = 0; i < 10; i++) {
        if (send_test_packet(i) == GMAC_STATUS_SUCCESS) {
            sent++;
        }
        if (g_delay) g_delay(10);
    }

    LOG_I(TAG, "Sent %lu packets", (unsigned long)sent);

    /* Wait for packets to loop back */
    if (g_delay) g_delay(100);

    /* Read counters */
    read_all_stats(&g_stats_after);

    /* Disable loopback */
    lan9646_read_reg8(g_lan, 0x6020, &ctrl);
    ctrl &= ~0x40;
    lan9646_write_reg8(g_lan, 0x6020, ctrl);

    /* Analyze results */
    uint32_t gmac_tx_delta = g_stats_after.gmac_tx_good - g_stats_before.gmac_tx_good;
    uint32_t gmac_rx_delta = g_stats_after.gmac_rx_good - g_stats_before.gmac_rx_good;
    uint32_t gmac_rx_crc_delta = g_stats_after.gmac_rx_crc_err - g_stats_before.gmac_rx_crc_err;
    uint32_t lan_rx_delta = g_stats_after.lan_rx_good - g_stats_before.lan_rx_good;
    uint32_t lan_rx_crc_delta = g_stats_after.lan_rx_crc_err - g_stats_before.lan_rx_crc_err;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== RESULTS ===");
    LOG_I(TAG, "GMAC TX:       %lu packets", (unsigned long)gmac_tx_delta);
    LOG_I(TAG, "LAN9646 RX:    %lu good, %lu CRC errors",
          (unsigned long)lan_rx_delta, (unsigned long)lan_rx_crc_delta);
    LOG_I(TAG, "GMAC RX:       %lu good, %lu CRC errors",
          (unsigned long)gmac_rx_delta, (unsigned long)gmac_rx_crc_delta);

    /* Diagnose */
    LOG_I(TAG, "");
    rgmii_test_result_t result = RGMII_TEST_PASS;

    if (gmac_tx_delta == 0) {
        LOG_E(TAG, "DIAGNOSIS: GMAC not transmitting - check GMAC config");
        result = RGMII_TEST_FAIL_TX_CLK;
    }
    else if (lan_rx_delta == 0 && lan_rx_crc_delta == 0) {
        LOG_E(TAG, "DIAGNOSIS: LAN9646 not receiving anything from GMAC");
        LOG_E(TAG, "  -> Check TX_CLK, TX_CTL, TXD[0:3] signals");
        LOG_E(TAG, "  -> Possible: Open circuit or wrong pinout");
        result = RGMII_TEST_FAIL_TX_DATA;
    }
    else if (lan_rx_crc_delta > 0 && lan_rx_delta == 0) {
        LOG_E(TAG, "DIAGNOSIS: LAN9646 receives but ALL packets have CRC error");
        LOG_E(TAG, "  -> TX timing issue (data not aligned with clock)");
        LOG_E(TAG, "  -> Try different RGMII delay options");
        result = RGMII_TEST_FAIL_TIMING;
    }
    else if (lan_rx_delta > 0 && gmac_rx_delta == 0 && gmac_rx_crc_delta == 0) {
        LOG_E(TAG, "DIAGNOSIS: LAN9646 receives OK but GMAC not receiving loopback");
        LOG_E(TAG, "  -> Check RX_CLK, RX_CTL, RXD[0:3] signals");
        LOG_E(TAG, "  -> Possible: RX path hardware issue");
        result = RGMII_TEST_FAIL_RX_DATA;
    }
    else if (gmac_rx_crc_delta > 0) {
        LOG_E(TAG, "DIAGNOSIS: GMAC receives but with CRC errors");
        LOG_E(TAG, "  -> RX timing issue (LAN9646 TX delay)");
        result = RGMII_TEST_FAIL_TIMING;
    }
    else if (gmac_rx_delta == gmac_tx_delta) {
        LOG_I(TAG, "DIAGNOSIS: ALL PACKETS LOOPED BACK SUCCESSFULLY!");
        result = RGMII_TEST_PASS;
    }
    else {
        LOG_W(TAG, "DIAGNOSIS: Partial success (%lu/%lu packets)",
              (unsigned long)gmac_rx_delta, (unsigned long)gmac_tx_delta);
        result = RGMII_TEST_FAIL_TIMING;
    }

    return result;
}

/*===========================================================================*/
/*                     TEST 3: TX PATH                                        */
/*===========================================================================*/

rgmii_test_result_t rgmii_diag_test_tx_path(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "#              TEST 3: TX PATH (GMAC -> LAN9646)               #");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "");
    LOG_I(TAG, "This test checks if packets from GMAC reach LAN9646 Port 6");
    LOG_I(TAG, "");

    /* Clear counters */
    flush_mib(6);
    read_all_stats(&g_stats_before);

    /* Send packets */
    LOG_I(TAG, "Sending 20 test packets...");
    uint32_t sent = 0;
    for (int i = 0; i < 20; i++) {
        if (send_test_packet(i) == GMAC_STATUS_SUCCESS) {
            sent++;
        }
        if (g_delay) g_delay(5);
    }

    if (g_delay) g_delay(50);

    /* Read counters */
    read_all_stats(&g_stats_after);

    uint32_t gmac_tx = g_stats_after.gmac_tx_good - g_stats_before.gmac_tx_good;
    uint32_t lan_rx = g_stats_after.lan_rx_good - g_stats_before.lan_rx_good;
    uint32_t lan_crc = g_stats_after.lan_rx_crc_err - g_stats_before.lan_rx_crc_err;
    uint32_t lan_symbol = g_stats_after.lan_rx_symbol_err - g_stats_before.lan_rx_symbol_err;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== TX PATH RESULTS ===");
    LOG_I(TAG, "GMAC sent:        %lu", (unsigned long)gmac_tx);
    LOG_I(TAG, "LAN9646 received: %lu good", (unsigned long)lan_rx);
    LOG_I(TAG, "LAN9646 errors:   %lu CRC, %lu Symbol",
          (unsigned long)lan_crc, (unsigned long)lan_symbol);

    LOG_I(TAG, "");
    if (lan_rx == gmac_tx && lan_crc == 0) {
        LOG_I(TAG, "RESULT: TX PATH OK - 100%% packets received correctly");
        return RGMII_TEST_PASS;
    }
    else if (lan_rx > 0 && lan_crc > 0) {
        LOG_W(TAG, "RESULT: TX PATH PARTIAL - Some CRC errors");
        LOG_W(TAG, "  -> Adjust TX timing delay");
        return RGMII_TEST_FAIL_TIMING;
    }
    else if (lan_rx == 0 && lan_crc > 0) {
        LOG_E(TAG, "RESULT: TX PATH FAIL - All CRC errors");
        LOG_E(TAG, "  -> Severe timing issue or data line problem");
        return RGMII_TEST_FAIL_TIMING;
    }
    else {
        LOG_E(TAG, "RESULT: TX PATH FAIL - No packets received");
        LOG_E(TAG, "  -> Check TX_CLK and TXD signals");
        return RGMII_TEST_FAIL_TX_DATA;
    }
}

/*===========================================================================*/
/*                     TEST 4: RX PATH                                        */
/*===========================================================================*/

rgmii_test_result_t rgmii_diag_test_rx_path(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "#              TEST 4: RX PATH (LAN9646 -> GMAC)               #");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "");
    LOG_I(TAG, "This test requires external traffic into LAN9646 Port 1-4");
    LOG_I(TAG, "Connect a PC to one of the RJ45 ports and send packets to");
    LOG_I(TAG, "MAC address 10:11:22:77:77:77");
    LOG_I(TAG, "");

    read_all_stats(&g_stats_before);

    LOG_I(TAG, "Waiting 10 seconds for external traffic...");
    if (g_delay) g_delay(10000);

    read_all_stats(&g_stats_after);

    uint32_t gmac_rx = g_stats_after.gmac_rx_good - g_stats_before.gmac_rx_good;
    uint32_t gmac_crc = g_stats_after.gmac_rx_crc_err - g_stats_before.gmac_rx_crc_err;
    uint32_t lan_tx = g_stats_after.lan_tx_good - g_stats_before.lan_tx_good;

    LOG_I(TAG, "");
    LOG_I(TAG, "=== RX PATH RESULTS ===");
    LOG_I(TAG, "LAN9646 Port 6 TX: %lu", (unsigned long)lan_tx);
    LOG_I(TAG, "GMAC RX:           %lu good, %lu CRC errors",
          (unsigned long)gmac_rx, (unsigned long)gmac_crc);

    if (lan_tx == 0) {
        LOG_W(TAG, "RESULT: No external traffic detected");
        return RGMII_TEST_FAIL_UNKNOWN;
    }
    else if (gmac_rx > 0 && gmac_crc == 0) {
        LOG_I(TAG, "RESULT: RX PATH OK");
        return RGMII_TEST_PASS;
    }
    else if (gmac_crc > 0) {
        LOG_E(TAG, "RESULT: RX PATH FAIL - CRC errors");
        LOG_E(TAG, "  -> Adjust LAN9646 TX delay");
        return RGMII_TEST_FAIL_TIMING;
    }
    else {
        LOG_E(TAG, "RESULT: RX PATH FAIL - No packets received");
        return RGMII_TEST_FAIL_RX_DATA;
    }
}

/*===========================================================================*/
/*                     TEST 5: TIMING SWEEP                                   */
/*===========================================================================*/

void rgmii_diag_timing_sweep(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "#              TEST 5: TIMING SWEEP                            #");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "");
    LOG_I(TAG, "Testing all 4 delay combinations:");
    LOG_I(TAG, "");

    const char* delay_names[] = {
        "No delay",
        "TX delay only (+1.3ns)",
        "RX delay only (+1.3ns)",
        "Both TX+RX delay"
    };
    /* 1Gbps mode: bit6=0, bit4=RX_delay, bit3=TX_delay */
    const uint8_t delay_configs[] = {0x00, 0x08, 0x10, 0x18};

    LOG_I(TAG, "Option | Config | LAN RX | LAN CRC | GMAC RX | GMAC CRC | Status");
    LOG_I(TAG, "-------+--------+--------+---------+---------+----------+--------");

    for (int opt = 0; opt < 4; opt++) {
        /* Set delay option */
        lan9646_write_reg8(g_lan, 0x6301, delay_configs[opt]);
        if (g_delay) g_delay(10);

        /* Clear and test */
        flush_mib(6);
        read_all_stats(&g_stats_before);

        /* Enable loopback and send */
        uint8_t ctrl;
        lan9646_read_reg8(g_lan, 0x6020, &ctrl);
        ctrl |= 0x40;
        lan9646_write_reg8(g_lan, 0x6020, ctrl);

        for (int i = 0; i < 10; i++) {
            send_test_packet(i);
            if (g_delay) g_delay(5);
        }
        if (g_delay) g_delay(50);

        /* Disable loopback */
        ctrl &= ~0x40;
        lan9646_write_reg8(g_lan, 0x6020, ctrl);

        read_all_stats(&g_stats_after);

        uint32_t lan_rx = g_stats_after.lan_rx_good - g_stats_before.lan_rx_good;
        uint32_t lan_crc = g_stats_after.lan_rx_crc_err - g_stats_before.lan_rx_crc_err;
        uint32_t gmac_rx = g_stats_after.gmac_rx_good - g_stats_before.gmac_rx_good;
        uint32_t gmac_crc = g_stats_after.gmac_rx_crc_err - g_stats_before.gmac_rx_crc_err;

        const char* status;
        if (lan_rx > 0 && lan_crc == 0 && gmac_rx > 0 && gmac_crc == 0) {
            status = "<<< BEST";
        } else if (lan_rx > 0 && gmac_rx > 0) {
            status = "Partial";
        } else {
            status = "Fail";
        }

        LOG_I(TAG, "  %d    |  0x%02X  |   %2lu   |   %2lu    |   %2lu    |    %2lu    | %s",
              opt, delay_configs[opt],
              (unsigned long)lan_rx, (unsigned long)lan_crc,
              (unsigned long)gmac_rx, (unsigned long)gmac_crc,
              status);
    }

    LOG_I(TAG, "");
    LOG_I(TAG, "Select the option marked '<<< BEST' for your RGMII_DELAY_OPTION");
}

/*===========================================================================*/
/*                     RUN ALL TESTS                                          */
/*===========================================================================*/

rgmii_test_result_t rgmii_diag_run_all(void) {
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "##                                                            ##");
    LOG_I(TAG, "##          RGMII HARDWARE DIAGNOSTIC SUITE                   ##");
    LOG_I(TAG, "##          S32K388 GMAC <-> LAN9646 Port 6                    ##");
    LOG_I(TAG, "##                                                            ##");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "################################################################");

    rgmii_test_result_t result;
    rgmii_test_result_t final_result = RGMII_TEST_PASS;

    /* Test 1: Clocks */
    result = rgmii_diag_test_clocks();
    if (result != RGMII_TEST_PASS) {
        final_result = result;
        LOG_E(TAG, "TEST 1 FAILED: %s", rgmii_diag_result_str(result));
        LOG_E(TAG, "Fix clock configuration before continuing.");
        return final_result;
    }

    /* Test 2: MAC Loopback */
    result = rgmii_diag_test_mac_loopback();
    if (result != RGMII_TEST_PASS) {
        final_result = result;
        LOG_E(TAG, "TEST 2 FAILED: %s", rgmii_diag_result_str(result));
    }

    /* Test 3: TX Path */
    result = rgmii_diag_test_tx_path();
    if (result != RGMII_TEST_PASS && final_result == RGMII_TEST_PASS) {
        final_result = result;
    }

    /* Test 5: Timing sweep (always run to find best config) */
    rgmii_diag_timing_sweep();

    /* Summary */
    LOG_I(TAG, "");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "#                     DIAGNOSTIC SUMMARY                       #");
    LOG_I(TAG, "################################################################");
    LOG_I(TAG, "");

    if (final_result == RGMII_TEST_PASS) {
        LOG_I(TAG, "OVERALL RESULT: PASS - RGMII interface working correctly");
    } else {
        LOG_E(TAG, "OVERALL RESULT: FAIL - %s", rgmii_diag_result_str(final_result));
        LOG_E(TAG, "");
        LOG_E(TAG, "TROUBLESHOOTING:");

        switch (final_result) {
            case RGMII_TEST_FAIL_TX_CLK:
                LOG_E(TAG, "1. Check S32K388 TX_CLK output (should be 25MHz for 100M)");
                LOG_E(TAG, "2. Verify DCMRWF3.GMAC_TX_CLK_OUT_EN = 1");
                LOG_E(TAG, "3. Check PCB trace from S32K388 to LAN9646");
                break;

            case RGMII_TEST_FAIL_TX_DATA:
                LOG_E(TAG, "1. Check TXD0-TXD3 and TX_CTL signals");
                LOG_E(TAG, "2. Verify PCB traces are connected correctly");
                LOG_E(TAG, "3. Check for solder bridges or open circuits");
                break;

            case RGMII_TEST_FAIL_RX_CLK:
            case RGMII_TEST_FAIL_RX_DATA:
                LOG_E(TAG, "1. Check RXD0-RXD3, RX_CLK, RX_CTL signals");
                LOG_E(TAG, "2. Verify LAN9646 is driving these signals");
                LOG_E(TAG, "3. Check DCMRWF3.GMAC_RX_CLK_MUX_BYPASS setting");
                break;

            case RGMII_TEST_FAIL_TIMING:
                LOG_E(TAG, "1. Check timing sweep results above");
                LOG_E(TAG, "2. Try different RGMII_DELAY_OPTION values");
                LOG_E(TAG, "3. PCB trace length mismatch may cause this");
                break;

            default:
                LOG_E(TAG, "1. Check all RGMII connections");
                LOG_E(TAG, "2. Verify power supply to LAN9646");
                break;
        }
    }

    return final_result;
}

void rgmii_diag_print_report(void) {
    print_stats("Current Statistics", &g_stats_after);
}

