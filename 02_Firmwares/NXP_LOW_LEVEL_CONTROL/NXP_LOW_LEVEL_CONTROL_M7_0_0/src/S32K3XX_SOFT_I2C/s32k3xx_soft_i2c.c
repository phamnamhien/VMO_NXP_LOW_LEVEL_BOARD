/**
 * \file            s32k3xx_soft_i2c.c
 * \brief           Software I2C implementation for S32K3XX
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
 * This file is part of Soft I2C library.
 *
 * Author:          Pham Nam Hien
 * Version:         v1.0.0
 */
#include "s32k3xx_soft_i2c.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"

/* Timeout for clock stretching (in loop iterations) */
#define S32K3XX_SOFTI2C_TIMEOUT_CNT 10000U

/* CPU clock frequency in Hz - adjust according to your system */
#ifndef S32K3XX_SOFTI2C_CPU_FREQ_HZ
#define S32K3XX_SOFTI2C_CPU_FREQ_HZ 160000000UL /* Default: 160MHz */
#endif

/* Calculate delay cycles per microsecond */
/* Each NOP takes ~1 cycle, with loop overhead we use factor of 0.25 */
#define S32K3XX_SOFTI2C_CYCLES_PER_US ((S32K3XX_SOFTI2C_CPU_FREQ_HZ / 1000000UL) / 4UL)

/* Private function prototypes */
static void prv_delay_us(uint32_t us);
static void prv_scl_high(softi2c_t* handle);
static void prv_scl_low(softi2c_t* handle);
static void prv_sda_high(softi2c_t* handle);
static void prv_sda_low(softi2c_t* handle);
static uint8_t prv_sda_read(softi2c_t* handle);
static softi2cr_t prv_wait_scl_high(softi2c_t* handle);

/**
 * \brief           Delay in microseconds (blocking)
 * \note            Accuracy depends on CPU clock configuration
 *                  For better accuracy, consider using hardware timer
 * \param[in]       us: Delay time in microseconds
 */
static void
prv_delay_us(uint32_t us) {
    volatile uint32_t i;
    volatile uint32_t cycles;

    /* Calculate total cycles needed */
    cycles = us * S32K3XX_SOFTI2C_CYCLES_PER_US;

    /* Execute delay loop */
    for (i = 0; i < cycles; ++i) {
        __asm("NOP");
    }
}

/**
 * \brief           Set SCL pin high (release, open-drain)
 * \param[in]       handle: Pointer to I2C handle
 */
static void
prv_scl_high(softi2c_t* handle) {
	Siul2_Dio_Ip_WritePin(handle->pins.scl_port, handle->pins.scl_pin, 1U);
}

/**
 * \brief           Set SCL pin low
 * \param[in]       handle: Pointer to I2C handle
 */
static void
prv_scl_low(softi2c_t* handle) {
	Siul2_Dio_Ip_WritePin(handle->pins.scl_port, handle->pins.scl_pin, 0U);
}

/**
 * \brief           Set SDA pin high (release, open-drain)
 * \param[in]       handle: Pointer to I2C handle
 */
static void
prv_sda_high(softi2c_t* handle) {
	Siul2_Dio_Ip_WritePin(handle->pins.sda_port, handle->pins.sda_pin, 1U);
}

/**
 * \brief           Set SDA pin low
 * \param[in]       handle: Pointer to I2C handle
 */
static void
prv_sda_low(softi2c_t* handle) {
	Siul2_Dio_Ip_WritePin(handle->pins.sda_port, handle->pins.sda_pin, 0U);
}

/**
 * \brief           Read SDA pin state
 * \param[in]       handle: Pointer to I2C handle
 * \return          Pin state (0 or 1)
 */
static uint8_t
prv_sda_read(softi2c_t* handle) {
    return Siul2_Dio_Ip_ReadPin(handle->pins.sda_port, handle->pins.sda_pin);
}

