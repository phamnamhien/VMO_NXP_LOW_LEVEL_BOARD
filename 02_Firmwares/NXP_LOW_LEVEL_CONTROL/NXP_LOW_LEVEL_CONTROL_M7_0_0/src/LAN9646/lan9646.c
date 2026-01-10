/**
 * \file            lan9646.c
 * \brief           LAN9646 Ethernet Switch Driver Implementation
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
#include "lan9646.h"
#include <string.h>

/* Private function prototypes */
static lan9646r_t prv_spi_read_reg(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len);
static lan9646r_t prv_spi_write_reg(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len);
static lan9646r_t prv_i2c_read_reg(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len);
static lan9646r_t prv_i2c_write_reg(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len);
static lan9646r_t prv_miim_read_reg(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len);
static lan9646r_t prv_miim_write_reg(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len);

/**
 * \brief           Initialize LAN9646 device
 * \param[in]       handle: Pointer to device handle
 * \param[in]       cfg: Pointer to configuration structure
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_init(lan9646_t* handle, const lan9646_cfg_t* cfg) {
    lan9646r_t res;

    if (handle == NULL || cfg == NULL) {
        return lan9646INVPARAM;
    }

    /* Copy configuration */
    memcpy(&handle->cfg, cfg, sizeof(lan9646_cfg_t));

    /* Initialize selected interface */
    switch (cfg->if_type) {
        case LAN9646_IF_SPI:
            if (cfg->ops.spi.init_fn == NULL) {
                return lan9646INVPARAM;
            }
            res = cfg->ops.spi.init_fn();
            break;

        case LAN9646_IF_I2C:
            if (cfg->ops.i2c.init_fn == NULL) {
                return lan9646INVPARAM;
            }
            res = cfg->ops.i2c.init_fn();
            break;

        case LAN9646_IF_MIIM:
            if (cfg->ops.miim.init_fn == NULL) {
                return lan9646INVPARAM;
            }
            res = cfg->ops.miim.init_fn();
            break;

        default: return lan9646INVPARAM;
    }

    if (res == lan9646OK) {
        handle->is_init = 1;
    }

    return res;
}

/**
 * \brief           De-initialize LAN9646 device
 * \param[in]       handle: Pointer to device handle
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_deinit(lan9646_t* handle) {
    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    handle->is_init = 0;
    return lan9646OK;
}

/**
 * \brief           SPI read register implementation
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address (16-bit)
 * \param[out]      data: Buffer to store read data
 * \param[in]       len: Number of bytes to read
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
static lan9646r_t
prv_spi_read_reg(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len) {
    lan9646_spi_t* spi;
    lan9646r_t res;
    uint8_t cmd_buf[4];

    spi = &handle->cfg.ops.spi;

    /* Check required callbacks */
    if (spi->cs_low_fn == NULL || spi->cs_high_fn == NULL || spi->transfer_fn == NULL) {
        return lan9646INVPARAM;
    }

    /* Prepare command: CMD(1) + ADDR_H(1) + ADDR_L(1) + DUMMY(1) */
    cmd_buf[0] = LAN9646_SPI_CMD_READ;
    cmd_buf[1] = (uint8_t)(reg_addr >> 8);
    cmd_buf[2] = (uint8_t)(reg_addr & 0xFF);
    cmd_buf[3] = 0x00; /* Turnaround/dummy byte */

    /* Assert CS */
    spi->cs_low_fn();

    /* Send command and address */
    res = spi->transfer_fn(cmd_buf, NULL, 4);
    if (res != lan9646OK) {
        spi->cs_high_fn();
        return res;
    }

    /* Read data */
    res = spi->transfer_fn(NULL, data, len);

    /* De-assert CS */
    spi->cs_high_fn();

    return res;
}

