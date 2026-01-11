/**
 * \file            lan9646.h
 * \brief           LAN9646 Ethernet Switch Driver - Low Level Register Access
 * \note            Fixed according to LAN9646 Datasheet DS00005175B
 */

#ifndef LAN9646_HDR_H
#define LAN9646_HDR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/*                              RETURN CODES                                  */
/*===========================================================================*/

typedef enum {
    lan9646OK = 0,
    lan9646ERR,
    lan9646TIMEOUT,
    lan9646INVPARAM,
    lan9646BUSERR,
} lan9646r_t;

/*===========================================================================*/
/*                           INTERFACE TYPES                                  */
/*===========================================================================*/

typedef enum {
    LAN9646_IF_SPI = 0,
    LAN9646_IF_I2C,
    LAN9646_IF_MIIM,
} lan9646_if_t;

/*===========================================================================*/
/*                           CALLBACK STRUCTURES                              */
/*===========================================================================*/

typedef struct {
    lan9646r_t (*init_fn)(void);
    lan9646r_t (*write_fn)(const uint8_t* data, uint16_t len);
    lan9646r_t (*read_fn)(uint8_t* data, uint16_t len);
    lan9646r_t (*transfer_fn)(const uint8_t* tx_data, uint8_t* rx_data, uint16_t len);
    void (*cs_low_fn)(void);
    void (*cs_high_fn)(void);
} lan9646_spi_t;

typedef struct {
    lan9646r_t (*init_fn)(void);
    lan9646r_t (*write_fn)(uint8_t dev_addr, const uint8_t* data, uint16_t len);
    lan9646r_t (*read_fn)(uint8_t dev_addr, uint8_t* data, uint16_t len);
    lan9646r_t (*mem_write_fn)(uint8_t dev_addr, uint16_t mem_addr, const uint8_t* data, uint16_t len);
    lan9646r_t (*mem_read_fn)(uint8_t dev_addr, uint16_t mem_addr, uint8_t* data, uint16_t len);
} lan9646_i2c_t;

typedef struct {
    lan9646r_t (*init_fn)(void);
    lan9646r_t (*write_fn)(uint8_t phy_addr, uint8_t reg_addr, uint16_t data);
    lan9646r_t (*read_fn)(uint8_t phy_addr, uint8_t reg_addr, uint16_t* data);
} lan9646_miim_t;

typedef struct {
    lan9646_if_t if_type;
    union {
        lan9646_spi_t spi;
        lan9646_i2c_t i2c;
        lan9646_miim_t miim;
    } ops;
    uint8_t i2c_addr;
    uint8_t phy_addr;
} lan9646_cfg_t;

typedef struct {
    lan9646_cfg_t cfg;
    uint8_t is_init;
} lan9646_t;

/*===========================================================================*/
/*                              SPI COMMANDS                                  */
/*===========================================================================*/

#define LAN9646_SPI_CMD_READ        0x03
#define LAN9646_SPI_CMD_WRITE       0x02
#define LAN9646_SPI_CMD_FAST_READ   0x0B

/*===========================================================================*/
/*                           I2C ADDRESS                                      */
/*===========================================================================*/

#define LAN9646_I2C_ADDR_DEFAULT    0x5F

/*===========================================================================*/
/*                           CHIP ID (Datasheet: 0x9477)                      */
/*===========================================================================*/

#define LAN9646_CHIP_ID             0x9477  /*!< Full 16-bit Chip ID (MSB:LSB) */
#define LAN9646_CHIP_ID_MSB         0x94    /*!< Chip ID MSB at 0x0001 */
#define LAN9646_CHIP_ID_LSB         0x77    /*!< Chip ID LSB at 0x0002 */

/*===========================================================================*/
/*                    GLOBAL OPERATION CONTROL (0x0000-0x00FF)                */
/*===========================================================================*/