/**
 * \brief           Wait for SCL to go high (clock stretching support)
 * \param[in]       handle: Pointer to I2C handle
 * \return          \ref softi2cOK on success, \ref softi2cTIMEOUT on timeout
 */
static softi2cr_t
prv_wait_scl_high(softi2c_t* handle) {
    uint32_t timeout;

    timeout = S32K3XX_SOFTI2C_TIMEOUT_CNT;
    while (Siul2_Dio_Ip_ReadPin(handle->pins.scl_port, handle->pins.scl_pin) == 0 && timeout > 0) {
        --timeout;
    }

    return (timeout > 0) ? softi2cOK : softi2cTIMEOUT;
}

/**
 * \brief           Initialize software I2C
 * \param[in]       handle: Pointer to I2C handle
 * \param[in]       pins: Pointer to pin configuration
 * \return          \ref softi2cOK on success, member of \ref softi2cr_t otherwise
 */
softi2cr_t
softi2c_init(softi2c_t* handle, const softi2c_pins_t* pins) {
    if (handle == NULL || pins == NULL) {
        return softi2cINVPARAM;
    }

    /* Copy pin configuration */
    handle->pins.scl_port = pins->scl_port;
    handle->pins.scl_pin = pins->scl_pin;
    handle->pins.sda_port = pins->sda_port;
    handle->pins.sda_pin = pins->sda_pin;
    handle->pins.delay_us = pins->delay_us;

    /* Initialize pins to idle state (both high) */
    prv_scl_high(handle);
    prv_sda_high(handle);

    prv_delay_us(handle->pins.delay_us);

    handle->is_init = 1;

    return softi2cOK;
}

/**
 * \brief           De-initialize software I2C
 * \param[in]       handle: Pointer to I2C handle
 * \return          \ref softi2cOK on success, member of \ref softi2cr_t otherwise
 */
softi2cr_t
softi2c_deinit(softi2c_t* handle) {
    if (handle == NULL || !handle->is_init) {
        return softi2cINVPARAM;
    }

    /* Release pins */
    prv_scl_high(handle);
    prv_sda_high(handle);

    handle->is_init = 0;

    return softi2cOK;
}

/**
 * \brief           Generate I2C START condition
 * \param[in]       handle: Pointer to I2C handle
 * \return          \ref softi2cOK on success, member of \ref softi2cr_t otherwise
 */
softi2cr_t
softi2c_start(softi2c_t* handle) {
    softi2cr_t res;

    if (handle == NULL || !handle->is_init) {
        return softi2cINVPARAM;
    }

    /* Ensure both lines are high */
    prv_sda_high(handle);
    prv_delay_us(handle->pins.delay_us);
    prv_scl_high(handle);
    prv_delay_us(handle->pins.delay_us);

    /* Wait for SCL to go high (clock stretching) */
    res = prv_wait_scl_high(handle);
    if (res != softi2cOK) {
        return res;
    }

    /* START condition: SDA falls while SCL is high */
    prv_sda_low(handle);
    prv_delay_us(handle->pins.delay_us);
    prv_scl_low(handle);
    prv_delay_us(handle->pins.delay_us);

    return softi2cOK;
}

/**
 * \brief           Generate I2C STOP condition
 * \param[in]       handle: Pointer to I2C handle
 * \return          \ref softi2cOK on success, member of \ref softi2cr_t otherwise
 */
softi2cr_t
softi2c_stop(softi2c_t* handle) {
    softi2cr_t res;

    if (handle == NULL || !handle->is_init) {
        return softi2cINVPARAM;
    }

    /* Ensure SDA is low */
    prv_sda_low(handle);
    prv_delay_us(handle->pins.delay_us);

    /* SCL high */
    prv_scl_high(handle);
    prv_delay_us(handle->pins.delay_us);

    /* Wait for SCL to go high */
    res = prv_wait_scl_high(handle);
    if (res != softi2cOK) {
        return res;
    }

    /* STOP condition: SDA rises while SCL is high */
    prv_sda_high(handle);
    prv_delay_us(handle->pins.delay_us);

    return softi2cOK;
}

