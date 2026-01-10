/**
 * \file            lan9646_soft_i2c_init_example.h
 * \brief           LAN9646 initialization with Soft I2C - Header file
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
 * This file is part of LAN9646 + Soft I2C example.
 *
 * Author:          Pham Nam Hien
 * Version:         v1.0.0
 */

#ifndef LAN9646_SOFT_I2C_INIT_EXAMPLE_HDR_H
#define LAN9646_SOFT_I2C_INIT_EXAMPLE_HDR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Complete LAN9646 initialization with Soft I2C
 * \note            This function performs full initialization:
 *                  - Initialize Soft I2C on MDC/MDIO pins
 *                  - Verify LAN9646 presence
 *                  - Read chip information
 *                  - Configure Port 6 (CPU port)
 *                  - Configure Ports 1-4 (Switch ports)
 *                  - Enable forwarding between ports
 *                  - Read and log all port status
 */
void lan9646_complete_init_example(void);

/**
 * \brief           Periodic status check for LAN9646
 * \note            Call this function periodically from main loop
 *                  to monitor link status and detect errors
 */
void lan9646_periodic_status_check(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LAN9646_SOFT_I2C_INIT_EXAMPLE_HDR_H */

