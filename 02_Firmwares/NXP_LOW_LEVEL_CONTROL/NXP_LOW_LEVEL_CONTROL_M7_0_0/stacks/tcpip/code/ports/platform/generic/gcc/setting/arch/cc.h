/*
 * Copyright 2017-2020 NXP
 * All rights reserved.
 *
 * This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading, installing, activating and/or otherwise
 * using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software. The production use license in
 * Section 2.3 is expressly granted for this software.
 *
 * This file is derived from lwIP contribution example with the following copyright:
 *
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 */

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Directive 4.9, A function should be used in preference
 * to a function-like macro where they are interchangeable.
 * Function-like macros are used instead of inline functions in order to ensure
 * that the performance will not be decreased if the functions will not be
 * inlined by the compiler.
 *
 */

#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#ifdef USING_RTD
#include "Devassert.h"
#include "lwip/arch.h"

/* Manages custom TCP/IP cache management. Can be used instead of RTD Gmac cache management.
 * Note: Not ported for all the platforms. */
#define NETIF_CUSTOM_CACHE_MANAGEMENT STD_OFF

#if defined D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON)
#include "Cache_Ip.h"
#if (defined(S32E27) || defined(S32S27))
    #include "S32E2_SCB.h"
    #include "S32E2_LMEM64.h"
#elif defined(S32Z27)
    #include "S32Z2_SCB.h"
    #include "S32Z2_LMEM64.h"
#endif /* (defined(S32E27) || defined(S32S27) */
#endif /* D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON)*/
#else
#include "device_registers.h"
#endif /* USING_RTD */
#include <stdlib.h>
#include "lwipcfg.h"


#define LWIP_PROVIDE_ERRNO              1


/* Compiler hints for packing structures */
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#define LWIP_ERROR(message, expression, handler) do { if (!(expression)) { \
  handler;} } while(0)

#ifndef LWIP_PLATFORM_DIAG
#ifdef CPU_SJA1110
#include <stdio.h>
#define U8_F "c"
#define S8_F "c"
#define X8_F "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "lu"
#define S32_F "ld"
#define X32_F "lx"
#define LWIP_PLATFORM_DIAG(x)  do {printf x;} while(0)
#else
#define LWIP_PLATFORM_DIAG(x) do {} while(0)
#endif /* CPU_SJA1110 */
#endif /* LWIP_PLATFORM_DIAG */

#if !defined(BKPT_ASM)

#if defined ( __DCC__ )
#define BKPT_ASM __asm ("BKPT 0\n\t")
#else
#define BKPT_ASM __asm ("BKPT #0\n\t")
#endif

#endif /* BKPT_ASM */

#ifndef LWIP_PLATFORM_ASSERT
#define LWIP_PLATFORM_ASSERT(x) do {(void)(x); while(1); }while(0)
#endif /* LWIP_PLATFORM_ASSERT */

/* C runtime functions redefined */
#define LWIP_RAND() ((u32_t)rand())

#define PPP_INCLUDE_SETTINGS_HEADER

/* Custom TCP/IP cache management. Can be used instead of RTD Gmac cache management.
 * Note: Not ported for all the platforms. */
#if defined D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON)
#if defined CPU_CORTEX_M7
#define __CM7_DCACHE_LINE_SIZE 32U
LOCAL_INLINE void DataCacheClean(void);
LOCAL_INLINE void DataCacheInvbyAddr(const uint32 addr, const uint32 length);
LOCAL_INLINE void DataCacheCleanbyAddr(const uint32 addr, const uint32 length);

LOCAL_INLINE void DataCacheClean(void)
{
    uint32 cacheSetSize;
    uint32 cacheWaySize;
    uint32 setIdx;
    uint32 wayIdx;

    /* CACHE Type shall be selected before any other operation */
    S32_SCB->CSSELR = DCACHE_CSSELR_EN(S32_SCB->CSSELR);
    cacheSetSize = CACHE_CCSIDR_SET_SIZE(S32_SCB->CCSIDR);
    cacheWaySize = CACHE_CCSIDR_WAY_SIZE(S32_SCB->CCSIDR);

    for(setIdx = 0; setIdx < cacheSetSize; setIdx++)
    {
        for(wayIdx = 0; wayIdx < cacheWaySize; wayIdx++)
        {
            S32_SCB->DCCSW = ((setIdx << DCACHE_DCCXSW_SET_SHIFT) & DCACHE_DCCXSW_SET_MASK)
                             | ((wayIdx << DCACHE_DCCXSW_WAY_SHIFT) & DCACHE_DCCXSW_WAY_MASK);
        }
    }
    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();
}
LOCAL_INLINE void DataCacheCleanbyAddr(const uint32 addr, const uint32 length)
{
    if ( length > 0 ) {
      int32_t op_size = length + (((uint32_t)addr) & (__CM7_DCACHE_LINE_SIZE - 1U));
      uint32_t op_addr = (uint32_t)addr /* & ~(__CM7_DCACHE_LINE_SIZE - 1U) */;

      MCAL_DATA_SYNC_BARRIER();

    do {
    	S32_SCB->DCCMVAC = op_addr;             /* register accepts only 32byte aligned values, only bits 31..5 are valid */
      op_addr += __CM7_DCACHE_LINE_SIZE;
      op_size -= __CM7_DCACHE_LINE_SIZE;
    } while ( op_size > 0 );

    MCAL_DATA_SYNC_BARRIER();
    MCAL_INSTRUCTION_SYNC_BARRIER();
    }

}
LOCAL_INLINE void DataCacheInvbyAddr(const uint32 addr, const uint32 length)
{

    if ( length > 0 ) {
        int32_t inv_size = length + (((uint32_t)addr) & (__CM7_DCACHE_LINE_SIZE - 1U));
        uint32_t inv_addr = (uint32_t)addr /* & ~(__CM7_DCACHE_LINE_SIZE - 1U) */;
        S32_SCB->CSSELR = DCACHE_CSSELR_EN(S32_SCB->CSSELR);
        MCAL_DATA_SYNC_BARRIER();

        do {
        	S32_SCB->DCIMVAC = inv_addr;             /* register accepts only 32byte aligned values, only bits 31..5 are valid */
        	inv_addr += __CM7_DCACHE_LINE_SIZE;
            inv_size -= __CM7_DCACHE_LINE_SIZE;
        } while ( inv_size > 0 );
        MCAL_DATA_SYNC_BARRIER();
        MCAL_INSTRUCTION_SYNC_BARRIER();
    }
}
#elif defined CPU_CORTEX_M33
LOCAL_INLINE void m4_cache_flush_buffer(uint32_t start_addr, long size);
LOCAL_INLINE void m4_cache_flush(void);
LOCAL_INLINE void m4_cache_invalidate(void);