/**
 * \brief           Write one byte to I2C bus
 * \param[in]       handle: Pointer to I2C handle
 * \param[in]       data: Byte to write
 * \return          \ref softi2cOK on ACK, \ref softi2cNACK on NACK, other on error
 */
softi2cr_t
softi2c_write_byte(softi2c_t* handle, uint8_t data) {
    softi2cr_t res;
    uint8_t i, ack;

    if (handle == NULL || !handle->is_init) {
        return softi2cINVPARAM;
    }

    /* Write 8 bits, MSB first */
    for (i = 0; i < 8; ++i) {
        /* Set SDA based on bit value */
        if ((data & 0x80) != 0) {
            prv_sda_high(handle);
        } else {
            prv_sda_low(handle);
        }
        data <<= 1;
        prv_delay_us(handle->pins.delay_us);

        /* Clock pulse */
        prv_scl_high(handle);
        prv_delay_us(handle->pins.delay_us);
        res = prv_wait_scl_high(handle);
        if (res != softi2cOK) {
            return res;
        }
        prv_scl_low(handle);
        prv_delay_us(handle->pins.delay_us);
    }

    /* Read ACK bit */
    prv_sda_high(handle); /* Release SDA */
    prv_delay_us(handle->pins.delay_us);
    prv_scl_high(handle);
    prv_delay_us(handle->pins.delay_us);

    res = prv_wait_scl_high(handle);
    if (res != softi2cOK) {
        return res;
    }

    ack = prv_sda_read(handle);
    prv_scl_low(handle);
    prv_delay_us(handle->pins.delay_us);

    return (ack == 0) ? softi2cOK : softi2cNACK;
}

/**
 * \brief           Read one byte from I2C bus
 * \param[in]       handle: Pointer to I2C handle
 * \param[out]      data: Pointer to store read byte
 * \param[in]       ack: Send ACK (1) or NACK (0) after read
 * \return          \ref softi2cOK on success, member of \ref softi2cr_t otherwise
 */
softi2cr_t
softi2c_read_byte(softi2c_t* handle, uint8_t* data, uint8_t ack) {
    softi2cr_t res;
    uint8_t i, byte;

    if (handle == NULL || data == NULL || !handle->is_init) {
        return softi2cINVPARAM;
    }

    byte = 0;
    prv_sda_high(handle); /* Release SDA for reading */

    /* Read 8 bits, MSB first */
    for (i = 0; i < 8; ++i) {
        prv_delay_us(handle->pins.delay_us);
        prv_scl_high(handle);
        prv_delay_us(handle->pins.delay_us);

        res = prv_wait_scl_high(handle);
        if (res != softi2cOK) {
            return res;
        }

        byte <<= 1;
        if (prv_sda_read(handle) != 0) {
            byte |= 0x01;
        }

        prv_scl_low(handle);
    }

    *data = byte;

    /* Send ACK or NACK */
    if (ack != 0) {
        prv_sda_low(handle); /* ACK */
    } else {
        prv_sda_high(handle); /* NACK */
    }
    prv_delay_us(handle->pins.delay_us);

    prv_scl_high(handle);
    prv_delay_us(handle->pins.delay_us);
    res = prv_wait_scl_high(handle);
    if (res != softi2cOK) {
        return res;
    }
    prv_scl_low(handle);
    prv_delay_us(handle->pins.delay_us);

    prv_sda_high(handle); /* Release SDA */

    return softi2cOK;
}

/**
 * \brief           Write data to I2C device
 * \param[in]       handle: Pointer to I2C handle
 * \param[in]       dev_addr: Device address (7-bit, will be shifted)
 * \param[in]       data: Pointer to data buffer
 * \param[in]       len: Number of bytes to write
 * \return          \ref softi2cOK on success, member of \ref softi2cr_t otherwise
 */
