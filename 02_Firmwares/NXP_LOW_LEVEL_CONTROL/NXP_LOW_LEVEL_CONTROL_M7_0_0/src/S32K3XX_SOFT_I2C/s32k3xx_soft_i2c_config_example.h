///**
// * \file            s32k3xx_soft_i2c_config_example.h
// * \brief           Configuration examples for S32K3XX Soft I2C
// */
//
///*
// * Copyright (c) 2026 Pham Nam Hien
// *
// * Permission is hereby granted, free of charge, to any person
// * obtaining a copy of this software and associated documentation
// * files (the "Software"), to deal in the Software without restriction,
// * including without limitation the rights to use, copy, modify, merge,
// * publish, distribute, sublicense, and/or sell copies of the Software,
// * and to permit persons to whom the Software is furnished to do so,
// * subject to the following conditions:
// *
// * The above copyright notice and this permission notice shall be
// * included in all copies or substantial portions of the Software.
// *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// * OTHER DEALINGS IN THE SOFTWARE.
// *
// * This file is part of S32K3XX Soft I2C library.
// *
// * Author:          Pham Nam Hien
// * Version:         v1.0.0
// */
//
//#ifndef S32K3XX_SOFT_I2C_CONFIG_EXAMPLE_HDR_H
//#define S32K3XX_SOFT_I2C_CONFIG_EXAMPLE_HDR_H
//
///*
// * =============================================================================
// * CPU CLOCK CONFIGURATION
// * =============================================================================
// * Define CPU clock frequency before including s32k3xx_soft_i2c.h
// * This affects delay accuracy
// */
//
///* Example 1: S32K388 running at 160MHz (default) */
///* #define S32K3XX_SOFTI2C_CPU_FREQ_HZ 160000000UL */
//
///* Example 2: S32K388 running at 200MHz */
///* #define S32K3XX_SOFTI2C_CPU_FREQ_HZ 200000000UL */
//
///* Example 3: S32K344 running at 120MHz */
///* #define S32K3XX_SOFTI2C_CPU_FREQ_HZ 120000000UL */
//
///*
// * =============================================================================
// * I2C SPEED CALCULATION
// * =============================================================================
// * Formula: delay_us = 500000 / I2C_freq_Hz
// *
// * Standard I2C speeds:
// * - Standard mode (100kHz):     delay_us = 5
// * - Fast mode (400kHz):         delay_us = 1 or 2 (adjust for your MCU)
// * - Fast mode plus (1MHz):      delay_us = 0 or 1 (may not be achievable)
// */
//
///* Standard mode: 100kHz */
//#define SOFTI2C_DELAY_STANDARD  5U
//
///* Fast mode: ~250kHz (practical speed with overhead) */
//#define SOFTI2C_DELAY_FAST      2U
//
///* Very fast: ~166kHz */
//#define SOFTI2C_DELAY_MEDIUM    3U
//
///*
// * =============================================================================
// * PIN CONFIGURATION EXAMPLES
// * =============================================================================
// */
//
///* Example 1: Using PTB4 (MSCR 20) for SCL, PTB5 (MSCR 21) for SDA */
///*
//softi2c_pins_t pins = {
//    .scl_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .scl_pin = 20,           // PTB4 = Port_1 * 16 + 4 = 20
//    .sda_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .sda_pin = 21,           // PTB5 = Port_1 * 16 + 5 = 21
//    .delay_us = SOFTI2C_DELAY_STANDARD
//};
//*/
//
///* Example 2: Using PTC7 (MSCR 39) for SCL, PTC8 (MSCR 40) for SDA */
///*
//softi2c_pins_t pins = {
//    .scl_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .scl_pin = 39,           // PTC7 = Port_2 * 16 + 7 = 39
//    .sda_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .sda_pin = 40,           // PTC8 = Port_2 * 16 + 8 = 40
//    .delay_us = SOFTI2c_DELAY_FAST
//};
//*/
//
///*
// * =============================================================================
// * MSCR INDEX CALCULATION
// * =============================================================================
// * Formula: MSCR_index = Port_Number * 16 + Pin_Number
// *
// * Port mapping:
// * - PTA = Port 0  -> PTA0 = 0*16+0 = 0,   PTA15 = 0*16+15 = 15
// * - PTB = Port 1  -> PTB0 = 1*16+0 = 16,  PTB15 = 1*16+15 = 31
// * - PTC = Port 2  -> PTC0 = 2*16+0 = 32,  PTC15 = 2*16+15 = 47
// * - PTD = Port 3  -> PTD0 = 3*16+0 = 48,  PTD15 = 3*16+15 = 63
// * - PTE = Port 4  -> PTE0 = 4*16+0 = 64,  PTE15 = 4*16+15 = 79
// * - PTF = Port 5  -> PTF0 = 5*16+0 = 80,  PTF15 = 5*16+15 = 95
// *
// * Example calculations:
// * - PTB4  = 1*16 + 4  = 20
// * - PTC12 = 2*16 + 12 = 44
// * - PTD9  = 3*16 + 9  = 57
// */
//
///*
// * =============================================================================
// * USAGE WITH DIFFERENT I2C DEVICES
// * =============================================================================
// */
//
///* Example: EEPROM AT24C256 (typically 400kHz max) */
///*
//softi2c_pins_t eeprom_i2c = {
//    .scl_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .scl_pin = 20,
//    .sda_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .sda_pin = 21,
//    .delay_us = SOFTI2C_DELAY_FAST
//};
//*/
//
///* Example: Temperature sensor (typically 100kHz) */
///*
//softi2c_pins_t sensor_i2c = {
//    .scl_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .scl_pin = 39,
//    .sda_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .sda_pin = 40,
//    .delay_us = SOFTI2C_DELAY_STANDARD
//};
//*/
//
///* Example: LAN9646 Ethernet switch (I2C address 0x5F) */
///*
//softi2c_pins_t lan9646_i2c = {
//    .scl_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .scl_pin = 44,           // PTC12
//    .sda_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .sda_pin = 45,           // PTC13
//    .delay_us = SOFTI2C_DELAY_FAST
//};
//*/
//
///* Example: LAN9646 using GMAC MDC/MDIO pins (PTD16/PTD17) */
///*
//softi2c_pins_t lan9646_i2c = {
//    .scl_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .scl_pin = 113,          // PTD17 - GMAC0_ETH_MDC (MSCR 113)
//    .sda_base = (Siul2_Port_Ip_PortType*)IP_SIUL2,
//    .sda_pin = 112,          // PTD16 - GMAC0_ETH_MDIO (MSCR 112)
//    .delay_us = SOFTI2C_DELAY_STANDARD
//};
//*/
//
///*
// * =============================================================================
// * TIMING CONSIDERATIONS
// * =============================================================================
// *
// * 1. Clock stretching:
// *    - Automatically handled by prv_wait_scl_high()
// *    - Timeout: S32K3XX_SOFTI2C_TIMEOUT_CNT iterations
// *
// * 2. Delay accuracy:
// *    - Depends on CPU clock configuration
// *    - Loop overhead reduces actual delay
// *    - For critical timing, use hardware timer instead
// *
// * 3. Maximum achievable speed:
// *    - Limited by:
// *      a) Function call overhead
// *      b) GPIO switching speed
// *      c) External pull-up resistor values
// *    - Practical max: ~250-400kHz on S32K3xx
// *
// * 4. Pull-up resistors:
// *    - Recommended: 2.2kΩ - 10kΩ
// *    - Lower values = faster rise time, higher speed
// *    - Higher values = lower power, slower speed
// *    - For 400kHz: use 2.2kΩ - 4.7kΩ
// *    - For 100kHz: use 4.7kΩ - 10kΩ
// */
//
///*
// * =============================================================================
// * COMPILE-TIME CONFIGURATION
// * =============================================================================
// * Add to your project's preprocessor defines or before including header:
// */
//
///* Set CPU frequency (choose one based on your clock config) */
///* #define S32K3XX_SOFTI2C_CPU_FREQ_HZ 160000000UL */  /* 160 MHz */
///* #define S32K3XX_SOFTI2C_CPU_FREQ_HZ 200000000UL */  /* 200 MHz */
///* #define S32K3XX_SOFTI2C_CPU_FREQ_HZ 120000000UL */  /* 120 MHz */
//
//#endif /* S32K3XX_SOFT_I2C_CONFIG_EXAMPLE_HDR_H */
//
