/**
 * @file Eth_Callbacks.c
 * @brief GMAC interrupt callbacks
 */

#include "Gmac_Ip.h"


/**
 * @brief RX interrupt callback
 * @param instance GMAC instance (0 for GMAC0)
 * @param channel DMA channel
 */
void Eth_43_GMAC_RxIrqCallback(uint8 instance, uint8 channel)
{
    (void)instance;
    (void)channel;
    /* TODO: Implement RX packet handling */
    /* Example:
     * - Check which buffers have data
     * - Process received frames
     * - Release buffers back to GMAC
     */
}
/**
 * @brief TX interrupt callback
 * @param instance GMAC instance (0 for GMAC0)
 * @param channel DMA channel
 */
void Eth_43_GMAC_TxIrqCallback(uint8 instance, uint8 channel)
{
    (void)instance;
    (void)channel;
    /* TODO: Implement TX complete handling */
    /* Example:
     * - Check which buffers are free
     * - Update TX buffer status
     * - Signal application that TX is done
     */
}

//void Eth_43_GMAC_RxIrqCallback(uint8 instance, uint8 channel)
//{
//    Eth_RxIrqCallback(instance, channel);
//}
//
//void Eth_43_GMAC_TxIrqCallback(uint8 instance, uint8 channel)
//{
//    Eth_TxIrqCallback(instance, channel);
//}