/* Chip ID Registers */
#define LAN9646_REG_CHIP_ID0        0x0000  /*!< Fixed value = 0x00 */
#define LAN9646_REG_CHIP_ID1        0x0001  /*!< Chip ID MSB = 0x94 */
#define LAN9646_REG_CHIP_ID2        0x0002  /*!< Chip ID LSB = 0x77 */
#define LAN9646_REG_CHIP_ID3        0x0003  /*!< Revision [7:4] + Reset [0] */

/* PME Pin Control */
#define LAN9646_REG_PME_PIN_CTRL    0x0006  /*!< PME Pin Control */

/* Global Interrupt (32-bit) */
#define LAN9646_REG_GLOBAL_INT_STAT 0x0010  /*!< Global Interrupt Status (32-bit) */
#define LAN9646_REG_GLOBAL_INT_MASK 0x0014  /*!< Global Interrupt Mask (32-bit) */

/* Global Port Interrupt */
#define LAN9646_REG_GPORT_INT_STAT  0x0018  /*!< Global Port Interrupt Status */
#define LAN9646_REG_GPORT_INT_MASK  0x001C  /*!< Global Port Interrupt Mask */

/* Global Control (backward compat) */
#define LAN9646_REG_GLOBAL_CTRL     0x0003  /*!< Same as CHIP_ID3 - Reset bit */

/*===========================================================================*/
/*                    GLOBAL I/O CONTROL (0x0100-0x01FF)                      */
/*===========================================================================*/

#define LAN9646_REG_IO_CTRL0        0x0100
#define LAN9646_REG_LED_OVERRIDE    0x0120
#define LAN9646_REG_LED_OUTPUT      0x0124

/*===========================================================================*/
/*                    GLOBAL PHY CONTROL (0x0200-0x02FF)                      */
/*===========================================================================*/

#define LAN9646_REG_PHY_POWER       0x0201

/*===========================================================================*/
/*                    GLOBAL SWITCH CONTROL (0x0300-0x03FF)                   */
/*===========================================================================*/

/* Switch Operation */
#define LAN9646_REG_SWITCH_OP       0x0300  /*!< Switch Operation */

/* Lookup Engine Control */
#define LAN9646_REG_LUE_CTRL0       0x0310  /*!< LUE Control 0 */
#define LAN9646_REG_LUE_CTRL1       0x0311  /*!< LUE Control 1 */
#define LAN9646_REG_LUE_CTRL2       0x0312  /*!< LUE Control 2 */
#define LAN9646_REG_AGE_PERIOD      0x0313  /*!< Age Period (seconds) */

/* Address Lookup Table Interrupt */
#define LAN9646_REG_ALU_INT_STAT    0x0314  /*!< ALU Interrupt Status */
#define LAN9646_REG_ALU_INT_MASK    0x0315  /*!< ALU Interrupt Mask */

/* Unknown Destination Control */
#define LAN9646_REG_UNKNOWN_UCAST   0x0320  /*!< Unknown Unicast Ctrl */
#define LAN9646_REG_UNKNOWN_MCAST   0x0324  /*!< Unknown Multicast Ctrl */
#define LAN9646_REG_UNKNOWN_VID     0x0328  /*!< Unknown VID Ctrl */

/* Global Port Mirroring */
#define LAN9646_REG_GLOBAL_MIRROR   0x0370  /*!< Global Mirror Control */
#define LAN9646_REG_MIRROR_DSCP     0x0378  /*!< Mirror DSCP */

/* Queue Management */
#define LAN9646_REG_QUEUE_MGMT_CTRL 0x0390  /*!< Queue Management Control 0 */

/* Switch MAC Address */
#define LAN9646_REG_SWITCH_MAC0     0x0302  /*!< Switch MAC Address [47:40] */
#define LAN9646_REG_SWITCH_MAC1     0x0303  /*!< Switch MAC Address [39:32] */
#define LAN9646_REG_SWITCH_MAC2     0x0304  /*!< Switch MAC Address [31:24] */
#define LAN9646_REG_SWITCH_MAC3     0x0305  /*!< Switch MAC Address [23:16] */
#define LAN9646_REG_SWITCH_MAC4     0x0306  /*!< Switch MAC Address [15:8] */
#define LAN9646_REG_SWITCH_MAC5     0x0307  /*!< Switch MAC Address [7:0] */

