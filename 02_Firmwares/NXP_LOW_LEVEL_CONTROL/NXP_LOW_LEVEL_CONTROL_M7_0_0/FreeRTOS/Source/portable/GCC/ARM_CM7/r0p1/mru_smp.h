/*
 * Copyright 2025 NXP.
 */

#ifndef MRU_SMP_H
#define MRU_SMP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* TODO: add generated S32DS config to select channel MRU, Mbox ID and set SMP value */
#define CFG_MRU_CHANNEL_SMP 6
#define CFG_MRU_INTERRUPT_SMP 0
#define CFG_MRU_MB_ID_SMP 0
#define CFG_MRU_MB_VAL_SMP 0xab
/* Each Cortex-M7 core associated to 2 MRU, identified by IDs 0(first MRU) and 1(second MRU) */
#define CFG_MRU_ASSOCIATED_TO_CORE 1

#define MRU_INT_ID (CFG_MRU_ASSOCIATED_TO_CORE == 0 ? 22 : 23)
#define MRU_INT_PRI 7

#define CORE_ID_2_MRU_ID(coreId) (((coreId) * 0x2) + (CFG_MRU_ASSOCIATED_TO_CORE) * 0x1)

void MRUInit(uint32_t mruId, uint32_t ChannelId, uint32_t mbId, uint32_t InterruptType);
int MRURegisterMBCB(uint32_t cbIndex, void *cbHandler);
void MRUMBSet(uint32_t mruId, uint32_t mruCh, uint32_t mbId, uint32_t val);
void MRUIRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /** MRU_SMP_H */
