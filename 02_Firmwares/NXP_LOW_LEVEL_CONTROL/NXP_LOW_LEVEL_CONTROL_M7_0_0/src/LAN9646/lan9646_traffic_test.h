/**
 * \file            lan9646_traffic_test.h
 * \brief           LAN9646 Port 6 Traffic Test Header
 */

#ifndef LAN9646_TRAFFIC_TEST_H
#define LAN9646_TRAFFIC_TEST_H

#include "lan9646.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief           Enable/disable MAC loopback
 * \param[in]       h: Device handle
 * \param[in]       port: Port number (6 or 7)
 * \param[in]       enable: true to enable, false to disable
 * \return          lan9646OK on success
 */
lan9646r_t lan9646_set_mac_loopback(lan9646_t* h, uint8_t port, bool enable);

/**
 * \brief           Enable/disable Remote MAC loopback
 * \param[in]       h: Device handle
 * \param[in]       port: Port number
 * \param[in]       enable: true to enable, false to disable
 * \return          lan9646OK on success
 */
lan9646r_t lan9646_set_remote_loopback(lan9646_t* h, uint8_t port, bool enable);

/**
 * \brief           Monitor traffic counters over time
 * \param[in]       h: Device handle
 * \param[in]       port: Port to monitor
 * \param[in]       duration_ms: Monitoring duration in milliseconds
 * \param[in]       delay_fn: Platform delay function (takes ms parameter)
 */
void lan9646_traffic_test_monitor(lan9646_t* h, uint8_t port,
                                   uint32_t duration_ms,
                                   void (*delay_fn)(uint32_t));

/**
 * \brief           Test MAC loopback functionality
 * \param[in]       h: Device handle
 * \param[in]       port: Port to test
 * \param[in]       delay_fn: Platform delay function
 */
void lan9646_traffic_test_mac_loopback(lan9646_t* h, uint8_t port,
                                        void (*delay_fn)(uint32_t));

/**
 * \brief           Test remote loopback functionality
 * \param[in]       h: Device handle
 * \param[in]       port: Port to test
 * \param[in]       delay_fn: Platform delay function
 */
void lan9646_traffic_test_remote_loopback(lan9646_t* h, uint8_t port,
                                           void (*delay_fn)(uint32_t));

/**
 * \brief           Check and display error counters
 * \param[in]       h: Device handle
 * \param[in]       port: Port to check
 */
void lan9646_traffic_test_errors(lan9646_t* h, uint8_t port);

/**
 * \brief           Run all traffic tests
 * \param[in]       h: Device handle
 * \param[in]       delay_fn: Platform delay function
 */
void lan9646_traffic_test_all(lan9646_t* h, void (*delay_fn)(uint32_t));

#ifdef __cplusplus
}
#endif

#endif /* LAN9646_TRAFFIC_TEST_H */