/* Switch MIB Control */
#define LAN9646_REG_SWITCH_MIB_CTRL 0x0308  /*!< Switch MIB Control */

/*===========================================================================*/
/*                    GLOBAL LUE CONTROL (0x0400-0x04FF)                      */
/*===========================================================================*/

/* ALU Table Access */
#define LAN9646_REG_ALU_TABLE_CTRL  0x0410  /*!< ALU Table Access Control */
#define LAN9646_REG_ALU_TABLE_INDEX 0x0414  /*!< ALU Table Entry Index */
#define LAN9646_REG_ALU_TABLE_ENTRY0 0x0420  /*!< ALU Table Entry 0 */
#define LAN9646_REG_ALU_TABLE_ENTRY1 0x0424  /*!< ALU Table Entry 1 */
#define LAN9646_REG_ALU_TABLE_ENTRY2 0x0428  /*!< ALU Table Entry 2 */
#define LAN9646_REG_ALU_TABLE_ENTRY3 0x042C  /*!< ALU Table Entry 3 */

/* Static Table Access */
#define LAN9646_REG_STATIC_TABLE_CTRL 0x0440

/* VLAN Table (at 0x0480 with VID offset) */
#define LAN9646_REG_VLAN_TABLE_BASE 0x0480

/*===========================================================================*/
/*                        PORT REGISTERS (0xN000-0xNFFF)                      */
/*   N = Port Number: 1-4 (PHY), 5 (reserved), 6-7 (RGMII)                   */
/*===========================================================================*/

#define LAN9646_PORT_BASE(n)        ((uint16_t)(n) << 12)

/* Port Default Tag (0xN000-0xN003) */
#define LAN9646_REG_PORT_DEFAULT_TAG0(n)    (LAN9646_PORT_BASE(n) | 0x0000)
#define LAN9646_REG_PORT_DEFAULT_TAG1(n)    (LAN9646_PORT_BASE(n) | 0x0001)

/* Port PME/WoL (0xN013, 0xN017) */
#define LAN9646_REG_PORT_PME_EVENT(n)       (LAN9646_PORT_BASE(n) | 0x0013)
#define LAN9646_REG_PORT_PME_ENABLE(n)      (LAN9646_PORT_BASE(n) | 0x0017)

/* Port Interrupt (0xN01B, 0xN01F) */
#define LAN9646_REG_PORT_INT_STATUS(n)      (LAN9646_PORT_BASE(n) | 0x001B)
#define LAN9646_REG_PORT_INT_MASK(n)        (LAN9646_PORT_BASE(n) | 0x001F)

/* Port Operation Control (0xN020-0xN02F) */
#define LAN9646_REG_PORT_OP_CTRL0(n)        (LAN9646_PORT_BASE(n) | 0x0020)
#define LAN9646_REG_PORT_OP_CTRL1(n)        (LAN9646_PORT_BASE(n) | 0x0021)

/* Port Status (0xN030) - 8-bit for ALL ports */
#define LAN9646_REG_PORT_STATUS(n)          (LAN9646_PORT_BASE(n) | 0x0030)