/**
 * \brief           SPI write register implementation
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address (16-bit)
 * \param[in]       data: Data to write
 * \param[in]       len: Number of bytes to write
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
static lan9646r_t
prv_spi_write_reg(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len) {
    lan9646_spi_t* spi;
    lan9646r_t res;
    uint8_t cmd_buf[3];

    spi = &handle->cfg.ops.spi;

    /* Check required callbacks */
    if (spi->cs_low_fn == NULL || spi->cs_high_fn == NULL || spi->write_fn == NULL) {
        return lan9646INVPARAM;
    }

    /* Prepare command: CMD(1) + ADDR_H(1) + ADDR_L(1) */
    cmd_buf[0] = LAN9646_SPI_CMD_WRITE;
    cmd_buf[1] = (uint8_t)(reg_addr >> 8);
    cmd_buf[2] = (uint8_t)(reg_addr & 0xFF);

    /* Assert CS */
    spi->cs_low_fn();

    /* Send command and address */
    res = spi->write_fn(cmd_buf, 3);
    if (res != lan9646OK) {
        spi->cs_high_fn();
        return res;
    }

    /* Write data */
    res = spi->write_fn(data, len);

    /* De-assert CS */
    spi->cs_high_fn();

    return res;
}

/**
 * \brief           I2C read register implementation
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address (16-bit)
 * \param[out]      data: Buffer to store read data
 * \param[in]       len: Number of bytes to read
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
static lan9646r_t
prv_i2c_read_reg(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len) {
    lan9646_i2c_t* i2c;
    lan9646r_t res;
    uint8_t addr_buf[2];

    i2c = &handle->cfg.ops.i2c;

    /* Use memory read if available */
    if (i2c->mem_read_fn != NULL) {
        return i2c->mem_read_fn(handle->cfg.i2c_addr, reg_addr, data, len);
    } else if (i2c->write_fn != NULL && i2c->read_fn != NULL) {
        /* Manual memory read: write address then read data */
        addr_buf[0] = (uint8_t)(reg_addr >> 8);
        addr_buf[1] = (uint8_t)(reg_addr & 0xFF);

        res = i2c->write_fn(handle->cfg.i2c_addr, addr_buf, 2);
        if (res != lan9646OK) {
            return res;
        }

        return i2c->read_fn(handle->cfg.i2c_addr, data, len);
    }

    return lan9646INVPARAM;
}

/**
 * \brief           I2C write register implementation
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address (16-bit)
 * \param[in]       data: Data to write
 * \param[in]       len: Number of bytes to write
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
static lan9646r_t
prv_i2c_write_reg(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len) {
    lan9646_i2c_t* i2c;
    uint8_t tx_buf[258]; /* 2 bytes addr + max 256 bytes data */

    i2c = &handle->cfg.ops.i2c;

    /* Use memory write if available */
    if (i2c->mem_write_fn != NULL) {
        return i2c->mem_write_fn(handle->cfg.i2c_addr, reg_addr, data, len);
    } else if (i2c->write_fn != NULL) {
        /* Manual memory write: address + data in single transaction */
        if (len > 256) {
            return lan9646INVPARAM;
        }

        tx_buf[0] = (uint8_t)(reg_addr >> 8);
        tx_buf[1] = (uint8_t)(reg_addr & 0xFF);
        memcpy(&tx_buf[2], data, len);

        return i2c->write_fn(handle->cfg.i2c_addr, tx_buf, len + 2);
    }

    return lan9646INVPARAM;
}

/**
 * \brief           MIIM read register implementation
 * \note            MIIM only supports 16-bit PHY register access
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address (5-bit for MIIM)
 * \param[out]      data: Buffer to store read data (must be 2 bytes)
 * \param[in]       len: Number of bytes to read (must be 2)
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
static lan9646r_t
prv_miim_read_reg(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len) {
    lan9646_miim_t* miim;
    lan9646r_t res;
    uint16_t value;

    miim = &handle->cfg.ops.miim;

    if (miim->read_fn == NULL) {
        return lan9646INVPARAM;
    }

    /* MIIM only supports 16-bit access */
    if (len != 2) {
        return lan9646INVPARAM;
    }

    res = miim->read_fn(handle->cfg.phy_addr, (uint8_t)reg_addr, &value);

    if (res == lan9646OK) {
        data[0] = (uint8_t)(value >> 8);
        data[1] = (uint8_t)(value & 0xFF);
    }

    return res;
}

