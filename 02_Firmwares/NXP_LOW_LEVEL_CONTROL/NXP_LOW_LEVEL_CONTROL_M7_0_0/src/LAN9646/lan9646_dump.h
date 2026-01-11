/**
 * \file            lan9646_dump.h
 * \brief           LAN9646 Complete Register Dump Functions
 */

#ifndef LAN9646_DUMP_HDR_H
#define LAN9646_DUMP_HDR_H

#include "lan9646.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/*                          DUMP CONFIGURATION                                */
/*===========================================================================*/

typedef struct {
    bool global_regs;       /*!< Dump global registers */
    bool port_status;       /*!< Dump port status summary */
    bool port_regs[8];      /*!< Dump specific port [1-4, 6-7] */
    bool phy_regs;          /*!< Dump PHY registers (port 1-4) */
    bool mib_counters;      /*!< Dump MIB counters */
    bool vlan_table;        /*!< Dump VLAN table */
    bool mac_table;         /*!< Dump MAC address table */
} lan9646_dump_cfg_t;

/*===========================================================================*/
/*                          MAIN DUMP FUNCTIONS                               */
/*===========================================================================*/

/**
 * \brief           Dump ALL LAN9646 registers (comprehensive)
 */
void lan9646_dump_all_registers(lan9646_t* h);

/**
 * \brief           Dump only Port 6 (GMAC) related registers
 */
void lan9646_dump_port6_only(lan9646_t* h);

/**
 * \brief           Custom dump with configuration
 */
void lan9646_dump_custom(lan9646_t* h, const lan9646_dump_cfg_t* cfg);

/*===========================================================================*/
/*                          SPECIFIC DUMP FUNCTIONS                           */
/*===========================================================================*/

/**
 * \brief           Dump global registers only
 */
void lan9646_dump_global(lan9646_t* h);

/**
 * \brief           Dump single port registers
 */
void lan9646_dump_port(lan9646_t* h, uint8_t port);

/**
 * \brief           Dump PHY registers for ports 1-4
 */
void lan9646_dump_phy(lan9646_t* h, uint8_t port);

/**
 * \brief           Dump XMII/RGMII registers for port 6-7
 */
void lan9646_dump_xmii(lan9646_t* h, uint8_t port);

/**
 * \brief           Dump MIB counters for a port
 */
void lan9646_dump_mib(lan9646_t* h, uint8_t port);

/**
 * \brief           Dump all MIB counters for all ports
 */
void lan9646_dump_all_mib(lan9646_t* h);

/**
 * \brief           Dump port status summary table
 */
void lan9646_dump_status_summary(lan9646_t* h);

/**
 * \brief           Dump GMAC configuration check
 */
void lan9646_dump_gmac_check(lan9646_t* h);

/**
 * \brief           Dump VLAN table entries
 */
void lan9646_dump_vlan_table(lan9646_t* h, uint16_t start_vid, uint16_t count);

/**
 * \brief           Dump port membership table
 */
void lan9646_dump_membership(lan9646_t* h);

/**
 * \brief           Quick diagnostic dump (most useful info)
 */
void lan9646_dump_quick(lan9646_t* h);

#ifdef __cplusplus
}
#endif

#endif /* LAN9646_DUMP_HDR_H */