/*---------------------------------------------------------------------------*/
/* Port PHY Registers (0xN100-0xN1FF) - Only for Port 1-4                    */
/*---------------------------------------------------------------------------*/
#define LAN9646_REG_PORT_PHY_BASIC_CTRL(n)  (LAN9646_PORT_BASE(n) | 0x0100)
#define LAN9646_REG_PORT_PHY_BASIC_STAT(n)  (LAN9646_PORT_BASE(n) | 0x0102)
#define LAN9646_REG_PORT_PHY_ID_H(n)        (LAN9646_PORT_BASE(n) | 0x0104)
#define LAN9646_REG_PORT_PHY_ID_L(n)        (LAN9646_PORT_BASE(n) | 0x0106)
#define LAN9646_REG_PORT_PHY_AUTONEG_ADV(n) (LAN9646_PORT_BASE(n) | 0x0108)
#define LAN9646_REG_PORT_PHY_LINK_PARTNER(n)(LAN9646_PORT_BASE(n) | 0x010A)
#define LAN9646_REG_PORT_PHY_AUTONEG_EXP(n) (LAN9646_PORT_BASE(n) | 0x010C)
#define LAN9646_REG_PORT_PHY_AUTONEG_NP(n)  (LAN9646_PORT_BASE(n) | 0x010E)
#define LAN9646_REG_PORT_PHY_LP_NP(n)       (LAN9646_PORT_BASE(n) | 0x0110)
#define LAN9646_REG_PORT_PHY_1000_CTRL(n)   (LAN9646_PORT_BASE(n) | 0x0112)
#define LAN9646_REG_PORT_PHY_1000_STAT(n)   (LAN9646_PORT_BASE(n) | 0x0114)
#define LAN9646_REG_PORT_PHY_MMD_SETUP(n)   (LAN9646_PORT_BASE(n) | 0x0134)
#define LAN9646_REG_PORT_PHY_MMD_DATA(n)    (LAN9646_PORT_BASE(n) | 0x0136)
#define LAN9646_REG_PORT_PHY_EXT_STAT(n)    (LAN9646_PORT_BASE(n) | 0x013E)

/*---------------------------------------------------------------------------*/
/* Port SGMII Registers (0xN200-0xN2FF) - Port 7 only                        */
/*---------------------------------------------------------------------------*/
#define LAN9646_REG_PORT_SGMII_ADDR(n)      (LAN9646_PORT_BASE(n) | 0x0200)
#define LAN9646_REG_PORT_SGMII_DATA(n)      (LAN9646_PORT_BASE(n) | 0x0206)

/*---------------------------------------------------------------------------*/
/* Port XMII/RGMII Control (0xN300-0xN3FF) - Port 6-7                        */
/*---------------------------------------------------------------------------*/
#define LAN9646_REG_PORT_XMII_CTRL0(n)      (LAN9646_PORT_BASE(n) | 0x0300)
#define LAN9646_REG_PORT_XMII_CTRL1(n)      (LAN9646_PORT_BASE(n) | 0x0301)

/*---------------------------------------------------------------------------*/
/* Port MAC Control (0xN400-0xN4FF)                                          */
/*---------------------------------------------------------------------------*/
#define LAN9646_REG_PORT_MAC_CTRL0(n)       (LAN9646_PORT_BASE(n) | 0x0400)
#define LAN9646_REG_PORT_MAC_CTRL1(n)       (LAN9646_PORT_BASE(n) | 0x0401)
#define LAN9646_REG_PORT_IN_RATE(n)         (LAN9646_PORT_BASE(n) | 0x0410)
#define LAN9646_REG_PORT_PRI_RATE(n)        (LAN9646_PORT_BASE(n) | 0x0411)
#define LAN9646_REG_PORT_OUT_RATE(n)        (LAN9646_PORT_BASE(n) | 0x0420)

/*---------------------------------------------------------------------------*/
/* Port MIB Counters (0xN500-0xN5FF) - Indirect Access                       */
/*---------------------------------------------------------------------------*/
#define LAN9646_REG_PORT_MIB_CTRL(n)        (LAN9646_PORT_BASE(n) | 0x0500)
#define LAN9646_REG_PORT_MIB_DATA(n)        (LAN9646_PORT_BASE(n) | 0x0504)

/*---------------------------------------------------------------------------*/
/* Port ACL (0xN600-0xN6FF)                                                  */
/*---------------------------------------------------------------------------*/
#define LAN9646_REG_PORT_ACL_CTRL0(n)       (LAN9646_PORT_BASE(n) | 0x0680)
#define LAN9646_REG_PORT_ACL_CTRL1(n)       (LAN9646_PORT_BASE(n) | 0x0681)
#define LAN9646_REG_PORT_ACL_ACCESS(n,i)    (LAN9646_PORT_BASE(n) | (0x0600 + (i)))

