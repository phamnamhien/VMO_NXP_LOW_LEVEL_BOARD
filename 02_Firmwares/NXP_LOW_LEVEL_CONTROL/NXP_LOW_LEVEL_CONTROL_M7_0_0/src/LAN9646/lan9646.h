/**
 * \file            lan9646.h
 * \brief           LAN9646 Ethernet Switch Driver
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
#ifndef LAN9646_HDR_H
#define LAN9646_HDR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Status return codes
 */
typedef enum {
    lan9646OK = 0,   /*!< Operation succeeded */
    lan9646ERR,      /*!< General error */
    lan9646TIMEOUT,  /*!< Operation timeout */
    lan9646INVPARAM, /*!< Invalid parameter */
    lan9646BUSERR,   /*!< Bus communication error */
} lan9646r_t;

/**
 * \brief           Communication interface types
 */
typedef enum {
    LAN9646_IF_SPI = 0, /*!< SPI interface (up to 50MHz) */
    LAN9646_IF_I2C,     /*!< I2C interface */
    LAN9646_IF_MIIM,    /*!< MIIM (MDC/MDIO) interface */
} lan9646_if_t;

/**
 * \brief           SPI operation callback structure
 */
typedef struct {
    lan9646r_t (*init_fn)(void);  /*!< Initialize SPI peripheral */
    lan9646r_t (*write_fn)(const uint8_t* data,
                           uint16_t len);                               /*!< Write data via SPI */
    lan9646r_t (*read_fn)(uint8_t* data, uint16_t len);                /*!< Read data via SPI */
    lan9646r_t (*transfer_fn)(const uint8_t* tx_data, uint8_t* rx_data,
                              uint16_t len);                            /*!< Full-duplex SPI transfer */
    void (*cs_low_fn)(void);                                            /*!< Assert CS (active low) */
    void (*cs_high_fn)(void);                                           /*!< De-assert CS */
} lan9646_spi_t;

/**
 * \brief           I2C operation callback structure
 */
typedef struct {
    lan9646r_t (*init_fn)(void); /*!< Initialize I2C peripheral */
    lan9646r_t (*write_fn)(uint8_t dev_addr, const uint8_t* data,
                           uint16_t len);                                       /*!< Write data via I2C */
    lan9646r_t (*read_fn)(uint8_t dev_addr, uint8_t* data, uint16_t len);      /*!< Read data via I2C */
    lan9646r_t (*mem_write_fn)(uint8_t dev_addr, uint16_t mem_addr, const uint8_t* data,
                               uint16_t len);                                   /*!< Memory write via I2C */
    lan9646r_t (*mem_read_fn)(uint8_t dev_addr, uint16_t mem_addr, uint8_t* data,
                              uint16_t len);                                    /*!< Memory read via I2C */
} lan9646_i2c_t;

/**
 * \brief           MIIM operation callback structure
 */
typedef struct {
    lan9646r_t (*init_fn)(void); /*!< Initialize MIIM peripheral */
    lan9646r_t (*write_fn)(uint8_t phy_addr, uint8_t reg_addr,
                           uint16_t data);                                 /*!< Write PHY register */
    lan9646r_t (*read_fn)(uint8_t phy_addr, uint8_t reg_addr,
                          uint16_t* data);                                 /*!< Read PHY register */
} lan9646_miim_t;

/**
 * \brief           Device configuration structure
 */
typedef struct {
    lan9646_if_t if_type; /*!< Interface type selection */

    union {
        lan9646_spi_t spi;   /*!< SPI operations */
        lan9646_i2c_t i2c;   /*!< I2C operations */
        lan9646_miim_t miim; /*!< MIIM operations */
    } ops;                   /*!< Interface-specific operations */

    uint8_t i2c_addr; /*!< I2C device address (for I2C interface) */
    uint8_t phy_addr; /*!< PHY address (for MIIM interface) */
} lan9646_cfg_t;

/**
 * \brief           Device handle structure
 */
typedef struct {
    lan9646_cfg_t cfg; /*!< Device configuration */
    uint8_t is_init;   /*!< Initialization flag */
} lan9646_t;

/**
 * \name            Common register addresses
 * \{
 */

#define LAN9646_REG_CHIP_ID     0x0000 /*!< Chip ID register */
#define LAN9646_REG_GLOBAL_CTRL 0x0003 /*!< Global control register */
#define LAN9646_REG_PORT1_CTRL  0x1000 /*!< Port 1 control register */
#define LAN9646_REG_PORT2_CTRL  0x2000 /*!< Port 2 control register */
#define LAN9646_REG_PORT3_CTRL  0x3000 /*!< Port 3 control register */
#define LAN9646_REG_PORT4_CTRL  0x4000 /*!< Port 4 control register */
#define LAN9646_REG_PORT5_CTRL  0x5000 /*!< Port 5 control register */
#define LAN9646_REG_PORT6_CTRL  0x6000 /*!< Port 6 control register */

/**
 * \}
 */

/**
 * \name            SPI commands
 * \{
 */

#define LAN9646_SPI_CMD_READ       0x03 /*!< SPI read command */
#define LAN9646_SPI_CMD_WRITE      0x02 /*!< SPI write command */
#define LAN9646_SPI_CMD_FAST_READ  0x0B /*!< SPI fast read command */

/**
 * \}
 */

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

lan9646r_t lan9646_get_chip_id(lan9646_t* handle, uint16_t* chip_id);
lan9646r_t lan9646_soft_reset(lan9646_t* handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LAN9646_HDR_H */


