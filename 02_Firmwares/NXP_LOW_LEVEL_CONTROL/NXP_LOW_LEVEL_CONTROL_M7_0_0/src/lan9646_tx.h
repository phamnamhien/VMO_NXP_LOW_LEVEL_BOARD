#ifndef LAN9646_TX_H
#define LAN9646_TX_H

#include "S32K388_GMAC.h"
#include <stdint.h>
#include <stdbool.h>

// TX Descriptor structure
typedef struct {
    uint32_t des0;  // Buffer address
    uint32_t des1;  // Reserved
    uint32_t des2;  // Buffer lengths
    uint32_t des3;  // Status/Control
} LAN9646_TxDesc_t;

// TX Ring configuration
#define LAN9646_TX_DESC_COUNT   8
#define LAN9646_TX_BUFFER_SIZE  1536

// Status codes
typedef enum {
    LAN9646_TX_OK = 0,
    LAN9646_TX_BUSY,
    LAN9646_TX_ERROR,
    LAN9646_TX_NO_DESC
} LAN9646_TxStatus_t;

// Functions
void LAN9646_TX_Init(void);
LAN9646_TxStatus_t LAN9646_TX_Send(const uint8_t *data, uint16_t len);
bool LAN9646_TX_IsReady(void);


#endif