softi2cr_t
softi2c_write(softi2c_t* handle, uint8_t dev_addr, const uint8_t* data, uint16_t len) {
    softi2cr_t res;
    uint16_t i;

    if (handle == NULL || data == NULL || len == 0 || !handle->is_init) {
        return softi2cINVPARAM;
    }

    /* START condition */
    res = softi2c_start(handle);
    if (res != softi2cOK) {
        return res;
    }

    /* Send device address with write bit */
    res = softi2c_write_byte(handle, (dev_addr << 1) | 0x00);
    if (res != softi2cOK) {
        softi2c_stop(handle);
        return res;
    }

    /* Write data bytes */
    for (i = 0; i < len; ++i) {
        res = softi2c_write_byte(handle, data[i]);
        if (res != softi2cOK) {
            softi2c_stop(handle);
            return res;
        }
    }

    /* STOP condition */
    return softi2c_stop(handle);
}

/**
 * \brief           Read data from I2C device
 * \param[in]       handle: Pointer to I2C handle
 * \param[in]       dev_addr: Device address (7-bit, will be shifted)
 * \param[out]      data: Pointer to data buffer
 * \param[in]       len: Number of bytes to read
 * \return          \ref softi2cOK on success, member of \ref softi2cr_t otherwise
 */
softi2cr_t
softi2c_read(softi2c_t* handle, uint8_t dev_addr, uint8_t* data, uint16_t len) {
    softi2cr_t res;
    uint16_t i;

    if (handle == NULL || data == NULL || len == 0 || !handle->is_init) {
        return softi2cINVPARAM;
    }

    /* START condition */
    res = softi2c_start(handle);
    if (res != softi2cOK) {
        return res;
    }

    /* Send device address with read bit */
    res = softi2c_write_byte(handle, (dev_addr << 1) | 0x01);
    if (res != softi2cOK) {
        softi2c_stop(handle);
        return res;
    }

    /* Read data bytes */
    for (i = 0; i < len; ++i) {
        /* Send ACK for all bytes except the last one */
        res = softi2c_read_byte(handle, &data[i], (i < (len - 1)) ? 1 : 0);
        if (res != softi2cOK) {
            softi2c_stop(handle);
            return res;
        }
    }

    /* STOP condition */
    return softi2c_stop(handle);
}

/**
 * \brief           Write data to I2C device memory address
 * \param[in]       handle: Pointer to I2C handle
 * \param[in]       dev_addr: Device address (7-bit)
 * \param[in]       mem_addr: Memory address
 * \param[in]       mem_addr_size: Memory address size (1 or 2 bytes)
 * \param[in]       data: Pointer to data buffer
 * \param[in]       len: Number of bytes to write
 * \return          \ref softi2cOK on success, member of \ref softi2cr_t otherwise
 */
softi2cr_t
softi2c_mem_write(softi2c_t* handle, uint8_t dev_addr, uint16_t mem_addr, uint8_t mem_addr_size, const uint8_t* data,
                  uint16_t len) {
    softi2cr_t res;
    uint16_t i;

    if (handle == NULL || data == NULL || len == 0 || !handle->is_init) {
        return softi2cINVPARAM;
    }

    if (mem_addr_size != 1 && mem_addr_size != 2) {
        return softi2cINVPARAM;
    }

    /* START condition */
    res = softi2c_start(handle);
    if (res != softi2cOK) {
        return res;
    }

    /* Send device address with write bit */
    res = softi2c_write_byte(handle, (dev_addr << 1) | 0x00);
    if (res != softi2cOK) {
        softi2c_stop(handle);
        return res;
    }

    /* Send memory address (1 or 2 bytes) */
    if (mem_addr_size == 2) {
        res = softi2c_write_byte(handle, (uint8_t)(mem_addr >> 8));
        if (res != softi2cOK) {
            softi2c_stop(handle);
            return res;
        }
    }
    res = softi2c_write_byte(handle, (uint8_t)(mem_addr & 0xFF));
    if (res != softi2cOK) {
        softi2c_stop(handle);
        return res;
    }

    /* Write data bytes */
    for (i = 0; i < len; ++i) {
        res = softi2c_write_byte(handle, data[i]);
        if (res != softi2cOK) {
            softi2c_stop(handle);
            return res;
        }
    }

    /* STOP condition */
    return softi2c_stop(handle);
}