/*---------------------------------------------------------------------------*/
/* Port Ingress Control (0xN800-0xN8FF)                                      */
/*---------------------------------------------------------------------------*/
#define LAN9646_REG_PORT_CLASS_CTRL(n)      (LAN9646_PORT_BASE(n) | 0x0800)
#define LAN9646_REG_PORT_MIRROR_CTRL(n)     (LAN9646_PORT_BASE(n) | 0x0804)
#define LAN9646_REG_PORT_PRIO_CTRL(n)       (LAN9646_PORT_BASE(n) | 0x0808)

/*---------------------------------------------------------------------------*/
/* Port Egress Control (0xN900-0xN9FF)                                       */
/*---------------------------------------------------------------------------*/
#define LAN9646_REG_PORT_SCHED_CTRL(n)      (LAN9646_PORT_BASE(n) | 0x0900)
#define LAN9646_REG_PORT_SHAPING_CTRL(n)    (LAN9646_PORT_BASE(n) | 0x0904)

/*---------------------------------------------------------------------------*/
/* Port Queue Management (0xNA00-0xNAFF)                                     */
/*---------------------------------------------------------------------------*/
#define LAN9646_REG_PORT_QUEUE_CTRL(n)      (LAN9646_PORT_BASE(n) | 0x0A00)
#define LAN9646_REG_PORT_MEMBERSHIP(n)      (LAN9646_PORT_BASE(n) | 0x0A04)

/*---------------------------------------------------------------------------*/
/* Port Address Lookup (0xNB00-0xNBFF)                                       */
/*---------------------------------------------------------------------------*/
#define LAN9646_REG_PORT_CTRL2(n)           (LAN9646_PORT_BASE(n) | 0x0B00)
#define LAN9646_REG_PORT_MSTP_PTR(n)        (LAN9646_PORT_BASE(n) | 0x0B01)
#define LAN9646_REG_PORT_MSTP_STATE(n)      (LAN9646_PORT_BASE(n) | 0x0B04)

/*===========================================================================*/
/*                           MIB COUNTER INDICES                              */
/*   According to Datasheet Table 5-6                                        */
/*===========================================================================*/

#define LAN9646_MIB_RX_HI_PRIO_BYTE    0x00  /*!< RX high priority bytes */
#define LAN9646_MIB_RX_UNDERSIZE       0x01  /*!< RX undersize packets */
#define LAN9646_MIB_RX_FRAGMENT        0x02  /*!< RX fragments */
#define LAN9646_MIB_RX_OVERSIZE        0x03  /*!< RX oversize packets */
#define LAN9646_MIB_RX_JABBER          0x04  /*!< RX jabbers */
#define LAN9646_MIB_RX_SYMBOL_ERR      0x05  /*!< RX symbol errors */
#define LAN9646_MIB_RX_CRC_ERR         0x06  /*!< RX CRC errors */
#define LAN9646_MIB_RX_ALIGN_ERR       0x07  /*!< RX alignment errors */
#define LAN9646_MIB_RX_CTRL_8808       0x08  /*!< RX 0x8808 control frames */
#define LAN9646_MIB_RX_PAUSE           0x09  /*!< RX pause frames */
#define LAN9646_MIB_RX_BROADCAST       0x0A  /*!< RX broadcast */
#define LAN9646_MIB_RX_MULTICAST       0x0B  /*!< RX multicast */
#define LAN9646_MIB_RX_UNICAST         0x0C  /*!< RX unicast */
#define LAN9646_MIB_RX_64              0x0D  /*!< RX 64 bytes */
#define LAN9646_MIB_RX_65_127          0x0E  /*!< RX 65-127 bytes */
#define LAN9646_MIB_RX_128_255         0x0F  /*!< RX 128-255 bytes */
#define LAN9646_MIB_RX_256_511         0x10  /*!< RX 256-511 bytes */
#define LAN9646_MIB_RX_512_1023        0x11  /*!< RX 512-1023 bytes */
#define LAN9646_MIB_RX_1024_1522       0x12  /*!< RX 1024-1522 bytes */
#define LAN9646_MIB_RX_1523_2000       0x13  /*!< RX 1523-2000 bytes */
#define LAN9646_MIB_RX_2001_PLUS       0x14  /*!< RX 2001+ bytes */