/**
 * \brief           MIIM write register implementation
 * \note            MIIM only supports 16-bit PHY register access
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address (5-bit for MIIM)
 * \param[in]       data: Data to write (must be 2 bytes)
 * \param[in]       len: Number of bytes to write (must be 2)
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
static lan9646r_t
prv_miim_write_reg(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len) {
    lan9646_miim_t* miim;
    uint16_t value;

    miim = &handle->cfg.ops.miim;

    if (miim->write_fn == NULL) {
        return lan9646INVPARAM;
    }

    /* MIIM only supports 16-bit access */
    if (len != 2) {
        return lan9646INVPARAM;
    }

    value = ((uint16_t)data[0] << 8) | data[1];

    return miim->write_fn(handle->cfg.phy_addr, (uint8_t)reg_addr, value);
}

/**
 * \brief           Read 8-bit register
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address
 * \param[out]      data: Pointer to store read value
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_read_reg8(lan9646_t* handle, uint16_t reg_addr, uint8_t* data) {
    if (handle == NULL || data == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: return prv_spi_read_reg(handle, reg_addr, data, 1);
        case LAN9646_IF_I2C: return prv_i2c_read_reg(handle, reg_addr, data, 1);
        case LAN9646_IF_MIIM: return lan9646INVPARAM; /* MIIM does not support 8-bit */
        default: return lan9646ERR;
    }
}

/**
 * \brief           Write 8-bit register
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address
 * \param[in]       data: Value to write
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_write_reg8(lan9646_t* handle, uint16_t reg_addr, uint8_t data) {
    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: return prv_spi_write_reg(handle, reg_addr, &data, 1);
        case LAN9646_IF_I2C: return prv_i2c_write_reg(handle, reg_addr, &data, 1);
        case LAN9646_IF_MIIM: return lan9646INVPARAM; /* MIIM does not support 8-bit */
        default: return lan9646ERR;
    }
}

/**
 * \brief           Read 16-bit register
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address
 * \param[out]      data: Pointer to store read value
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_read_reg16(lan9646_t* handle, uint16_t reg_addr, uint16_t* data) {
    lan9646r_t res;
    uint8_t buf[2];

    if (handle == NULL || data == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: res = prv_spi_read_reg(handle, reg_addr, buf, 2); break;
        case LAN9646_IF_I2C: res = prv_i2c_read_reg(handle, reg_addr, buf, 2); break;
        case LAN9646_IF_MIIM: res = prv_miim_read_reg(handle, reg_addr, buf, 2); break;
        default: return lan9646ERR;
    }

    if (res == lan9646OK) {
        *data = ((uint16_t)buf[0] << 8) | buf[1];
    }

    return res;
}

/**
 * \brief           Write 16-bit register
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address
 * \param[in]       data: Value to write
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_write_reg16(lan9646_t* handle, uint16_t reg_addr, uint16_t data) {
    uint8_t buf[2];

    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    buf[0] = (uint8_t)(data >> 8);
    buf[1] = (uint8_t)(data & 0xFF);

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: return prv_spi_write_reg(handle, reg_addr, buf, 2);
        case LAN9646_IF_I2C: return prv_i2c_write_reg(handle, reg_addr, buf, 2);
        case LAN9646_IF_MIIM: return prv_miim_write_reg(handle, reg_addr, buf, 2);
        default: return lan9646ERR;
    }
}

/**
 * \brief           Read 32-bit register
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address
 * \param[out]      data: Pointer to store read value
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_read_reg32(lan9646_t* handle, uint16_t reg_addr, uint32_t* data) {
    lan9646r_t res;
    uint8_t buf[4];

    if (handle == NULL || data == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: res = prv_spi_read_reg(handle, reg_addr, buf, 4); break;
        case LAN9646_IF_I2C: res = prv_i2c_read_reg(handle, reg_addr, buf, 4); break;
        case LAN9646_IF_MIIM: return lan9646INVPARAM; /* MIIM does not support 32-bit */
        default: return lan9646ERR;
    }

    if (res == lan9646OK) {
        *data = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | buf[3];
    }

    return res;
}