LOCAL_INLINE void m4_cache_flush_buffer(uint32_t start_addr, long size)
{
  /* 32B line size => number of loops = size to invalidate divided by line size in bits plus 1 */
  uint32_t loops = size/32;
  uint32_t addr = start_addr;
  uint32_t i;

  for (i=0; i<loops; i++,addr+=32){
    /* LACC 27        - Read:             0x0  */
    /* LADSEL 26      - Physical Address: 0x1  */
    /* LCMD 25-24     - Push:             0x10 */
    /* TDSEL 16       - Data:             0x0  */
    /* WSEL 14        - Way 0:            0x0  */
    IP_SMU__LMEM64->PCCLCR = 0x06000000;

    /* set physical address as start address with 2 LSB dropped */
    IP_SMU__LMEM64->PCCSAR = addr&0xFFFFFFFC;

    /* set csar[lgo] to initiate command indicated by bits 27-24 in clcr */
    IP_SMU__LMEM64->PCCSAR |= LMEM64_PCCSAR_LGO(1);

    /* wait until the csar[lgo] bit clears to indicate command complete */
    while((IP_SMU__LMEM64->PCCSAR & LMEM64_PCCSAR_LGO_MASK) == LMEM64_PCCSAR_LGO_MASK);
  }

}

LOCAL_INLINE void m4_cache_flush(void)
{
	/* Cache Set Command: set command bits in CCR */
	/* set invalidate way 1 and invalidate way 0 bits */
	IP_SMU__LMEM64->PCCCR = 0x0A000000; /* set INVW0 and INVW1 */

	/* set ccr[go] bit to initiate command to invalidate cache */
	IP_SMU__LMEM64->PCCCR |= LMEM64_PCCCR_GO(1);

	/* wait until the ccr[go] bit clears to indicate command complete */
	while((IP_SMU__LMEM64->PCCCR & LMEM64_PCCCR_GO_MASK) == LMEM64_PCCCR_GO_MASK);

	/* Cache Set Command: set command bits in CCR */
	/* set invalidate way 1 and invalidate way 0 bits */
	IP_SMU__LMEM64->PSCCR = 0x0A000000; /* set INVW0 and INVW1 */

	/* set ccr[go] bit to initiate command to invalidate cache */
	IP_SMU__LMEM64->PSCCR |= LMEM64_PSCCR_GO(1);

	/* wait until the ccr[go] bit clears to indicate command complete */
	while((IP_SMU__LMEM64->PSCCR & LMEM64_PSCCR_GO_MASK) == LMEM64_PSCCR_GO_MASK);

	MCAL_DATA_SYNC_BARRIER();
	MCAL_INSTRUCTION_SYNC_BARRIER();
}


LOCAL_INLINE void m4_cache_invalidate(void)
{
	/* Cache Set Command: set command bits in CCR */
	/* set invalidate way 1 and invalidate way 0 bits */
	IP_SMU__LMEM64->PCCCR = 0x05000000; /* set INVW0 and INVW1 */

	/* set ccr[go] bit to initiate command to invalidate cache */
	IP_SMU__LMEM64->PCCCR |= LMEM64_PCCCR_GO(1);

	/* wait until the ccr[go] bit clears to indicate command complete */
	while((IP_SMU__LMEM64->PCCCR & LMEM64_PCCCR_GO_MASK) == LMEM64_PCCCR_GO_MASK);

	/* Cache Set Command: set command bits in CCR */
	/* set invalidate way 1 and invalidate way 0 bits */
	IP_SMU__LMEM64->PSCCR = 0x05000000; /* set INVW0 and INVW1 */

	/* set ccr[go] bit to initiate command to invalidate cache */
	IP_SMU__LMEM64->PSCCR |= LMEM64_PSCCR_GO(1);

	/* wait until the ccr[go] bit clears to indicate command complete */
	while((IP_SMU__LMEM64->PSCCR & LMEM64_PSCCR_GO_MASK) == LMEM64_PSCCR_GO_MASK);

	MCAL_DATA_SYNC_BARRIER();
	MCAL_INSTRUCTION_SYNC_BARRIER();

}
#endif /* CPU_CORTEX_M7 */
#endif /* D_CACHE_ENABLE && (NETIF_CUSTOM_CACHE_MANAGEMENT == STD_ON)*/
#endif /* LWIP_ARCH_CC_H */
