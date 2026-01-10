/**
 * \file            s32k3xx_soft_i2c.h
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
#ifndef S32K3XX_SOFT_I2C_HDR_H
#define S32K3XX_SOFT_I2C_HDR_H

#include <stdbool.h>
#include <stdint.h>
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Status return codes
 */
typedef enum {
    softi2cOK = 0,      /*!< Operation succeeded */
    softi2cERR,         /*!< General error */
    softi2cTIMEOUT,     /*!< Operation timeout */
    softi2cNACK,        /*!< NACK received */
    softi2cINVPARAM,    /*!< Invalid parameter */
    softi2cBUSBUSY,     /*!< Bus is busy */
} softi2cr_t;

/**
 * \brief           Pin configuration structure
 * \note            delay_us controls I2C speed:
 *                  - Standard mode (100kHz): delay_us = 5 (half period = 5us)
 *                  - Fast mode (400kHz): delay_us = 1.25 (half period = 1.25us)
 *                  - Fast mode plus (1MHz): delay_us = 0.5 (half period = 0.5us)
 *                  Formula: delay_us = 500000 / I2C_freq_Hz
 *                  Example: For 100kHz -> delay_us = 500000/100000 = 5
 */
typedef struct {
	Siul2_Dio_Ip_GpioType* scl_port; 	/*!< SCL port base (e.g., IP_SIUL2_0) */
    uint16_t scl_pin;                 	/*!< SCL pin number (MSCR index) */
    Siul2_Dio_Ip_GpioType* sda_port; 	/*!< SDA port base (e.g., IP_SIUL2_0) */
    uint16_t sda_pin;                 	/*!< SDA pin number (MSCR index) */
    uint32_t delay_us;                	/*!< Half clock period in microseconds (for speed control) */
} softi2c_pins_t;

/**
 * \brief           I2C handle structure
 */
typedef struct {
    softi2c_pins_t pins;    /*!< Pin configuration */
    uint8_t is_init;        /*!< Initialization flag */
} softi2c_t;

softi2cr_t softi2c_init(softi2c_t* handle, const softi2c_pins_t* pins);
softi2cr_t softi2c_deinit(softi2c_t* handle);

softi2cr_t softi2c_start(softi2c_t* handle);
softi2cr_t softi2c_stop(softi2c_t* handle);
softi2cr_t softi2c_write_byte(softi2c_t* handle, uint8_t data);
softi2cr_t softi2c_read_byte(softi2c_t* handle, uint8_t* data, uint8_t ack);

softi2cr_t softi2c_write(softi2c_t* handle, uint8_t dev_addr, const uint8_t* data, uint16_t len);
softi2cr_t softi2c_read(softi2c_t* handle, uint8_t dev_addr, uint8_t* data, uint16_t len);

softi2cr_t softi2c_mem_write(softi2c_t* handle, uint8_t dev_addr, uint16_t mem_addr, uint8_t mem_addr_size,
                             const uint8_t* data, uint16_t len);
softi2cr_t softi2c_mem_read(softi2c_t* handle, uint8_t dev_addr, uint16_t mem_addr, uint8_t mem_addr_size,
                            uint8_t* data, uint16_t len);

softi2cr_t softi2c_is_device_ready(softi2c_t* handle, uint8_t dev_addr, uint8_t trials);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* S32K3XX_SOFT_I2C_HDR_H */

