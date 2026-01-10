#ifndef LAN9646_MONITOR_FIXED_H
#define LAN9646_MONITOR_FIXED_H

#include "lan9646.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Global instance */
extern lan9646_t g_lan;

void lan9646_init_monitor(void);
void lan9646_check_links(void);
void lan9646_compare_ports(void);
void lan9646_debug_port34(void);
void lan9646_try_enable_port34(void);
void lan9646_debug_port6(void);
void lan9646_try_enable_port6(void);
void lan9646_periodic(void);

#ifdef __cplusplus
}
#endif

#endif
