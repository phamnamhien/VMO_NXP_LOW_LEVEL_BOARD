/**
 * @file Eth_Callbacks.h
 * @brief GMAC interrupt callbacks declaration
 */

#ifndef ETH_CALLBACKS_H
#define ETH_CALLBACKS_H

#include "StandardTypes.h"

/**
 * @brief RX interrupt callback
 */
void Eth_43_GMAC_RxIrqCallback(uint8 instance, uint8 channel);

/**
 * @brief TX interrupt callback
 */
void Eth_43_GMAC_TxIrqCallback(uint8 instance, uint8 channel);

#endif /* ETH_CALLBACKS_H */