/**
 * \brief           Read data from I2C device memory address
 * \param[in]       handle: Pointer to I2C handle
 * \param[in]       dev_addr: Device address (7-bit)
 * \param[in]       mem_addr: Memory address
 * \param[in]       mem_addr_size: Memory address size (1 or 2 bytes)
 * \param[out]      data: Pointer to data buffer
 * \param[in]       len: Number of bytes to read
 * \return          \ref softi2cOK on success, member of \ref softi2cr_t otherwise
 */
softi2cr_t
softi2c_mem_read(softi2c_t* handle, uint8_t dev_addr, uint16_t mem_addr, uint8_t mem_addr_size, uint8_t* data,
                 uint16_t len) {
    softi2cr_t res;
    uint16_t i;

    if (handle == NULL || data == NULL || len == 0 || !handle->is_init) {
        return softi2cINVPARAM;
    }

    if (mem_addr_size != 1 && mem_addr_size != 2) {
        return softi2cINVPARAM;
    }

    /* START condition */
    res = softi2c_start(handle);
    if (res != softi2cOK) {
        return res;
    }

    /* Send device address with write bit */
    res = softi2c_write_byte(handle, (dev_addr << 1) | 0x00);
    if (res != softi2cOK) {
        softi2c_stop(handle);
        return res;
    }

    /* Send memory address (1 or 2 bytes) */
    if (mem_addr_size == 2) {
        res = softi2c_write_byte(handle, (uint8_t)(mem_addr >> 8));
        if (res != softi2cOK) {
            softi2c_stop(handle);
            return res;
        }
    }
    res = softi2c_write_byte(handle, (uint8_t)(mem_addr & 0xFF));
    if (res != softi2cOK) {
        softi2c_stop(handle);
        return res;
    }

    /* Repeated START condition */
    res = softi2c_start(handle);
    if (res != softi2cOK) {
        return res;
    }

    /* Send device address with read bit */
    res = softi2c_write_byte(handle, (dev_addr << 1) | 0x01);
    if (res != softi2cOK) {
        softi2c_stop(handle);
        return res;
    }

    /* Read data bytes */
    for (i = 0; i < len; ++i) {
        res = softi2c_read_byte(handle, &data[i], (i < (len - 1)) ? 1 : 0);
        if (res != softi2cOK) {
            softi2c_stop(handle);
            return res;
        }
    }

    /* STOP condition */
    return softi2c_stop(handle);
}

/**
 * \brief           Check if I2C device is ready
 * \param[in]       handle: Pointer to I2C handle
 * \param[in]       dev_addr: Device address (7-bit)
 * \param[in]       trials: Number of trials
 * \return          \ref softi2cOK if device responds, member of \ref softi2cr_t otherwise
 */
softi2cr_t
softi2c_is_device_ready(softi2c_t* handle, uint8_t dev_addr, uint8_t trials) {
    softi2cr_t res;
    uint8_t i;

    if (handle == NULL || !handle->is_init || trials == 0) {
        return softi2cINVPARAM;
    }

    for (i = 0; i < trials; ++i) {
        /* START condition */
        res = softi2c_start(handle);
        if (res != softi2cOK) {
            continue;
        }

        /* Send device address with write bit */
        res = softi2c_write_byte(handle, (dev_addr << 1) | 0x00);

        /* STOP condition */
        softi2c_stop(handle);

        if (res == softi2cOK) {
            return softi2cOK;
        }

        /* Small delay between trials */
        prv_delay_us(1000);
    }

    return softi2cNACK;
}

