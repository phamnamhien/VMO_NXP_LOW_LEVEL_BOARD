/*
 * Copyright 2025 NXP.
 */

#include "FreeRTOS.h"

#ifdef MRU_SMP_USE

#include "mru_smp.h"

#define MRU_INSTANCE_COUNT 8

uint32_t MRU_BASE_ADDRS[MRU_INSTANCE_COUNT] = {
    0x4045C000u, /** M7 core0 MRU0 base address */
    0x40478000u, /** M7 core0 MRU1 base address */
    0x40A68000u, /** M7 core1 MRU2 base address */
    0x40A84000u, /** M7 core1 MRU3 base address */
    0x40AA0000u, /** M7 core2 MRU4 base address */
    0x40ABC000u, /** M7 core2 MRU5 base address */
    0x40AD8000u, /** M7 core3 MRU6 base address */
    0x40AF4000u, /** M7 core3 MRU7 base address */
};

#define MRU_SET_CFG(mruId, chId, idx, val) \
    *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) * 0x10) + ((idx) * 0x4)) = val

#define MRU_GET_CFG(mruId, chId, idx, val) \
    val = *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) * 0x10) + ((idx) * 0x4))

#define MRU_CLR_MBSTAT(mruId, chId, val) \
    *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) * 0x10) + 0x8) = val

#define MRU_GET_MBSTAT(mruId, chId, val) \
    val = *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) * 0x10) + 0x8)

#define MRU_SET_MAILBOX(mruId, chId, idx, val) \
    *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + ((chId) * 0x4000) + ((idx) * 0x4)) = val

#define MRU_GET_MAILBOX(mruId, chId, idx, val) \
    val = *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + ((chId) * 0x4000) + ((idx) * 0x4))

#define MRU_CFG0_IE_MASK 0x00000004UL
#define MRU_CFG0_CHE_MASK 0x00000001UL

extern void Core_registerIsrHandler(uint16_t irq_id, void (*isr_handler)(void));

/**
 * @brief Init MRU
 * @details Fucntion used to enable MRU chanels [1-n], message boxes [0-n] and configuration interrupt type [0-3]
 * for message boxes. Without this MRU module does not work.
 */
void MRUInit(uint32_t mruId, uint32_t ChannelId, uint32_t mbId, uint32_t InterruptType)
{
    uint32_t val = 0;

    /* Enable channel */
    MRU_GET_CFG(mruId, ChannelId, 0, val);
    val |= MRU_CFG0_CHE_MASK;
    MRU_SET_CFG(mruId, ChannelId, 0, val);

    /* Clear corresponding message boxes */
    MRU_SET_MAILBOX(mruId, ChannelId, mbId, 0);

    /* Clear corresponding mailbox status */
    MRU_GET_MBSTAT(mruId, ChannelId, val);
    val |= (1 << (mbId + 16));
    MRU_CLR_MBSTAT(mruId, ChannelId, val);

    /* Set corresponding message boxes interrupts type */
    MRU_GET_CFG(mruId, ChannelId, 1, val);
    val |= (InterruptType << (mbId * 2));
    MRU_SET_CFG(mruId, ChannelId, 1, val);

    /* Enable corresponding mailboxes and their associated interrupts */
    MRU_GET_CFG(mruId, ChannelId, 0, val);
    val |= ((1 << (mbId + 16)) | (MRU_CFG0_IE_MASK));
    MRU_SET_CFG(mruId, ChannelId, 0, val);

    /* Register IRQ interrupt */
    Core_registerIsrHandler(MRU_INT_ID, MRUIRQHandler);
    NVIC_SET_PRIORITY(MRU_INT_ID, MRU_INT_PRI);
    NVIC_ENABLE_IRQ(MRU_INT_ID);
}

/**
 * @brief        Write mailbox if available
 * @details      This function writes the selected mailbox, using channel
 *
 * @param[in]    mruId           select the MRU ID [0-n]
 * @param[in]    mruCh           select the MRU Channel ID [1-n]
 * @param[in]    mbId            select the mailbox [0-n]
 * @param[in]    val             value to be written
 *
 * @return       none
 */
void MRUMBSet(uint32_t mruId, uint32_t mruCh, uint32_t mbId, uint32_t val)
{
    MRU_SET_MAILBOX(mruId, mruCh, mbId, val);
}

typedef void (*pfMailBoxCallBack)(uint32_t, uint32_t);

/**
 * @brief        Callback for each MRU interrupt
 */
pfMailBoxCallBack MailBoxArray[4] = {
    (pfMailBoxCallBack)0,
    (pfMailBoxCallBack)0,
    (pfMailBoxCallBack)0,
    (pfMailBoxCallBack)0,
};

/**
 * @brief        Handle MRU interrupt and call Hypervisor CallBack if any
 * @details      This function handles interrupt of the channel and calls specified CallBack
 *
 * @return       none
 */
void MRUIRQHandler(void)
{
    uint32_t mruSts = 0;
    uint32_t coreId = (GET_MSCM_CPXNUM & CPXNUM_CPN_MASK) - CORE_M7_OFFSET;
    uint32_t mruId = CORE_ID_2_MRU_ID(coreId);
    uint32_t mbId = CFG_MRU_MB_ID_SMP;
    uint32_t interruptType = CFG_MRU_INTERRUPT_SMP;

    MRU_GET_MBSTAT(mruId, CFG_MRU_CHANNEL_SMP, mruSts);
    MRU_CLR_MBSTAT(mruId, CFG_MRU_CHANNEL_SMP, mruSts);
    if ((mruSts & (1 << (mbId + 16))) && MailBoxArray[interruptType])
    {
        uint32_t mbVal;
        MRU_GET_MAILBOX(mruId, CFG_MRU_CHANNEL_SMP, mbId, mbVal);

        if (CFG_MRU_MB_VAL_SMP == mbVal)
        {
            MailBoxArray[interruptType](mbVal, coreId); /* mailbox callback */
        }
    }
}

/**
 * @brief        Register a MRU CallBack
 * @details      This function register a CallBack on channel
 *
 * @param[in]    cbIndex         select the MRU interrupt type [0-3]
 * @param[in]    cbHandler       callback handler
 *
 * @return       The function returns 0 success, -1 already registered
 */
int MRURegisterMBCB(uint32_t cbIndex, void *cbHandler)
{
    int retVal = -1;

    if (!MailBoxArray[cbIndex])
    {
        MailBoxArray[cbIndex] = (pfMailBoxCallBack)cbHandler;
        retVal = 0;
    }

    return retVal;
}

#endif /** MRU_SMP_USE */