#define LAN9646_MIB_TX_HI_PRIO_BYTE    0x60  /*!< TX high priority bytes */
#define LAN9646_MIB_TX_LATE_COL        0x61  /*!< TX late collisions */
#define LAN9646_MIB_TX_PAUSE           0x62  /*!< TX pause frames */
#define LAN9646_MIB_TX_BROADCAST       0x63  /*!< TX broadcast */
#define LAN9646_MIB_TX_MULTICAST       0x64  /*!< TX multicast */
#define LAN9646_MIB_TX_UNICAST         0x65  /*!< TX unicast */
#define LAN9646_MIB_TX_DEFERRED        0x66  /*!< TX deferred */
#define LAN9646_MIB_TX_TOTAL_COL       0x67  /*!< TX total collisions */
#define LAN9646_MIB_TX_EXCESS_COL      0x68  /*!< TX excessive collisions */
#define LAN9646_MIB_TX_SINGLE_COL      0x69  /*!< TX single collision */
#define LAN9646_MIB_TX_MULTI_COL       0x6A  /*!< TX multiple collisions */

#define LAN9646_MIB_RX_TOTAL           0x80  /*!< Total RX packet count */
#define LAN9646_MIB_TX_TOTAL           0x81  /*!< Total TX byte count */
#define LAN9646_MIB_RX_DROP            0x82  /*!< RX dropped packets */
#define LAN9646_MIB_TX_DROP            0x83  /*!< TX dropped packets */

/*===========================================================================*/
/*                           BIT DEFINITIONS                                  */
/*===========================================================================*/

/* Chip ID 3 (0x0003) */
#define LAN9646_CHIP_REV_MASK               0xF0
#define LAN9646_CHIP_REV_SHIFT              4
#define LAN9646_GLOBAL_SW_RESET             0x01

/* Port Status (0xN030) - PHY Ports 1-4, 8-bit register */
/* Bits 7:5 = Reserved */
#define LAN9646_PORT_STATUS_OP_SPEED_MASK   0x18    /*!< Bits 4:3: Speed */
#define LAN9646_PORT_STATUS_OP_SPEED_SHIFT  3
#define LAN9646_PORT_STATUS_OP_DUPLEX       0x04    /*!< Bit 2: Duplex 1=Full */
#define LAN9646_PORT_STATUS_TX_FLOW         0x02    /*!< Bit 1: TX Flow Control */
#define LAN9646_PORT_STATUS_RX_FLOW         0x01    /*!< Bit 0: RX Flow Control */

/* PHY Basic Status Register (0xN102) - For Link Status */
#define LAN9646_REG_PHY_BASIC_STATUS(n)     (LAN9646_PORT_BASE(n) | 0x0102)
#define LAN9646_PHY_LINK_STATUS             0x0004  /*!< Bit 2: Link 1=Up */
#define LAN9646_PHY_AN_COMPLETE             0x0020  /*!< Bit 5: AN complete */

/* Speed values for all ports */
#define LAN9646_SPEED_10                    0
#define LAN9646_SPEED_100                   1
#define LAN9646_SPEED_1000                  2

/* XMII Port Control 0 (0xN300) */
#define LAN9646_XMII_DUPLEX                 0x40    /*!< Full duplex */
#define LAN9646_XMII_TX_FLOW_EN             0x20    /*!< TX flow control */
#define LAN9646_XMII_SPEED_100              0x10    /*!< 100Mbps (when not 1000) */
#define LAN9646_XMII_RX_FLOW_EN             0x08    /*!< RX flow control */

