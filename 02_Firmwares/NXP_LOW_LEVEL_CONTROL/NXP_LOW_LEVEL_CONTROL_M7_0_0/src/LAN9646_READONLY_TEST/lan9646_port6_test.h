#ifndef LAN9646_PORT6_TEST_H
#define LAN9646_PORT6_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

void lan9646_enable_switch_operation(void);
void lan9646_enable_port_mirroring_to_port6(void);
void lan9646_check_vlan_config(void);
void lan9646_enable_unknown_unicast_to_port6(void);
void lan9646_setup_vlan_table_port6(void);
void lan9646_port6_read_mib(void);
void lan9646_port6_enable_switch(void);
void lan9646_port6_enable_forwarding(void);
void lan9646_port6_test_loopback(void);
void lan9646_port6_monitor_traffic(void);
void lan9646_port6_dump_mac(void);
void lan9646_port6_debug_no_traffic(void);
void lan9646_port6_test_rx_from_port1(void);

#ifdef __cplusplus
}
#endif

#endif /* LAN9646_PORT6_TEST_H */

