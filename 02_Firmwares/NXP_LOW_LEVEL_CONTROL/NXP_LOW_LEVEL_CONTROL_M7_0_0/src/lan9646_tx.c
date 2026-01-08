#include "lan9646_tx.h"
#include "log_debug.h"
#include "Mcal.h"
#include "Gmac_Ip.h"
#include <string.h>

#define TAG "LAN9646_TX"

extern GMAC_Type* const Gmac_apxBases[FEATURE_GMAC_NUM_INSTANCES];

static LAN9646_TxDesc_t *txDescriptors = NULL;
static uint32_t txDescCount = 0;
static volatile uint32_t txCurrentIdx = 0;

#define TDES3_OWN   (1U << 31)
#define TDES3_FD    (1U << 29)
#define TDES3_LD    (1U << 28)

void LAN9646_TX_Init(void)
{
    GMAC_Type *gmac = Gmac_apxBases[0];
    uint32_t i;

    LOG_I(TAG, "Using RTD TX ring...");

    txDescriptors = (LAN9646_TxDesc_t *)gmac->DMA_CH0_TXDESC_LIST_ADDRESS;
    txDescCount = gmac->DMA_CH0_TXDESC_RING_LENGTH + 1;

    // FORCE CLEAR tất cả descriptors
    for(i = 0; i < txDescCount; i++) {
        txDescriptors[i].des3 &= ~TDES3_OWN;
    }

    MCAL_DATA_SYNC_BARRIER();

    LOG_I(TAG, "TX Desc: 0x%08X", (uint32_t)txDescriptors);
    LOG_I(TAG, "TX Count: %d", txDescCount);
    LOG_I(TAG, "MAC_CONFIG: 0x%08X", gmac->MAC_CONFIGURATION);
    LOG_I(TAG, "TX_PKT_COUNT: %d", gmac->TX_PACKET_COUNT_GOOD_BAD);

    LOG_I(TAG, "TX Ready!");
}

bool LAN9646_TX_IsReady(void)
{
    if(!txDescriptors) return false;

    uint32_t idx = txCurrentIdx;
    return !(txDescriptors[idx].des3 & TDES3_OWN);
}

LAN9646_TxStatus_t LAN9646_TX_Send(const uint8_t *data, uint16_t len)
{
    GMAC_Type *gmac = Gmac_apxBases[0];
    uint32_t idx;
    LAN9646_TxDesc_t *desc;
    uint8_t *buffer;

    if(!txDescriptors || !data || len == 0 || len > 1536) {
        return LAN9646_TX_ERROR;
    }

    idx = txCurrentIdx;
    desc = &txDescriptors[idx];

    if(desc->des3 & TDES3_OWN) {
        LOG_W(TAG, "TX busy");
        return LAN9646_TX_BUSY;
    }

    buffer = (uint8_t *)desc->des0;
    memcpy(buffer, data, len);

    desc->des1 = 0;
    desc->des2 = len & 0x3FFF;
    desc->des3 = TDES3_FD | TDES3_LD | (len & 0x7FFF);

    MCAL_DATA_SYNC_BARRIER();
    desc->des3 |= TDES3_OWN;
    MCAL_DATA_SYNC_BARRIER();

    // SKIP descriptor lẻ - CHỈ DÙNG CHẴN (0,2,4,6)
    txCurrentIdx = (idx + 2) % txDescCount;

    gmac->DMA_CH0_TXDESC_TAIL_POINTER = (uint32_t)&txDescriptors[txCurrentIdx];

    LOG_I(TAG, "TX: %d bytes, desc[%d]", len, idx);

    return LAN9646_TX_OK;
}