/* XMII Port Control 1 (0xN301) - CRITICAL: Corrected per datasheet! */
#define LAN9646_XMII_SPEED_1000             0x40    /*!< Bit 6: 0=1000Mbps, 1=10/100 */
#define LAN9646_XMII_RGMII_RX_DLY_EN        0x10    /*!< Bit 4: RX Ingress delay 1.5ns */
#define LAN9646_XMII_RGMII_TX_DLY_EN        0x08    /*!< Bit 3: TX Egress delay 1.5ns */
#define LAN9646_XMII_MII_RMII_MODE          0x04    /*!< Bit 2: MII/RMII mode select */

/* Port Mirror Control (0xN804) */
#define LAN9646_MIRROR_RX_SNIFF             0x40
#define LAN9646_MIRROR_TX_SNIFF             0x20
#define LAN9646_MIRROR_SNIFFER_PORT         0x02

/* Port MSTP State (0xNB04) */
#define LAN9646_MSTP_TX_EN                  0x04
#define LAN9646_MSTP_RX_EN                  0x02
#define LAN9646_MSTP_LEARN_DIS              0x01

/* MIB Control (0xN500-0xN503) */
#define LAN9646_MIB_OVERFLOW                0x80000000UL
#define LAN9646_MIB_READ_EN                 0x02000000UL  /*!< Bit 25: Read Enable */
#define LAN9646_MIB_FLUSH_FREEZE_EN         0x01000000UL  /*!< Bit 24: Flush/Freeze En */
#define LAN9646_MIB_INDEX_MASK              0x00FF0000UL  /*!< Bits [23:16]: MIB Index */
#define LAN9646_MIB_INDEX_SHIFT             16

/* Switch MIB Control (0x0308) */
#define LAN9646_SW_MIB_FREEZE               0x40
#define LAN9646_SW_MIB_FLUSH                0x80

/* LUE Control 0 (0x0310) */
#define LAN9646_LUE_HASH_OPTION             0x80
#define LAN9646_LUE_UNICAST_EN              0x40
#define LAN9646_LUE_MULTICAST_EN            0x20
#define LAN9646_LUE_VLAN_EN                 0x10
#define LAN9646_LUE_AGE_CNT_MASK            0x0E
#define LAN9646_LUE_AGE_CNT_SHIFT           1
#define LAN9646_LUE_LEARNING_DIS            0x01

/* Port VLAN Membership (0xNA04) */
#define LAN9646_VLAN_MEMBERSHIP_MASK        0x7F

/*===========================================================================*/
/*                           API FUNCTIONS                                    */
/*===========================================================================*/

lan9646r_t lan9646_init(lan9646_t* handle, const lan9646_cfg_t* cfg);
lan9646r_t lan9646_deinit(lan9646_t* handle);

lan9646r_t lan9646_read_reg8(lan9646_t* handle, uint16_t reg_addr, uint8_t* data);
lan9646r_t lan9646_write_reg8(lan9646_t* handle, uint16_t reg_addr, uint8_t data);
lan9646r_t lan9646_read_reg16(lan9646_t* handle, uint16_t reg_addr, uint16_t* data);
lan9646r_t lan9646_write_reg16(lan9646_t* handle, uint16_t reg_addr, uint16_t data);
lan9646r_t lan9646_read_reg32(lan9646_t* handle, uint16_t reg_addr, uint32_t* data);
lan9646r_t lan9646_write_reg32(lan9646_t* handle, uint16_t reg_addr, uint32_t data);

lan9646r_t lan9646_read_burst(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len);
lan9646r_t lan9646_write_burst(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len);

lan9646r_t lan9646_modify_reg8(lan9646_t* handle, uint16_t reg_addr, uint8_t mask, uint8_t value);
lan9646r_t lan9646_modify_reg16(lan9646_t* handle, uint16_t reg_addr, uint16_t mask, uint16_t value);

lan9646r_t lan9646_get_chip_id(lan9646_t* handle, uint16_t* chip_id, uint8_t* revision);
lan9646r_t lan9646_soft_reset(lan9646_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* LAN9646_HDR_H */

