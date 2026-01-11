/**
 * \file            lan9646.c
 * \brief           LAN9646 Ethernet Switch Driver Implementation
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
 */
lan9646r_t
lan9646_init(lan9646_t* handle, const lan9646_cfg_t* cfg) {
    lan9646r_t res;

    if (handle == NULL || cfg == NULL) {
        return lan9646INVPARAM;
    }

    memcpy(&handle->cfg, cfg, sizeof(lan9646_cfg_t));

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
 */
static lan9646r_t
prv_spi_read_reg(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len) {
    lan9646_spi_t* spi;
    lan9646r_t res;
    uint8_t cmd_buf[4];

    spi = &handle->cfg.ops.spi;

    if (spi->cs_low_fn == NULL || spi->cs_high_fn == NULL || spi->transfer_fn == NULL) {
        return lan9646INVPARAM;
    }

    cmd_buf[0] = LAN9646_SPI_CMD_READ;
    cmd_buf[1] = (uint8_t)(reg_addr >> 8);
    cmd_buf[2] = (uint8_t)(reg_addr & 0xFF);
    cmd_buf[3] = 0x00;

    spi->cs_low_fn();

    res = spi->transfer_fn(cmd_buf, NULL, 4);
    if (res != lan9646OK) {
        spi->cs_high_fn();
        return res;
    }

    res = spi->transfer_fn(NULL, data, len);
    spi->cs_high_fn();

    return res;
}

/**
 * \brief           SPI write register implementation
 */
static lan9646r_t
prv_spi_write_reg(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len) {
    lan9646_spi_t* spi;
    lan9646r_t res;
    uint8_t cmd_buf[3];

    spi = &handle->cfg.ops.spi;

    if (spi->cs_low_fn == NULL || spi->cs_high_fn == NULL || spi->write_fn == NULL) {
        return lan9646INVPARAM;
    }

    cmd_buf[0] = LAN9646_SPI_CMD_WRITE;
    cmd_buf[1] = (uint8_t)(reg_addr >> 8);
    cmd_buf[2] = (uint8_t)(reg_addr & 0xFF);

    spi->cs_low_fn();

    res = spi->write_fn(cmd_buf, 3);
    if (res != lan9646OK) {
        spi->cs_high_fn();
        return res;
    }

    res = spi->write_fn(data, len);
    spi->cs_high_fn();

    return res;
}

/**
 * \brief           I2C read register implementation
 */
static lan9646r_t
prv_i2c_read_reg(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len) {
    lan9646_i2c_t* i2c;
    lan9646r_t res;
    uint8_t addr_buf[2];

    i2c = &handle->cfg.ops.i2c;

    if (i2c->mem_read_fn != NULL) {
        return i2c->mem_read_fn(handle->cfg.i2c_addr, reg_addr, data, len);
    } else if (i2c->write_fn != NULL && i2c->read_fn != NULL) {
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
 */
static lan9646r_t
prv_i2c_write_reg(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len) {
    lan9646_i2c_t* i2c;

    i2c = &handle->cfg.ops.i2c;

    if (i2c->mem_write_fn != NULL) {
        return i2c->mem_write_fn(handle->cfg.i2c_addr, reg_addr, data, len);
    }

    return lan9646INVPARAM;
}

/**
 * \brief           MIIM read register implementation
 */
static lan9646r_t
prv_miim_read_reg(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len) {
    lan9646_miim_t* miim;
    lan9646r_t res;
    uint16_t val;

    if (len != 2) {
        return lan9646INVPARAM;
    }

    miim = &handle->cfg.ops.miim;

    if (miim->read_fn == NULL) {
        return lan9646INVPARAM;
    }

    res = miim->read_fn(handle->cfg.phy_addr, (uint8_t)(reg_addr & 0x1F), &val);
    if (res == lan9646OK) {
        data[0] = (uint8_t)(val >> 8);
        data[1] = (uint8_t)(val & 0xFF);
    }

    return res;
}

/**
 * \brief           MIIM write register implementation
 */
static lan9646r_t
prv_miim_write_reg(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len) {
    lan9646_miim_t* miim;
    uint16_t val;

    if (len != 2) {
        return lan9646INVPARAM;
    }

    miim = &handle->cfg.ops.miim;

    if (miim->write_fn == NULL) {
        return lan9646INVPARAM;
    }

    val = ((uint16_t)data[0] << 8) | data[1];
    return miim->write_fn(handle->cfg.phy_addr, (uint8_t)(reg_addr & 0x1F), val);
}

/**
 * \brief           Read 8-bit register
 */
lan9646r_t
lan9646_read_reg8(lan9646_t* handle, uint16_t reg_addr, uint8_t* data) {
    if (handle == NULL || data == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: return prv_spi_read_reg(handle, reg_addr, data, 1);
        case LAN9646_IF_I2C: return prv_i2c_read_reg(handle, reg_addr, data, 1);
        case LAN9646_IF_MIIM: return lan9646INVPARAM;
        default: return lan9646ERR;
    }
}

/**
 * \brief           Write 8-bit register
 */
lan9646r_t
lan9646_write_reg8(lan9646_t* handle, uint16_t reg_addr, uint8_t data) {
    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: return prv_spi_write_reg(handle, reg_addr, &data, 1);
        case LAN9646_IF_I2C: return prv_i2c_write_reg(handle, reg_addr, &data, 1);
        case LAN9646_IF_MIIM: return lan9646INVPARAM;
        default: return lan9646ERR;
    }
}

/**
 * \brief           Read 16-bit register
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
        case LAN9646_IF_MIIM: return lan9646INVPARAM;
        default: return lan9646ERR;
    }

    if (res == lan9646OK) {
        *data = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
                ((uint32_t)buf[2] << 8) | buf[3];
    }

    return res;
}

/**
 * \brief           Write 32-bit register
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
        case LAN9646_IF_MIIM: return lan9646INVPARAM;
        default: return lan9646ERR;
    }
}

/**
 * \brief           Burst read
 */
lan9646r_t
lan9646_read_burst(lan9646_t* handle, uint16_t reg_addr, uint8_t* data, uint16_t len) {
    if (handle == NULL || data == NULL || !handle->is_init || len == 0) {
        return lan9646INVPARAM;
    }

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: return prv_spi_read_reg(handle, reg_addr, data, len);
        case LAN9646_IF_I2C: return prv_i2c_read_reg(handle, reg_addr, data, len);
        case LAN9646_IF_MIIM: return lan9646INVPARAM;
        default: return lan9646ERR;
    }
}

/**
 * \brief           Burst write
 */
lan9646r_t
lan9646_write_burst(lan9646_t* handle, uint16_t reg_addr, const uint8_t* data, uint16_t len) {
    if (handle == NULL || data == NULL || !handle->is_init || len == 0) {
        return lan9646INVPARAM;
    }

    switch (handle->cfg.if_type) {
        case LAN9646_IF_SPI: return prv_spi_write_reg(handle, reg_addr, data, len);
        case LAN9646_IF_I2C: return prv_i2c_write_reg(handle, reg_addr, data, len);
        case LAN9646_IF_MIIM: return lan9646INVPARAM;
        default: return lan9646ERR;
    }
}

/**
 * \brief           Modify 8-bit register (read-modify-write)
 */
lan9646r_t
lan9646_modify_reg8(lan9646_t* handle, uint16_t reg_addr, uint8_t mask, uint8_t value) {
    lan9646r_t res;
    uint8_t reg_val;

    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    res = lan9646_read_reg8(handle, reg_addr, &reg_val);
    if (res != lan9646OK) {
        return res;
    }

    reg_val &= ~mask;
    reg_val |= (value & mask);

    return lan9646_write_reg8(handle, reg_addr, reg_val);
}

/**
 * \brief           Modify 16-bit register (read-modify-write)
 */
lan9646r_t
lan9646_modify_reg16(lan9646_t* handle, uint16_t reg_addr, uint16_t mask, uint16_t value) {
    lan9646r_t res;
    uint16_t reg_val;

    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    res = lan9646_read_reg16(handle, reg_addr, &reg_val);
    if (res != lan9646OK) {
        return res;
    }

    reg_val &= ~mask;
    reg_val |= (value & mask);

    return lan9646_write_reg16(handle, reg_addr, reg_val);
}

/**
 * \brief           Get chip ID and revision
 * \param[in]       handle: Pointer to device handle
 * \param[out]      chip_id: Pointer to store chip ID (0x9477)
 * \param[out]      revision: Pointer to store revision (can be NULL)
 * \return          \ref lan9646OK on success
 */
lan9646r_t
lan9646_get_chip_id(lan9646_t* handle, uint16_t* chip_id, uint8_t* revision) {
    uint8_t id1, id2, id3;
    lan9646r_t res;

    if (handle == NULL || chip_id == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    /* Read Chip ID MSB from 0x0001 */
    res = lan9646_read_reg8(handle, LAN9646_REG_CHIP_ID1, &id1);
    if (res != lan9646OK) return res;

    /* Read Chip ID LSB from 0x0002 */
    res = lan9646_read_reg8(handle, LAN9646_REG_CHIP_ID2, &id2);
    if (res != lan9646OK) return res;

    /* Full 16-bit chip ID = MSB:LSB = 0x9477 */
    *chip_id = ((uint16_t)id1 << 8) | id2;

    /* Read revision if requested */
    if (revision != NULL) {
        res = lan9646_read_reg8(handle, LAN9646_REG_CHIP_ID3, &id3);
        if (res == lan9646OK) {
            *revision = (id3 >> 4) & 0x0F;
        }
    }

    return lan9646OK;
}

/**
 * \brief           Perform software reset
 * \param[in]       handle: Pointer to device handle
 * \return          \ref lan9646OK on success
 */
lan9646r_t
lan9646_soft_reset(lan9646_t* handle) {
    if (handle == NULL || !handle->is_init) {
        return lan9646INVPARAM;
    }

    /* Set soft reset bit (bit 0) in register 0x0003 */
    return lan9646_modify_reg8(handle, LAN9646_REG_GLOBAL_CTRL,
                               LAN9646_GLOBAL_SW_RESET, LAN9646_GLOBAL_SW_RESET);
}