/**
 * \brief           Write 32-bit register
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address
 * \param[in]       data: Value to write
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_write_reg32(lan9646_t* handle, uint16_t reg_addr, uint32_t data) {
    uint8_t buf[4];

    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    buf[0] = (uint8_t)(data >> 24);
    buf[1] = (uint8_t)(data >> 16);
    buf[2] = (uint8_t)(data >> 8);
    buf[3] = (uint8_t)(data & 0xFF);

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: return prv_spi_write_reg(handle, reg_addr, buf, 4);
        case LAN9646_IF_I2C: return prv_i2c_write_reg(handle, reg_addr, buf, 4);
        case LAN9646_IF_MIIM: return lan9646INVPARAM; /* MIIM does not support 32-bit */
        default: return lan9646ERR;
    }
}

/**
 * \brief           Read multiple consecutive registers (burst read)
 * \note            Not supported for MIIM interface
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Starting register address
 * \param[out]      data: Buffer to store read data
 * \param[in]       len: Number of bytes to read
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_read_burst(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len) {
    if (handle == NULL || data == NULL || !handle->is_init || len == 0) {
        return lan9646INVPARAM;
    }

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: return prv_spi_read_reg(handle, reg_addr, data, len);
        case LAN9646_IF_I2C: return prv_i2c_read_reg(handle, reg_addr, data, len);
        case LAN9646_IF_MIIM: return lan9646INVPARAM; /* MIIM does not support burst */
        default: return lan9646ERR;
    }
}

/**
 * \brief           Write multiple consecutive registers (burst write)
 * \note            Not supported for MIIM interface
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Starting register address
 * \param[in]       data: Data to write
 * \param[in]       len: Number of bytes to write
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_write_burst(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len) {
    if (handle == NULL || data == NULL || !handle->is_init || len == 0) {
        return lan9646INVPARAM;
    }

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: return prv_spi_write_reg(handle, reg_addr, data, len);
        case LAN9646_IF_I2C: return prv_i2c_write_reg(handle, reg_addr, data, len);
        case LAN9646_IF_MIIM: return lan9646INVPARAM; /* MIIM does not support burst */
        default: return lan9646ERR;
    }
}

/**
 * \brief           Modify 8-bit register (read-modify-write)
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address
 * \param[in]       mask: Bit mask for bits to modify
 * \param[in]       value: New value for masked bits
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_modify_reg8(lan9646_t* handle, uint16_t reg_addr, uint8_t mask, uint8_t value) {
    lan9646r_t res;
    uint8_t reg_val;

    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    /* Read current value */
    res = lan9646_read_reg8(handle, reg_addr, &reg_val);
    if (res != lan9646OK) {
        return res;
    }

    /* Modify bits */
    reg_val &= ~mask;
    reg_val |= (value & mask);

    /* Write back */
    return lan9646_write_reg8(handle, reg_addr, reg_val);
}

/**
 * \brief           Modify 16-bit register (read-modify-write)
 * \param[in]       handle: Pointer to device handle
 * \param[in]       reg_addr: Register address
 * \param[in]       mask: Bit mask for bits to modify
 * \param[in]       value: New value for masked bits
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_modify_reg16(lan9646_t* handle, uint16_t reg_addr, uint16_t mask, uint16_t value) {
    lan9646r_t res;
    uint16_t reg_val;

    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    /* Read current value */
    res = lan9646_read_reg16(handle, reg_addr, &reg_val);
    if (res != lan9646OK) {
        return res;
    }

    /* Modify bits */
    reg_val &= ~mask;
    reg_val |= (value & mask);

    /* Write back */
    return lan9646_write_reg16(handle, reg_addr, reg_val);
}

/**
 * \brief           Get chip ID
 * \param[in]       handle: Pointer to device handle
 * \param[out]      chip_id: Pointer to store chip ID value
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_get_chip_id(lan9646_t* handle, uint16_t* chip_id) {
    if (handle == NULL || chip_id == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    return lan9646_read_reg16(handle, LAN9646_REG_CHIP_ID, chip_id);
}

/**
 * \brief           Perform software reset
 * \param[in]       handle: Pointer to device handle
 * \return          \ref lan9646OK on success, member of \ref lan9646r_t otherwise
 */
lan9646r_t
lan9646_soft_reset(lan9646_t* handle) {
    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    /* Set soft reset bit in global control register */
    return lan9646_modify_reg8(handle, LAN9646_REG_GLOBAL_CTRL, 0x01, 0x01);
}


