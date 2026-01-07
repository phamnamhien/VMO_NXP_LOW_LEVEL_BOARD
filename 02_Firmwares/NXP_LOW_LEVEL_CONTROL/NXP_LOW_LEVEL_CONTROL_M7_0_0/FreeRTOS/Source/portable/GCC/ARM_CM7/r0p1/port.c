/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM7 port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#ifdef MRU_SMP_USE
#include "mru_smp.h"
#endif
#if( configENABLE_MPU == 1 )
#include "mpu_syscall_numbers.h"
#endif /* configENABLE_MPU == 1 */

#if( configENABLE_MPU == 1 )
/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
 * all the API functions to use the MPU wrappers.  That should only be done when
 * task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#endif /* configENABLE_MPU == 1 */

#ifndef __VFP_FP__
    #error This port can only be used when the project options are configured to enable hardware floating point support.
#endif

#if( configENABLE_MPU == 1 )
#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#endif /* configENABLE_MPU == 1 */

#if( configENABLE_MPU == 1 )
#ifndef configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS
#warning "configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS is not defined. We recommend defining it to 0 in FreeRTOSConfig.h for better security."
#define configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS    1
#endif
#endif /* #if( configENABLE_MPU == 1 ) */

/* Prototype of all Interrupt Service Routines (ISRs). */
typedef void (*portISR_t)(void);

/* Constants required to manipulate the core.  Registers first... */
#define portNVIC_SYSTICK_CTRL_REG             ( *( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG             ( *( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_SHPR2_REG                    ( *( ( volatile uint32_t * ) 0xe000ed1c ) )
#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )

#if( configENABLE_MPU == 1 )
#define portNVIC_SYS_CTRL_STATE_REG           ( *( ( volatile uint32_t * ) 0xe000ed24 ) )
#define portNVIC_MEM_FAULT_ENABLE             ( 1UL << 16UL )

/* Constants required to access and manipulate the MPU. */
#define portMPU_TYPE_REG                          ( *( ( volatile uint32_t * ) 0xe000ed90 ) )
#define portMPU_REGION_BASE_ADDRESS_REG           ( *( ( volatile uint32_t * ) 0xe000ed9c ) )
#define portMPU_REGION_ATTRIBUTE_REG              ( *( ( volatile uint32_t * ) 0xe000eda0 ) )
#define portMPU_CTRL_REG                          ( *( ( volatile uint32_t * ) 0xe000ed94 ) )
#define portEXPECTED_MPU_TYPE_VALUE               ( configTOTAL_MPU_REGIONS << 8UL )
#define portMPU_ENABLE                            ( 0x01UL )
#define portMPU_BACKGROUND_ENABLE                 ( 1UL << 2UL )
#define portPRIVILEGED_EXECUTION_START_ADDRESS    ( 0UL )
#define portMPU_REGION_VALID                      ( 0x10UL )
#define portMPU_REGION_ENABLE                     ( 0x01UL )

/* the PERIPHERALS addresses for current platform */
#define portPERIPHERALS_START_ADDRESS             0x40000000UL
#define portPERIPHERALS_END_ADDRESS               0x5FFFFFFFUL
#endif /* #if( configENABLE_MPU == 1 ) */

/* Constants required to access and manipulate the SysTick. */
#define portNVIC_SYSTICK_CLK_BIT              ( 1UL << 2UL )
#define portNVIC_SYSTICK_INT_BIT              ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT           ( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT       ( 1UL << 16UL )
#define portNVIC_PENDSVCLEAR_BIT              ( 1UL << 27UL )
#define portNVIC_PEND_SYSTICK_SET_BIT         ( 1UL << 26UL )
#define portNVIC_PEND_SYSTICK_CLEAR_BIT       ( 1UL << 25UL )

#define portMIN_INTERRUPT_PRIORITY            ( 255UL )
#define portNVIC_PENDSV_PRI                   ( ( ( uint32_t ) portMIN_INTERRUPT_PRIORITY ) << 16UL )
#define portNVIC_SYSTICK_PRI                  ( ( ( uint32_t ) portMIN_INTERRUPT_PRIORITY ) << 24UL )

/* Constants used to check the installation of the FreeRTOS interrupt handlers. */
#define portSCB_VTOR_REG                      ( *( ( portISR_t ** ) 0xE000ED08 ) )
#define portVECTOR_INDEX_SVC                  ( 11 )
#define portVECTOR_INDEX_PENDSV               ( 14 )

/* Constants required to check the validity of an interrupt priority. */
#define portFIRST_USER_INTERRUPT_NUMBER       ( 16 )
#define portNVIC_IP_REGISTERS_OFFSET_16       ( 0xE000E3F0 )
#define portAIRCR_REG                         ( *( ( volatile uint32_t * ) 0xE000ED0C ) )
#define portMAX_8_BIT_VALUE                   ( ( uint8_t ) 0xff )
#define portTOP_BIT_OF_BYTE                   ( ( uint8_t ) 0x80 )
#define portMAX_PRIGROUP_BITS                 ( ( uint8_t ) 7 )
#define portPRIORITY_GROUP_MASK               ( 0x07UL << 8UL )
#define portPRIGROUP_SHIFT                    ( 8UL )

/* Masks off all bits but the VECTACTIVE bits in the ICSR register. */
#define portVECTACTIVE_MASK                   ( 0xFFUL )

/* Constants required to manipulate the VFP. */
#define portFPCCR                             ( ( volatile uint32_t * ) 0xe000ef34 ) /* Floating point context control register. */
#define portASPEN_AND_LSPEN_BITS              ( 0x3UL << 30UL )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR                          ( 0x01000000UL )
#define portINITIAL_EXC_RETURN                    ( 0xfffffffdUL )

#if( configENABLE_MPU == 1 )
#define portINITIAL_CONTROL_IF_UNPRIVILEGED       ( 0x03 )
#define portINITIAL_CONTROL_IF_PRIVILEGED         ( 0x02 )
#endif /* configENABLE_MPU == 1 */

/* The systick is a 24-bit counter. */
#define portMAX_24_BIT_NUMBER                 ( 0xffffffUL )

/* For strict compliance with the Cortex-M spec the task start address should
 * have bit-0 clear, as it is loaded into the PC on exit from an ISR. */
#define portSTART_ADDRESS_MASK                ( ( StackType_t ) 0xfffffffeUL )

/* A fiddle factor to estimate the number of SysTick counts that would have
 * occurred while the SysTick counter is stopped during tickless idle
 * calculations. */
#ifndef portMISSED_COUNTS_FACTOR
#define portMISSED_COUNTS_FACTOR              ( 94UL )
#endif

/* Let the user override the default SysTick clock rate.  If defined by the
 * user, this symbol must equal the SysTick clock rate when the CLK bit is 0 in the
 * configuration register. */
#ifndef configSYSTICK_CLOCK_HZ
#define configSYSTICK_CLOCK_HZ             ( configCPU_CLOCK_HZ )
/* Ensure the SysTick is clocked at the same frequency as the core. */
#define portNVIC_SYSTICK_CLK_BIT_CONFIG    ( portNVIC_SYSTICK_CLK_BIT )
#else
    /* Select the option to clock SysTick not at the same frequency as the core. */
    #define portNVIC_SYSTICK_CLK_BIT_CONFIG    ( 0 )
#endif

/* Let the user override the pre-loading of the initial LR with the address of
 * prvTaskExitError() in case it messes up unwinding of the stack in the
 * debugger. */
#ifdef configTASK_RETURN_ADDRESS
    #define portTASK_RETURN_ADDRESS    configTASK_RETURN_ADDRESS
#else
#define portTASK_RETURN_ADDRESS    prvTaskExitError
#endif

/*
 * Setup the timer to generate the tick interrupts.  The implementation in this
 * file is weak to allow application writers to change the timer used to
 * generate the tick interrupt.
 */
void vPortSetupTimerInterrupt(void);

/*
 * Exception handlers.
 */
void xPortPendSVHandler( void ) __attribute__( ( naked ) ) PRIVILEGED_FUNCTION;
void xPortSysTickHandler( void ) PRIVILEGED_FUNCTION;
void vPortSVCHandler( void ) PRIVILEGED_FUNCTION;

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
void prvPortStartFirstTask(void) __attribute__( ( naked ) ) PRIVILEGED_FUNCTION;

#if( configENABLE_MPU == 1 )
/*
 * Configure a number of standard MPU regions that are used by all tasks.
 */
void prvSetupMPU(void) PRIVILEGED_FUNCTION;

/**
 * @brief Checks whether or not the processor is privileged.
 *
 * @return 1 if the processor is already privileged, 0 otherwise.
 */
BaseType_t xIsPrivileged( void ) __attribute__( ( naked ) );

/**
 * @brief Lowers the privilege level by setting the bit 0 of the CONTROL
 * register.
 *
 * Bit 0 of the CONTROL register defines the privilege level of Thread Mode.
 *  Bit[0] = 0 --> The processor is running privileged
 *  Bit[0] = 1 --> The processor is running unprivileged.
 */
void vResetPrivilege( void ) __attribute__( ( naked ) );

/**
 * @brief Make a task unprivileged.
 */
void vPortSwitchToUserMode(void);

/**
 * @brief Enter critical section.
 */
#if ( configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS == 1 )
void vPortEnterCritical(void) FREERTOS_SYSTEM_CALL;
#else
    void vPortEnterCritical( void ) PRIVILEGED_FUNCTION;
#endif

/**
 * @brief Exit from critical section.
 */
#if ( configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS == 1 )
void vPortExitCritical(void) FREERTOS_SYSTEM_CALL;
#else
    void vPortExitCritical( void ) PRIVILEGED_FUNCTION;
#endif
/*
 * Return the smallest MPU region size that a given number of bytes will fit
 * into.  The region size is returned as the value that should be programmed
 * into the region attribute register for that region.
 */
static uint32_t prvGetMPURegionSizeSetting(uint32_t ulActualSizeInBytes) PRIVILEGED_FUNCTION;

/*
 * Starts the scheduler by restoring the context of the first task to run.
 */
static void prvRestoreContextOfFirstTask( void ) __attribute__( ( naked ) ) PRIVILEGED_FUNCTION;

#endif /* #if( configENABLE_MPU == 1 ) */

/*
 * Function to enable the VFP.
 */
static void vPortEnableVFP(void) __attribute__( ( naked ) ) PRIVILEGED_FUNCTION;

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError(void);

/*-----------------------------------------------------------*/

/* Each task maintains its own interrupt status in the critical nesting
 * variable. */
UBaseType_t uxCriticalNesting[configNUMBER_OF_CORES] = { 0U };
UBaseType_t uxInterruptNested[configNUMBER_OF_CORES] = { 0U };
static volatile uint8_t flagCheckStartFirstTask[configNUMBER_OF_CORES] = { 0 };

/*
 * The number of SysTick increments that make up one tick period.
 */
#if ( configUSE_TICKLESS_IDLE == 1 )
    static uint32_t ulTimerCountsForOneTick = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * The maximum number of tick periods that can be suppressed is limited by the
 * 24 bit resolution of the SysTick timer.
 */
#if ( configUSE_TICKLESS_IDLE == 1 )
    static uint32_t xMaximumPossibleSuppressedTicks = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * Compensate for the CPU cycles that pass while the SysTick is stopped (low
 * power functionality only.
 */
#if ( configUSE_TICKLESS_IDLE == 1 )
    static uint32_t ulStoppedTimerCompensation = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * Used by the portASSERT_IF_INTERRUPT_PRIORITY_INVALID() macro to ensure
 * FreeRTOS API functions are not called from interrupts that have been assigned
 * a priority above configMAX_SYSCALL_INTERRUPT_PRIORITY.
 */
#if ( configASSERT_DEFINED == 1 )
    static uint8_t ucMaxSysCallPriority = 0;
    static uint32_t ulMaxPRIGROUPValue = 0;
    static const volatile uint8_t *const pcInterruptPriorityRegisters = (const volatile uint8_t* const ) portNVIC_IP_REGISTERS_OFFSET_16;
#endif /* configASSERT_DEFINED */

#if (configNUMBER_OF_CORES>1)
    extern volatile void *volatile pxCurrentTCBs[ configNUMBER_OF_CORES ];
#else
    extern volatile void *volatile pxCurrentTCB;
#endif
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
#if( configENABLE_MPU == 1 )
uint32_t task_debug_pattern = 0;
StackType_t* pxPortInitialiseStack(StackType_t *pxTopOfStack,
                    TaskFunction_t pxCode,
                    void *pvParameters,
                    BaseType_t xRunPrivileged,
                    xMPU_SETTINGS *xMPUSettings) {
    if (xRunPrivileged == pdTRUE) {
        xMPUSettings->ulTaskFlags |= portTASK_IS_PRIVILEGED_FLAG;
        xMPUSettings->ulContext[0] = portINITIAL_CONTROL_IF_PRIVILEGED;
    } else {
        xMPUSettings->ulTaskFlags &= (~( portTASK_IS_PRIVILEGED_FLAG));
        xMPUSettings->ulContext[0] = portINITIAL_CONTROL_IF_UNPRIVILEGED;
    }

    xMPUSettings->ulContext[1] = 0x00040404 + (task_debug_pattern << 24); /* r4. */
    xMPUSettings->ulContext[2] = 0x00050505 + (task_debug_pattern << 24); /* r5. */
    xMPUSettings->ulContext[3] = 0x00060606 + (task_debug_pattern << 24); /* r6. */
    xMPUSettings->ulContext[4] = 0x00070707 + (task_debug_pattern << 24); /* r7. */
    xMPUSettings->ulContext[5] = 0x00080808 + (task_debug_pattern << 24); /* r8. */
    xMPUSettings->ulContext[6] = 0x00090909 + (task_debug_pattern << 24); /* r9. */
    xMPUSettings->ulContext[7] = 0x00101010 + (task_debug_pattern << 24); /* r10. */
    xMPUSettings->ulContext[8] = 0x00111111 + (task_debug_pattern << 24); /* r11. */
    xMPUSettings->ulContext[9] = portINITIAL_EXC_RETURN; /* EXC_RETURN. */

    xMPUSettings->ulContext[10] = (uint32_t) (pxTopOfStack - 8); /* PSP with the hardware saved stack. */
    xMPUSettings->ulContext[11] = (uint32_t) pvParameters; /* r0. */
    xMPUSettings->ulContext[12] = 0x00010101 + (task_debug_pattern << 24); /* r1. */
    xMPUSettings->ulContext[13] = 0x00020202 + (task_debug_pattern << 24); /* r2. */
    xMPUSettings->ulContext[14] = 0x00030303 + (task_debug_pattern << 24); /* r3. */
    xMPUSettings->ulContext[15] = 0x00121212 + (task_debug_pattern << 24); /* r12. */
    xMPUSettings->ulContext[16] = 0xFFFFFFFF; /* LR or R14_USR, invalid return  */
    xMPUSettings->ulContext[17] = ((uint32_t) pxCode) & portSTART_ADDRESS_MASK; /* PC. */
    xMPUSettings->ulContext[18] = portINITIAL_XPSR; /* xPSR. */

#if ( configUSE_MPU_WRAPPERS_V1 == 0 )
    {
        /* Ensure that the system call stack is double word aligned. */
        xMPUSettings->xSystemCallStackInfo.pulSystemCallStack = &( xMPUSettings->xSystemCallStackInfo.ulSystemCallStackBuffer[ configSYSTEM_CALL_STACK_SIZE - 1 ] );
        xMPUSettings->xSystemCallStackInfo.pulSystemCallStack = ( uint32_t * ) ( ( uint32_t ) ( xMPUSettings->xSystemCallStackInfo.pulSystemCallStack ) &
                                                                                 ( uint32_t ) ( ~( portBYTE_ALIGNMENT_MASK ) ) );

        /* This is not NULL only for the duration of a system call. */
        xMPUSettings->xSystemCallStackInfo.pulTaskStack = NULL;
    }
    #endif /* #if ( configUSE_MPU_WRAPPERS_V1 == 0 ) */

    task_debug_pattern ++;

    return &(xMPUSettings->ulContext[19]);
}
#else /* #if( configENABLE_MPU == 1 ) */
StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                     TaskFunction_t pxCode,
                                     void * pvParameters )
{
    /* Simulate the stack frame as it would be created by a context switch
     * interrupt. */

    /* Offset added to account for the way the MCU uses the stack on entry/exit
     * of interrupts, and to ensure alignment. */
    pxTopOfStack--;

    *pxTopOfStack = portINITIAL_XPSR;                                    /* xPSR */
    pxTopOfStack--;
    *pxTopOfStack = ( ( StackType_t ) pxCode ) & portSTART_ADDRESS_MASK; /* PC */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) portTASK_RETURN_ADDRESS;             /* LR */

    /* Save code space by skipping register initialisation. */
    pxTopOfStack -= 5;                            /* R12, R3, R2 and R1. */
    *pxTopOfStack = ( StackType_t ) pvParameters; /* R0 */

    /* A save method is being used that requires each task to maintain its
     * own exec return value. */
    pxTopOfStack--;
    *pxTopOfStack = portINITIAL_EXC_RETURN;

    pxTopOfStack -= 8; /* R11, R10, R9, R8, R7, R6, R5 and R4. */

    return pxTopOfStack;
}
#endif /* #if( configENABLE_MPU == 1 ) */
/*-----------------------------------------------------------*/

static void prvTaskExitError(void)
{
    volatile uint32_t ulDummy = 0;

    /* A function that implements a task must not exit or attempt to return to
     * its caller as there is nothing to return to.  If a task wants to exit it
     * should instead call vTaskDelete( NULL ).
     *
     * Artificially force an assert() to be triggered if configASSERT() is
     * defined, then stop here so application writers can catch the error. */
    configASSERT(uxCriticalNesting[ portGET_CORE_ID() ] == ~0UL);
    portDISABLE_INTERRUPTS();

    while( ulDummy == 0 )
    {
        /* This file calls prvTaskExitError() after the scheduler has been
         * started to remove a compiler warning about the function being defined
         * but never called.  ulDummy is used purely to quieten other warnings
         * about code appearing after this function is called - making ulDummy
         * volatile makes the compiler think the function could return and
         * therefore not output an 'unreachable code' warning for code that appears
         * after it. */
    }
}
/*-----------------------------------------------------------*/
#if (configNUMBER_OF_CORES>1)

/* Variable to check semaphore gate lock or unlock (1: lock, 0: unlock)*/
__attribute__((section("..mcal_bss_no_cacheable"))) __attribute__((aligned(4))) uint32_t sema_gate_lock[portRTOS_LOCK_COUNT];

/* Sync core flags */
__attribute__((section("..mcal_bss_no_cacheable"))) __attribute__ ((aligned (4))) static volatile uint32_t sync_flags = 0;

/* Which core owns the lock */
__attribute__((section("..mcal_bss_no_cacheable"))) __attribute__((aligned(4))) volatile uint32_t ucOwnedByCore[portMAX_CORE_COUNT];

/* Lock count a core owns */
__attribute__((section("..mcal_bss_no_cacheable"))) __attribute__((aligned(4))) volatile uint32_t ucRecursionCountByLock[portRTOS_LOCK_COUNT];

void xCoreSyncSignalHandler(void);

// static inline
uint32_t Get_32(volatile uint32_t *x)
{
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("dmb sy" ::: "memory");
    return *x;
}

/* Write 32b value shared between cores */
// static inline
void Set_32(volatile uint32_t *x, uint32_t value)
{
    *x = value;
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("dmb sy" ::: "memory");
}

uint8_t vPortGET_CORE_ID(void)
{
    return ((GET_MSCM_CPXNUM & CPXNUM_CPN_MASK) - CORE_M7_OFFSET);
}

#ifndef configUSE_HW_SEMA42
#define configUSE_HW_SEMA42 0
#endif

#if (configUSE_HW_SEMA42 == 1)
#if defined(CPU_S32K566)
uint32_t SEMA42_BASE_ADDRS[2] = {
    0x40154000u,
    0x404a4000u,
};

#define CFG_SEMA42_ID 1 /* Select the second SEMA42 in the array */
#define SEMA42_LOCK_VALUE(core_id) (core_id + CORE_M7_OFFSET + 1)
#else /* S32K3XX */
uint32_t SEMA42_BASE_ADDRS[1] = {
    0x40460000u,
};

#define CFG_SEMA42_ID 0 /* Select the first SEMA42 in the array */
#define SEMA42_LOCK_VALUE(core_id) 1
#endif

/* SEMA42 Gate register width in 8 bits */
#define SEMA42_GTFSM(sema42_id, gate_id) *(volatile uint8_t *)(SEMA42_BASE_ADDRS[sema42_id] + gate_id)
#endif

static uint32_t sema_read_gate(uint32_t gate_id)
{
    __asm__ volatile("isb sy" ::: "memory");
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("dmb sy" ::: "memory");
#if (configUSE_HW_SEMA42 == 1)
    return SEMA42_GTFSM(CFG_SEMA42_ID, gate_id);
#else
    uint32_t old_val;

    /* Read back the current key value and mark the memory as exclusive*/
    /* LDREX return in %0 the word loaded from memory */
    __asm__ volatile("LDREX %0, [%1]" : "=r"(old_val) :  "r"(&sema_gate_lock[gate_id]));

    return old_val;
#endif
}

static uint32_t sema_write_gate(uint32_t value, uint32_t gate_id)
{
    uint32_t success = 0;

#if (configUSE_HW_SEMA42 == 1)
    SEMA42_GTFSM(CFG_SEMA42_ID, gate_id) = (uint8_t)value;
#else
    /* Try to write new value to lock using STREX*/

    /* returns
    0  if the instruction succeeds
    1  if the instruction is locked out.
    */
   __asm__ volatile("STREX %0, %1, [%2]" : "=&r"(success) : "r"(value), "r"(&sema_gate_lock[gate_id]));
#endif

    __asm__ volatile("isb sy" ::: "memory");
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("dmb sy" ::: "memory");

#if (configUSE_HW_SEMA42 == 1)
    if (SEMA42_GTFSM(CFG_SEMA42_ID, gate_id) != (uint8_t)value)
    {
        success = 1;
    }
#endif

    return success;
}

/* Function to get lock */
uint8_t sema_lock(uint32_t gate_id)
{
#if (configUSE_HW_SEMA42 == 1)
    uint32_t value = SEMA42_LOCK_VALUE(vPortGET_CORE_ID());
#else
    uint32_t value = 1;
#endif

    do{
        /* Call the load exclusive function to read the current value of the lock */
        /* lock = 1: This means there is already a different process */
        if (sema_read_gate(gate_id) != 0)
        {
            return 1;
        }

        /* Try using store_exclusive to write the value 1 to the lock (to lock it).*/
    } while (sema_write_gate(value, gate_id) != 0); /* If it fails (lock is changed by another process), try again */

    return 0;
}

/* Function to release the lock (unlock)*/
void sema_unlock(uint32_t gate_id)
{
#if (configUSE_HW_SEMA42 == 1)
    SEMA42_GTFSM(CFG_SEMA42_ID, gate_id) = 0;
#else
    sema_gate_lock[gate_id] = 0;
#endif
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("dmb sy" ::: "memory");
}

#ifndef CPU_S32K566
void Core_registerIsrHandler(uint16_t irq_id, void (*isr_handler)(void))
{
    uint32_t *pVectorRam = portSCB_VTOR_REG;
        /* Set handler into vector table */
    pVectorRam[irq_id+16] = (uint32_t)isr_handler;

}
#endif /* defined CPU_S32K566 */

void vYieldCore( int xCoreID )
{
     /* Remove warning if configASSERT is not defined.
     * xCoreID is not used in this function due to this is a dual-core system. The yielding core must be different from the current core. */
    configASSERT( xCoreID != ( int ) portGET_CORE_ID() );
#ifdef MRU_SMP_USE
    MRUMBSet(CORE_ID_2_MRU_ID(xCoreID), CFG_MRU_CHANNEL_SMP, CFG_MRU_MB_ID_SMP, CFG_MRU_MB_VAL_SMP);
#else
    TRIGGER_ISR_TO_CORE(xCoreID);
#endif
}
#endif

#ifdef MCAL_ENABLE_USER_MODE_SUPPORT
extern void SVCHandler_main(uint32_t *svc_args);
#endif

uint32_t svc_id = 0;
void vPortSVCHandler(void)
{
#if( configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY == 1 )
    /* Declaration when these variable are defined in code instead of being
     * exported from linker scripts. */
    extern uint32_t * __FreeRTOS_system_calls_start__;
    extern uint32_t * __FreeRTOS_system_calls_end__;
#endif /* #if( configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY == 1 ) */

    /* Load in R0 the stack pointer (depending on context from which SVC is called) */
    __asm volatile("tst lr, #4");
    __asm volatile("ite eq");
    __asm volatile("mrseq r0, MSP");
    __asm volatile("mrsne r0, PSP");             /* Load in R0 the stack pointer*/
#if defined (CPU_SJA1110)
    /* 32-bit instructions in Thumb mode */
    __asm volatile("ldr r0, [r0, #0x18]");       /* Get PC address (at offset 0x18) */
    __asm volatile("ldrb r0, [r0, #-2]");        /* Load the least significant byte (LSB) of the instruction at PC - 2 */
#else
    /* 16-bit instructions in Thumb mode */
    __asm volatile("add r0, r0, #0x18");         /* SP + 0x18  PC(r15) */
    __asm volatile("ldr r0, [r0]");              /* Get offset 0x18*/
    __asm volatile("sub r0, r0, #2");            /* = first two bytes, lsb, of the instruction which caused the SVC*/
    __asm volatile("ldr r0, [r0]");              /* Get the instruction of PC(r15)*/
#endif
    __asm volatile("and r1, r0, 0xFF");          /* Get SVC Id*/
    __asm volatile("ldr r2, =svc_id");           /* load svc_id*/
    __asm volatile("str r1, [r2]");              /* store SVC Id to svc_id*/

    switch (svc_id) {
    case portSVC_START_SCHEDULER:
#if( configENABLE_MPU == 1 )
        /* Setup the context of the first task so that the first task starts
         * executing. */
        prvRestoreContextOfFirstTask();
#else /* #if( configENABLE_MPU == 1 ) */
#if (configNUMBER_OF_CORES>1)
        flagCheckStartFirstTask[vPortGET_CORE_ID()] = 1;
#endif
        __asm volatile (
#if (configNUMBER_OF_CORES>1)
        " push {r2, r4}                     \n"
        " ldr  r2, =%[msmc_base]            \n" /* r2 <- MSMC_BASE */
        " ldr  r4, =%[cpxnum_offset]        \n" /* r4 <- CPXNUM_OFFSET */
        " add  r2, r2, r4                   \n" /* r2 <- MSMC_BASE + CPXNUM_OFFSET */
        " ldr  r4, [r2]                     \n" /* r4 <- *(MSMC_BASE + CPXNUM_OFFSET) */
        " ldr  r2, =%[core_m7_offset]       \n" /* r2 <- CORE_M7_OFFSET */
        " sub  r4, r4, r2                   \n" /* r4 <- CoreID = r4 - CORE_M7_OFFSET */
        " lsl  r4, r4, #2                   \n" /* CoreID = CoreID*4 */
        " ldr  r3, =pxCurrentTCBs           \n" /* Get current TCB */
        " add  r3, r4, r3                   \n" /* Current TCB [CoreID] */
        " pop  {r2, r4}                     \n"
#else
        " ldr  r3, =pxCurrentTCB            \n" /* Get current TCB */
#endif
        "   ldr r1, [r3]                    \n" /* Use pxCurrentTCBConst to get the pxCurrentTCB address. */
        "   ldr r0, [r1]                    \n" /* The first item in pxCurrentTCB is the task top of stack. */
        "   ldmia r0!, {r4-r11, r14}        \n" /* Pop the registers that are not automatically saved on exception entry and the critical nesting count. */
        "   msr psp, r0                     \n" /* Restore the task stack pointer. */
        "   isb                             \n"
        "   mov r0, #0                      \n"
        "   msr basepri, r0                 \n"
        "   bx r14                          \n"
#if (configNUMBER_OF_CORES>1)
        ::
        [msmc_base] "i" (MSMC_BASE),
        [cpxnum_offset] "i" (CPXNUM_OFFSET),
        [core_m7_offset] "i" (CORE_M7_OFFSET)
#endif
        );
#endif /* #if( configENABLE_MPU == 1 ) */
        break;

    case portSVC_YIELD:
        portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;

        /* Barriers are normally not required
         * but do ensure the code is completely
         * within the specified behavior for the
         * architecture. */
        __asm volatile ( "dsb" );
        __asm volatile ( "isb" );
        break;

#if ( configUSE_MPU_WRAPPERS_V1 == 1 )
#if ( configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY == 1 )
        case portSVC_RAISE_PRIVILEGE: /* Only raise the privilege, if the
                                       * svc was raised from any of the
                                       * system calls. */

            if( ( ulPC >= ( uint32_t ) __FreeRTOS_system_calls_start__ ) &&
                ( ulPC <= ( uint32_t ) __FreeRTOS_system_calls_end__ ) )
            {
                __asm volatile
                (
                        "   mrs r1, control     \n" /* Obtain current control value. */
                        "   bic r1, #1          \n" /* Set privilege bit. */
                        "   msr control, r1     \n" /* Write back new control value. */
                        ::: "r1", "memory"
                );
            }

            break;
    #else /* if ( configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY == 1 ) */
    case portSVC_RAISE_PRIVILEGE:
        __asm volatile
        (
                "   mrs r1, control     \n" /* Obtain current control value. */
                "   bic r1, #1          \n" /* Set privilege bit. */
                "   msr control, r1     \n" /* Write back new control value. */
                ::: "r1", "memory"
        );
        break;
#endif /* #if( configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY == 1 ) */
#endif /* #if ( configUSE_MPU_WRAPPERS_V1 == 1 ) */

    default:
#ifdef MCAL_ENABLE_USER_MODE_SUPPORT
        /* use SVCHandler_main for non FreeRTOS svc call ids */
        __asm volatile (
                "   tst lr, #4                  \n"
                "   ite eq                      \n"
                "   mrseq   r0, MSP             \n"
                "   mrsne   r0, PSP             \n"
                "   ldr r1,=SVCHandler_main     \n"
                "   bx r1                       \n"
        );
#else
        /* Incorrect SVC call. */
        configASSERT(pdFALSE)
        ;
#endif
        break;
    }
}
/*-----------------------------------------------------------*/
#if( configENABLE_MPU == 1 )
static void prvRestoreContextOfFirstTask( void )
{
    __asm volatile
    (
        " ldr r0, =0xE000ED08                   \n" /* Use the NVIC offset register to locate the stack. */
        " ldr r0, [r0]                          \n"
        " ldr r0, [r0]                          \n"
        " msr msp, r0                           \n" /* Set the msp back to the start of the stack. */
        "                                       \n"
        /*------------ Program MPU. ------------ */
        " ldr r3, =pxCurrentTCB                 \n" /* r3 = =pxCurrentTCB. */
        " ldr r2, [r3]                          \n" /* r2 = pxCurrentTCB. */
        " add r2, r2, #4                        \n" /* r2 = Second item in the TCB which is xMPUSettings. */
        "                                       \n"
        " dmb                                   \n" /* Complete outstanding transfers before disabling MPU. */
        " ldr r0, =0xe000ed94                   \n" /* MPU_CTRL register. */
        " ldr r3, [r0]                          \n" /* Read the value of MPU_CTRL. */
        " bic r3, #1                            \n" /* r3 = r3 & ~1 i.e. Clear the bit 0 in r3. */
        " dsb                                   \n" /* ARM recommendation: Ensure all memory operations complete */
        " isb                                   \n" /* ARM recommendation: Flush instruction pipeline before MPU changes */
        " str r3, [r0]                          \n" /* Disable MPU. */
        "                                       \n"
        " ldr r0, =0xe000ed9c                   \n" /* Load MPU Region Base Address register (MPU_RBAR) address into r0 */
        " mov r4, #0                            \n" /* Initialize region counter to 0 (strat with first region) */
        " 1:                                    \n" /* Local loop label for region configuration */
        " ldmia r2!, {r5-r6}                    \n" /* Load MPU region data from TCB: r5=RBAR, r6=RASR, increment r2 pointer */
        " stmia r0, {r5-r6}                     \n" /* Store region config to MPU hardware: write RBAR then RASR */
        " add r4, r4, #1                        \n" /* Increment region counter (move to next region) */
        " cmp r4, %[regions_in_tcb]             \n" /* Compare current region count with total regions to configure */
        " blt 1b                                \n" /* Branch back to loop start if more regions remain (r4 < total) */
        "                                       \n"
        " ldr r0, =0xe000ed94                   \n" /* MPU_CTRL register. */
        " ldr r3, [r0]                          \n" /* Read the value of MPU_CTRL. */
        " orr r3, #1                            \n" /* r3 = r3 | 1 i.e. Set the bit 0 in r3. */
        " str r3, [r0]                          \n" /* Enable MPU. */
        " dsb                                   \n" /* ARM recommendation: Ensure MPU enable takes effect */
        " isb                                   \n" /* ARM recommendation: Flush pipeline so instruction fetch sees new attributes immediately */
        "                                       \n"
        /*---------- Restore Context. ---------- */
        " ldr r3, =pxCurrentTCB                 \n" /* r3 = =pxCurrentTCB. */
        " ldr r2, [r3]                          \n" /* r2 = pxCurrentTCB. */
        " ldr r1, [r2]                          \n" /* r1 = Location of saved context in TCB. */
        "                                       \n"
        " ldmdb r1!, {r0, r4-r11}               \n" /* r0 contains PSP after the hardware had saved context. r4-r11 contain hardware saved context. */
        " msr psp, r0                           \n"
        " stmia r0, {r4-r11}                   \n" /* Copy the hardware saved context on the task stack. */
        " ldmdb r1!, {r3-r11, lr}               \n" /* r3 contains CONTROL register. r4-r11 and LR restored. */
        " msr control, r3                       \n"
        " str r1, [r2]                          \n" /* Save the location where the context should be saved next as the first member of TCB. */
        "                                       \n"
        " mov r0, #0                            \n"
        " msr basepri, r0                       \n"
        " bx lr                                 \n"
        "                                       \n"
        " .ltorg                                \n" /* Assemble current literal pool to avoid offset-out-of-bound errors with lto. */
        ::
        [regions_in_tcb] "i" (portTOTAL_NUM_REGIONS_IN_TCB)
    );
}
#endif
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
BaseType_t xPortStartScheduler( void )
{
    /* An application can install FreeRTOS interrupt handlers in one of the
     * following ways:
     * 1. Direct Routing - Install the functions vPortSVCHandler and
     *    xPortPendSVHandler for SVCall and PendSV interrupts respectively.
     * 2. Indirect Routing - Install separate handlers for SVCall and PendSV
     *    interrupts and route program control from those handlers to
     *    vPortSVCHandler and xPortPendSVHandler functions.
     *
     * Applications that use Indirect Routing must set
     * configCHECK_HANDLER_INSTALLATION to 0 in their FreeRTOSConfig.h. Direct
     * routing, which is validated here when configCHECK_HANDLER_INSTALLATION
     * is 1, should be preferred when possible. */
#if ( configCHECK_HANDLER_INSTALLATION == 1 )
    {
        const portISR_t *const pxVectorTable = portSCB_VTOR_REG;

        /* Validate that the application has correctly installed the FreeRTOS
         * handlers for SVCall and PendSV interrupts. We do not check the
         * installation of the SysTick handler because the application may
         * choose to drive the RTOS tick using a timer other than the SysTick
         * timer by overriding the weak function vPortSetupTimerInterrupt().
         *
         * Assertion failures here indicate incorrect installation of the
         * FreeRTOS handlers. For help installing the FreeRTOS handlers, see
         * https://www.FreeRTOS.org/FAQHelp.html.
         *
         * Systems with a configurable address for the interrupt vector table
         * can also encounter assertion failures or even system faults here if
         * VTOR is not set correctly to point to the application's vector table. */
        configASSERT(pxVectorTable[ portVECTOR_INDEX_SVC ] == vPortSVCHandler);
        configASSERT(pxVectorTable[ portVECTOR_INDEX_PENDSV ] == xPortPendSVHandler);
    }
#endif /* configCHECK_HANDLER_INSTALLATION */

#if ( configASSERT_DEFINED == 1 )
    {
        volatile uint8_t ucOriginalPriority;
        volatile uint32_t ulImplementedPrioBits = 0;
        volatile uint8_t *const pucFirstUserPriorityRegister = (volatile uint8_t* const ) ( portNVIC_IP_REGISTERS_OFFSET_16 + portFIRST_USER_INTERRUPT_NUMBER);
        volatile uint8_t ucMaxPriorityValue;

        /* Determine the maximum priority from which ISR safe FreeRTOS API
         * functions can be called.  ISR safe functions are those that end in
         * "FromISR".  FreeRTOS maintains separate thread and ISR API functions to
         * ensure interrupt entry is as fast and simple as possible.
         *
         * Save the interrupt priority value that is about to be clobbered. */
        ucOriginalPriority = *pucFirstUserPriorityRegister;

        /* Determine the number of priority bits available.  First write to all
         * possible bits. */
        *pucFirstUserPriorityRegister = portMAX_8_BIT_VALUE;

        /* Read the value back to see how many bits stuck. */
        ucMaxPriorityValue = *pucFirstUserPriorityRegister;

        /* Use the same mask on the maximum system call priority. */
        ucMaxSysCallPriority = configMAX_SYSCALL_INTERRUPT_PRIORITY & ucMaxPriorityValue;

        /* Check that the maximum system call priority is nonzero after
         * accounting for the number of priority bits supported by the
         * hardware. A priority of 0 is invalid because setting the BASEPRI
         * register to 0 unmasks all interrupts, and interrupts with priority 0
         * cannot be masked using BASEPRI.
         * See https://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html */
        configASSERT(ucMaxSysCallPriority);

        /* Check that the bits not implemented in hardware are zero in
         * configMAX_SYSCALL_INTERRUPT_PRIORITY. */
        configASSERT(( configMAX_SYSCALL_INTERRUPT_PRIORITY & ( ~ucMaxPriorityValue ) ) == 0U);

        /* Calculate the maximum acceptable priority group value for the number
         * of bits read back. */

        while( ( ucMaxPriorityValue & portTOP_BIT_OF_BYTE ) == portTOP_BIT_OF_BYTE )
        {
            ulImplementedPrioBits++;
            ucMaxPriorityValue <<= (uint8_t) 0x01;
        }

        if( ulImplementedPrioBits == 8 )
        {
            /* When the hardware implements 8 priority bits, there is no way for
             * the software to configure PRIGROUP to not have sub-priorities. As
             * a result, the least significant bit is always used for sub-priority
             * and there are 128 preemption priorities and 2 sub-priorities.
             *
             * This may cause some confusion in some cases - for example, if
             * configMAX_SYSCALL_INTERRUPT_PRIORITY is set to 5, both 5 and 4
             * priority interrupts will be masked in Critical Sections as those
             * are at the same preemption priority. This may appear confusing as
             * 4 is higher (numerically lower) priority than
             * configMAX_SYSCALL_INTERRUPT_PRIORITY and therefore, should not
             * have been masked. Instead, if we set configMAX_SYSCALL_INTERRUPT_PRIORITY
             * to 4, this confusion does not happen and the behaviour remains the same.
             *
             * The following assert ensures that the sub-priority bit in the
             * configMAX_SYSCALL_INTERRUPT_PRIORITY is clear to avoid the above mentioned
             * confusion. */
            configASSERT(( configMAX_SYSCALL_INTERRUPT_PRIORITY & 0x1U ) == 0U);
            ulMaxPRIGROUPValue = 0;
        }
        else
        {
            ulMaxPRIGROUPValue = portMAX_PRIGROUP_BITS - ulImplementedPrioBits;
        }

        /* Shift the priority group value back to its position within the AIRCR
         * register. */
        ulMaxPRIGROUPValue <<= portPRIGROUP_SHIFT;
        ulMaxPRIGROUPValue &= portPRIORITY_GROUP_MASK;

        /* Restore the clobbered interrupt priority register to its original
         * value. */
        *pucFirstUserPriorityRegister = ucOriginalPriority;
    }
#endif /* configASSERT_DEFINED */

    /* Make PendSV and SysTick the lowest priority interrupts, and make SVCall
     * the highest priority. */
    portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
    portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;
    portNVIC_SHPR2_REG = 0;

#if( configENABLE_MPU == 1 )
    /* Configure the regions in the MPU that are common to all tasks. */
    prvSetupMPU();
#endif /* configENABLE_MPU == 1 */

#if (configNUMBER_OF_CORES>1)
    if(vPortGET_CORE_ID()== configCORE_MASTER)
    {
        /* Start the timer that generates the tick ISR.  Interrupts are disabled
         * here already. */
        vPortSetupTimerInterrupt();
    }
    else
    {
#ifdef MRU_SMP_USE
        uint32_t coreId = vPortGET_CORE_ID();

        /* Register MRU to signal to cores */
        MRUInit(CORE_ID_2_MRU_ID(coreId), CFG_MRU_CHANNEL_SMP, CFG_MRU_MB_ID_SMP, CFG_MRU_INTERRUPT_SMP);
        MRURegisterMBCB(CFG_MRU_INTERRUPT_SMP, (void*)xCoreSyncSignalHandler);
#else
        /* Setup interrtupt for other cores */
        /* Init core to core interrupt */
        Core_registerIsrHandler(INT_ID, xCoreSyncSignalHandler);
        NVIC_SET_PRIORITY (INT_ID, 0);
        NVIC_ENABLE_IRQ (INT_ID);
#endif
    }

    /* Wait until semaphore is available */
    while(sema_lock(portRTOS_SEMA_GATE_SYNC_CORE) != 0);

    /* Synchronize with core master */
    sync_flags = sync_flags | (1 << vPortGET_CORE_ID());
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("dmb sy" ::: "memory");
    __asm__ volatile("isb");

    sema_unlock(portRTOS_SEMA_GATE_SYNC_CORE);

    /*Wait all core for synchronization point When all reach this point all are release at the same time, including core 0.*/
    while (sync_flags != RUNNING_CPUS_MASK)
    {
        ;
    }

#else
    /* Start the timer that generates the tick ISR.  Interrupts are disabled
     * here already. */
    vPortSetupTimerInterrupt();
#endif

    /* Initialise the critical nesting count ready for the first task. */
    uxCriticalNesting[portGET_CORE_ID()] = 0;

    /* Ensure the VFP is enabled - it should be anyway. */
    vPortEnableVFP();

    /* Lazy save always. */
    *( portFPCCR) |= portASPEN_AND_LSPEN_BITS;

    /* Start the first task. */
    prvPortStartFirstTask();

    /* Should never get here as the tasks will now be executing!  Call the task
     * exit error function to prevent compiler warnings about a static function
     * not being called in the case that the application writer overrides this
     * functionality by defining configTASK_RETURN_ADDRESS.  Call
     * vTaskSwitchContext() so link time optimisation does not remove the
     * symbol. */
    #if(configNUMBER_OF_CORES>1)
    {
        vTaskSwitchContext(vPortGET_CORE_ID());
    }
    #else
    {
        vTaskSwitchContext();
    }
    #endif

    prvTaskExitError();

    /* Should not get here! */
    return 0;
}
/*-----------------------------------------------------------*/
void prvPortStartFirstTask(void)
{
    /* Start the first task.  This also clears the bit that indicates the FPU is
     * in use in case the FPU was used before the scheduler was started - which
     * would otherwise result in the unnecessary leaving of space in the SVC stack
     * for lazy saving of FPU registers. */
    __asm volatile
    (
            " ldr r0, =0xE000ED08   \n" /* Use the NVIC offset register to locate the stack. */
            " ldr r0, [ r0 ]        \n"
            " ldr r0, [ r0 ]        \n"
            "                       \n"
            " msr msp, r0           \n" /* Set the msp back to the start of the stack. */
            "                       \n"
            " mov r0, #0            \n" /* Clear the bit that indicates the FPU is in use, see comment above. */
            " msr control, r0       \n"
            "                       \n"
            " cpsie i               \n"
            " cpsie f               \n"
            " dsb                   \n"
            " isb                   \n"
            " svc %0                \n" /* System call to start first task. */
            " nop                   \n"
            " nop                   \n"
            :: "i" (portSVC_START_SCHEDULER) : "memory" );
}
/*-----------------------------------------------------------*/
void vPortEndScheduler(void) {
    /* Not implemented in ports where there is nothing to return to.
     * Artificially force an assert. */
    configASSERT(uxCriticalNesting[ portGET_CORE_ID() ] == 1000UL);
}
/*-----------------------------------------------------------*/
#if( configENABLE_MPU == 1 )
void vPortEnterCritical(void) {
    BaseType_t coreID = portGET_CORE_ID();

#if ( configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS == 1 )
    if ( portIS_PRIVILEGED() == pdFALSE) {
        portRAISE_PRIVILEGE();
        portMEMORY_BARRIER();

        portDISABLE_INTERRUPTS();
        uxCriticalNesting[coreID]++;
        portMEMORY_BARRIER();

        portRESET_PRIVILEGE();
        portMEMORY_BARRIER();
    } else {
        portDISABLE_INTERRUPTS();
        uxCriticalNesting[coreID]++;
    }
#else /* if ( configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS == 1 ) */
    portDISABLE_INTERRUPTS();
    uxCriticalNesting[ coreID ] ++;
#endif /* if ( configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS == 1 ) */
}
#else /* #if( configENABLE_MPU == 1 ) */
void vPortEnterCritical(void) {
    BaseType_t coreID = portGET_CORE_ID();
    portDISABLE_INTERRUPTS();
    uxCriticalNesting[ coreID ] ++;

    /* This is not the interrupt safe version of the enter critical function so
     * assert() if it is being called from an interrupt context.  Only API
     * functions that end in "FromISR" can be used in an interrupt.  Only assert if
     * the critical nesting count is 1 to protect against recursive calls if the
     * assert function also uses a critical section. */
    if( uxCriticalNesting[ coreID ]  == 1 )
    {
        configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );
    }
}
#endif /* #if( configENABLE_MPU == 1 ) */
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
    BaseType_t coreID = portGET_CORE_ID();
    configASSERT(uxCriticalNesting[coreID]);
    uxCriticalNesting[coreID]--;
    if( uxCriticalNesting[coreID]  == 0 )
    {
        portENABLE_INTERRUPTS();
    }
}
/*-----------------------------------------------------------*/
void xPortPendSVHandler( void )
{
#if( configENABLE_MPU == 1 )
#if(configNUMBER_OF_CORES>1)
    if ( flagCheckStartFirstTask[ vPortGET_CORE_ID() ] == 1) /* Wait for cores other than core master go to first task to psp is stored*/
    {
#endif
    __asm volatile
    (
#if (configNUMBER_OF_CORES>1)
            " push {r2, r4}                         \n"
            " ldr  r2, =%[msmc_base]                \n" /* r2 <- MSMC_BASE */
            " ldr  r4, =%[cpxnum_offset]            \n" /* r4 <- CPXNUM_OFFSET */
            " add  r2, r2, r4                       \n" /* r2 <- MSMC_BASE + CPXNUM_OFFSET */
            " ldr  r4, [r2]                         \n" /* r4 <- *(MSMC_BASE + CPXNUM_OFFSET) */
            " ldr  r2, =%[core_m7_offset]           \n" /* r2 <- CORE_M7_OFFSET */
            " sub  r4, r4, r2                       \n" /* r4 <- CoreID = r4 - CORE_M7_OFFSET */
            " lsl  r4, r4, #2                       \n" /* CoreID = CoreID*4 */
            " ldr  r3, =pxCurrentTCBs               \n" /* Get current TCB */
            " add  r3, r4, r3                       \n" /* Current TCB [CoreID] */
            " pop  {r2, r4}                         \n"
#else
            " ldr  r3, =pxCurrentTCB                \n" /* Get current TCB */
#endif
            "   ldr r2, [r3]                        \n" /*  ldr r2, [r3]  r2 = pxCurrentTCB. */
            "   ldr r1, [r2]                        \n" /* r1 = Location where the context should be saved. */
            "                                       \n"
            "   mrs r3, control                     \n" /*------------ Save Context. ----------- */
            "   mrs r0, psp                         \n"
            "   isb                                 \n"
            "                                       \n"
            "   add r0, r0, #0x20                   \n" /* Move r0 to location where s0 is saved. */
            "   tst lr, #0x10                       \n"
            "   ittt eq                             \n"
            "   vstmiaeq r1!, {s16-s31}             \n" /* Store s16-s31. */
            "   vldmiaeq r0, {s0-s16}               \n" /* Copy hardware saved FP context into s0-s16. */
            "   vstmiaeq r1!, {s0-s16}              \n" /* Store hardware saved FP context. */
            "   sub r0, r0, #0x20                   \n" /* Set r0 back to the location of hardware saved context. */
            "                                       \n"
            "   stmia r1!, {r3-r11, lr}             \n" /* Store CONTROL register, r4-r11 and LR. */
            "   ldmia r0, {r4-r11}                  \n" /* Copy hardware saved context into r4-r11. */
            "   stmia r1!, {r0, r4-r11}             \n" /* Store original PSP (after hardware has saved context) and the hardware saved context. */
            "   str r1, [r2]                        \n" /* Save the location from where the context should be restored as the first member of TCB. */
            "                                       \n" /*---------- Select next task. --------- */
            "   mov r0, %0                          \n"
            "   cpsid i                             \n"/* ARM Cortex-M7 r0p1 Errata 837070 workaround. */
            "   msr basepri, r0                     \n"
            "   dsb                                 \n"
            "   isb                                 \n"
            "   cpsie i                             \n"/* ARM Cortex-M7 r0p1 Errata 837070 workaround. */
#if (configNUMBER_OF_CORES>1)
            " ldr  r2, =%[msmc_base]                \n" /* r2 <- MSMC_BASE */
            " ldr  r0, =%[cpxnum_offset]            \n" /* r0 <- CPXNUM_OFFSET */
            " add  r2, r2, r0                       \n" /* r2 <- MSMC_BASE + CPXNUM_OFFSET */
            " ldr  r0, [r2]                         \n" /* r0 <- *(MSMC_BASE + CPXNUM_OFFSET) */
            " ldr  r2, =%[core_m7_offset]           \n" /* r2 <- CORE_M7_OFFSET */
            " sub  r0, r0, r2                       \n" /* r0 <- CoreID = r0 - CORE_M7_OFFSET */
#endif
            "   bl vTaskSwitchContext               \n"
            "   mov r0, #0                          \n"
            "   msr basepri, r0                     \n"
            "                                       \n" /*------------ Program MPU. ------------ */
            "   ldr r3, =pxCurrentTCB               \n" /* r3 = &( pxCurrentTCB ). */
            "   ldr r2, [r3]                        \n" /* r2 = pxCurrentTCB. */
            "   add r2, r2, #4                      \n" /* r2 = Second item in the TCB which is xMPUSettings. */
            "                                       \n"
            "   dmb                                 \n" /* Complete outstanding transfers before disabling MPU. */
            "   ldr r0, =0xe000ed94                 \n" /* MPU_CTRL register. */
            "   ldr r3, [r0]                        \n" /* Read the value of MPU_CTRL. */
            "   bic r3, #1                          \n" /* r3 = r3 & ~1 i.e. Clear the bit 0 in r3. */
            "   dsb                                 \n" /* ARM recommendation: Ensure all memory operations complete */
            "   isb                                 \n" /* ARM recommendation: Flush instruction pipeline before MPU changes */
            "   str r3, [r0]                        \n" /* Disable MPU. */
            "                                       \n"
            "   ldr r0, =0xe000ed9c                 \n" /* Load MPU Region Base Address register (MPU_RBAR) address into r0 */
            "   mov r4, #0                          \n" /* Initialize region counter to 0 (strat with first region) */
            "   1:                                  \n" /* Local loop label for region configuration */
            "   ldmia r2!, {r5-r6}                  \n" /* Load MPU region data from TCB: r5=RBAR, r6=RASR, increment r2 pointer */
            "   stmia r0, {r5-r6}                   \n" /* Store region config to MPU hardware: write RBAR then RASR */
            "   add r4, r4, #1                      \n" /* Increment region counter (move to next region) */
            "   cmp r4, %[regions_in_tcb]           \n" /* Compare current region count with total regions to configure */
            "   blt 1b                              \n" /* Branch back to loop start if more regions remain (r4 < total) */
            "                                       \n"
            "   ldr r0, =0xe000ed94                 \n" /* MPU_CTRL register. */
            "   ldr r3, [r0]                        \n" /* Read the value of MPU_CTRL. */
            "   orr r3, #1                          \n" /* r3 = r3 | 1 i.e. Set the bit 0 in r3. */
            "   str r3, [r0]                        \n" /* Enable MPU. */
            "   dsb                                 \n" /* ARM recommendation: Ensure MPU enable takes effect */
            "   isb                                 \n" /* ARM recommendation: Flush pipeline so instruction fetch sees new attributes immediately */
            "                                       \n"  /*---------- Restore Context. ---------- */
            "   ldr r3, =pxCurrentTCB               \n" /* r3 = &( pxCurrentTCB ). */
            "   ldr r2, [r3]                        \n" /* r2 = pxCurrentTCB. */
            "   ldr r1, [r2]                        \n" /* r1 = Location of saved context in TCB. */
            "                                       \n"
            "   ldmdb r1!, {r0, r4-r11}             \n" /* r0 contains PSP after the hardware had saved context. r4-r11 contain hardware saved context. */
            "   msr psp, r0                         \n"
            "   stmia r0!, {r4-r11}                 \n" /* Copy the hardware saved context on the task stack. */
            "   ldmdb r1!, {r3-r11, lr}             \n" /* r3 contains CONTROL register. r4-r11 and LR restored. */
            "   msr control, r3                     \n"
            "                                       \n"
            "   tst lr, #0x10                       \n"
            "   ittt eq                             \n"
            "   vldmdbeq r1!, {s0-s16}              \n" /* s0-s16 contain hardware saved FP context. */
            "   vstmiaeq r0!, {s0-s16}              \n" /* Copy hardware saved FP context on the task stack. */
            "   vldmdbeq r1!, {s16-s31}             \n" /* Restore s16-s31. */
            "                                       \n"
#ifdef WORKAROUND_PMU_CM001 /* XMC4000 specific errata workaround. */
    #if WORKAROUND_PMU_CM001 == 1
            "           push { r14 }                \n"
            "           pop { pc }                  \n"
    #endif
#endif
            "   str r1, [r2]                        \n" /* Save the location where the context should be saved next as the first member of TCB. */
            "   bx lr                               \n"
            ::
            [regions_in_tcb] "i" (portTOTAL_NUM_REGIONS_IN_TCB),
#if (configNUMBER_OF_CORES>1)
            [msmc_base] "i" (MSMC_BASE),
            [cpxnum_offset] "i" (CPXNUM_OFFSET),
            [core_m7_offset] "i" (CORE_M7_OFFSET),
#endif
            [syscall_interrupt_priority] "i" (configMAX_SYSCALL_INTERRUPT_PRIORITY)
    );
#if(configNUMBER_OF_CORES > 1)
    }
    else
    {
        __asm volatile(" bx lr                      \n");
    }
#endif

#else /* #if( configENABLE_MPU == 1 ) */
#if(configNUMBER_OF_CORES>1)
    if ( flagCheckStartFirstTask[ vPortGET_CORE_ID() ] == 1) /* Wait for cores other than core master go to first task to psp is stored*/
    {
#endif
    __asm volatile
    (
        "   mrs r0, psp                         \n"
        "   isb                                 \n"
#if (configNUMBER_OF_CORES>1)
        " push {r2, r4}                         \n"
        " ldr  r2, =%[msmc_base]                \n" /* r2 <- MSMC_BASE */
        " ldr  r4, =%[cpxnum_offset]            \n" /* r4 <- CPXNUM_OFFSET */
        " add  r2, r2, r4                       \n" /* r2 <- MSMC_BASE + CPXNUM_OFFSET */
        " ldr  r4, [r2]                         \n" /* r4 <- *(MSMC_BASE + CPXNUM_OFFSET) */
        " ldr  r2, =%[core_m7_offset]           \n" /* r2 <- CORE_M7_OFFSET */
        " sub  r4, r4, r2                       \n" /* r4 <- CoreID = r4 - CORE_M7_OFFSET */
        " lsl  r4, r4, #2                       \n" /* CoreID = CoreID*4 */
        " ldr  r3, =pxCurrentTCBs               \n" /* Get current TCB */
        " add  r3, r4, r3                       \n" /* Current TCB [CoreID] */
        " pop  {r2, r4}                         \n"
#else
        " ldr  r3, =pxCurrentTCB                         \t\n" /* Get current TCB */
#endif
        "   ldr r2, [r3]                        \n"
        "                                       \n"
        "   tst r14, #0x10                      \n" /* Is the task using the FPU context?  If so, push high vfp registers. */
        "   it eq                               \n"
        "   vstmdbeq r0!, {s16-s31}             \n"
        "                                       \n"
        "   stmdb r0!, {r4-r11, r14}            \n" /* Save the core registers. */
        "   str r0, [r2]                        \n" /* Save the new top of stack into the first member of the TCB. */
        "                                       \n"
        "   stmdb sp!, {r0, r3}                 \n"
        "   mov r0, %[syscall_interrupt_priority]\n"
        "   cpsid i                             \n" /* ARM Cortex-M7 r0p1 Errata 837070 workaround. */
        "   msr basepri, r0                     \n"
        "   dsb                                 \n"
        "   isb                                 \n"
        "   cpsie i                             \n" /* ARM Cortex-M7 r0p1 Errata 837070 workaround. */
#if (configNUMBER_OF_CORES>1)
        " ldr  r2, =%[msmc_base]                \n" /* r2 <- MSMC_BASE */
        " ldr  r0, =%[cpxnum_offset]            \n" /* r0 <- CPXNUM_OFFSET */
        " add  r2, r2, r0                       \n" /* r2 <- MSMC_BASE + CPXNUM_OFFSET */
        " ldr  r0, [r2]                         \n" /* r0 <- *(MSMC_BASE + CPXNUM_OFFSET) */
        " ldr  r2, =%[core_m7_offset]           \n" /* r2 <- CORE_M7_OFFSET */
        " sub  r0, r0, r2                       \n" /* r0 <- CoreID = r0 - CORE_M7_OFFSET */
#endif
        "   bl vTaskSwitchContext               \n"
        "   mov r0, #0                          \n"
        "   msr basepri, r0                     \n"
        "   ldmia sp!, {r0, r3}                 \n"
        "                                       \n"
        "   ldr r1, [r3]                        \n" /* The first item in pxCurrentTCB is the task top of stack. */
        "   ldr r0, [r1]                        \n"
        "                                       \n"
        "   ldmia r0!, {r4-r11, r14}            \n" /* Pop the core registers. */
        "                                       \n"
        "   tst r14, #0x10                      \n" /* Is the task using the FPU context?  If so, pop the high vfp registers too. */
        "   it eq                               \n"
        "   vldmiaeq r0!, {s16-s31}             \n"
        "                                       \n"
        "   msr psp, r0                         \n"
        "   isb                                 \n"
        "                                       \n"
        #ifdef WORKAROUND_PMU_CM001 /* XMC4000 specific errata workaround. */
            #if WORKAROUND_PMU_CM001 == 1
                "           push { r14 }                \n"
                "           pop { pc }                  \n"
            #endif
        #endif
        "                                       \n"
        "   bx r14                              \n"
        "                                       \n"
        ::
#if (configNUMBER_OF_CORES>1)
        [msmc_base] "i" (MSMC_BASE),
        [cpxnum_offset] "i" (CPXNUM_OFFSET),
        [core_m7_offset] "i" (CORE_M7_OFFSET),
#endif
        [syscall_interrupt_priority] "i" (configMAX_SYSCALL_INTERRUPT_PRIORITY)
    );
#if(configNUMBER_OF_CORES>1)
    }
    else
    {
        __asm volatile(" bx lr                              \t\n");
    }
#endif
#endif  /* #if( configENABLE_MPU == 1 ) */
}
/*-----------------------------------------------------------*/

void xPortSysTickHandler( void )
{
    /* The SysTick runs at the lowest interrupt priority, so when this interrupt
     * executes all interrupts must be unmasked.  There is therefore no need to
     * save and then restore the interrupt mask value as its value is already
     * known. */
    uint32_t ulPreviousMask;

    ulPreviousMask = taskENTER_CRITICAL_FROM_ISR();
    traceISR_ENTER();
    {
        /* Increment the RTOS tick. */
        if( xTaskIncrementTick() != pdFALSE )
        {
            traceISR_EXIT_TO_SCHEDULER();

            /* A context switch is required.  Context switching is performed in
             * the PendSV interrupt.  Pend the PendSV interrupt. */
            portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
        }
        else
        {
            traceISR_EXIT();
        }
    }
    taskEXIT_CRITICAL_FROM_ISR(ulPreviousMask);
}
/*-----------------------------------------------------------*/

#if ( configUSE_TICKLESS_IDLE == 1 )

    __attribute__( ( weak ) ) void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
    {
        uint32_t ulReloadValue, ulCompleteTickPeriods, ulCompletedSysTickDecrements, ulSysTickDecrementsLeft;
        TickType_t xModifiableIdleTime;

        /* Make sure the SysTick reload value does not overflow the counter. */
        if( xExpectedIdleTime > xMaximumPossibleSuppressedTicks )
        {
            xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
        }

        /* Enter a critical section but don't use the taskENTER_CRITICAL()
         * method as that will mask interrupts that should exit sleep mode. */
        __asm volatile ( "cpsid i" ::: "memory" );
        __asm volatile ( "dsb" );
        __asm volatile ( "isb" );

        /* If a context switch is pending or a task is waiting for the scheduler
         * to be unsuspended then abandon the low power entry. */
        if( eTaskConfirmSleepModeStatus() == eAbortSleep )
        {
            /* Re-enable interrupts - see comments above the cpsid instruction
             * above. */
            __asm volatile ( "cpsie i" ::: "memory" );
        }
        else
        {
            /* Stop the SysTick momentarily.  The time the SysTick is stopped for
             * is accounted for as best it can be, but using the tickless mode will
             * inevitably result in some tiny drift of the time maintained by the
             * kernel with respect to calendar time. */
            portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT );

            /* Use the SysTick current-value register to determine the number of
             * SysTick decrements remaining until the next tick interrupt.  If the
             * current-value register is zero, then there are actually
             * ulTimerCountsForOneTick decrements remaining, not zero, because the
             * SysTick requests the interrupt when decrementing from 1 to 0. */
            ulSysTickDecrementsLeft = portNVIC_SYSTICK_CURRENT_VALUE_REG;

            if( ulSysTickDecrementsLeft == 0 )
            {
                ulSysTickDecrementsLeft = ulTimerCountsForOneTick;
            }

            /* Calculate the reload value required to wait xExpectedIdleTime
             * tick periods.  -1 is used because this code normally executes part
             * way through the first tick period.  But if the SysTick IRQ is now
             * pending, then clear the IRQ, suppressing the first tick, and correct
             * the reload value to reflect that the second tick period is already
             * underway.  The expected idle time is always at least two ticks. */
            ulReloadValue = ulSysTickDecrementsLeft + ( ulTimerCountsForOneTick * ( xExpectedIdleTime - 1UL ) );

            if( ( portNVIC_INT_CTRL_REG & portNVIC_PEND_SYSTICK_SET_BIT ) != 0 )
            {
                portNVIC_INT_CTRL_REG = portNVIC_PEND_SYSTICK_CLEAR_BIT;
                ulReloadValue -= ulTimerCountsForOneTick;
            }

            if( ulReloadValue > ulStoppedTimerCompensation )
            {
                ulReloadValue -= ulStoppedTimerCompensation;
            }

            /* Set the new reload value. */
            portNVIC_SYSTICK_LOAD_REG = ulReloadValue;

            /* Clear the SysTick count flag and set the count value back to
             * zero. */
            portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;

            /* Restart SysTick. */
            portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;

            /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
             * set its parameter to 0 to indicate that its implementation contains
             * its own wait for interrupt or wait for event instruction, and so wfi
             * should not be executed again.  However, the original expected idle
             * time variable must remain unmodified, so a copy is taken. */
            xModifiableIdleTime = xExpectedIdleTime;
            configPRE_SLEEP_PROCESSING( xModifiableIdleTime );

            if( xModifiableIdleTime > 0 )
            {
                __asm volatile ( "dsb" ::: "memory" );
                __asm volatile ( "wfi" );
                __asm volatile ( "isb" );
            }

            configPOST_SLEEP_PROCESSING( xExpectedIdleTime );

            /* Re-enable interrupts to allow the interrupt that brought the MCU
             * out of sleep mode to execute immediately.  See comments above
             * the cpsid instruction above. */
            __asm volatile ( "cpsie i" ::: "memory" );
            __asm volatile ( "dsb" );
            __asm volatile ( "isb" );

            /* Disable interrupts again because the clock is about to be stopped
             * and interrupts that execute while the clock is stopped will increase
             * any slippage between the time maintained by the RTOS and calendar
             * time. */
            __asm volatile ( "cpsid i" ::: "memory" );
            __asm volatile ( "dsb" );
            __asm volatile ( "isb" );

            /* Disable the SysTick clock without reading the
             * portNVIC_SYSTICK_CTRL_REG register to ensure the
             * portNVIC_SYSTICK_COUNT_FLAG_BIT is not cleared if it is set.  Again,
             * the time the SysTick is stopped for is accounted for as best it can
             * be, but using the tickless mode will inevitably result in some tiny
             * drift of the time maintained by the kernel with respect to calendar
             * time*/
            portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT );

            /* Determine whether the SysTick has already counted to zero. */
            if( ( portNVIC_SYSTICK_CTRL_REG & portNVIC_SYSTICK_COUNT_FLAG_BIT ) != 0 )
            {
                uint32_t ulCalculatedLoadValue;

                /* The tick interrupt ended the sleep (or is now pending), and
                 * a new tick period has started.  Reset portNVIC_SYSTICK_LOAD_REG
                 * with whatever remains of the new tick period. */
                ulCalculatedLoadValue = ( ulTimerCountsForOneTick - 1UL ) - ( ulReloadValue - portNVIC_SYSTICK_CURRENT_VALUE_REG );

                /* Don't allow a tiny value, or values that have somehow
                 * underflowed because the post sleep hook did something
                 * that took too long or because the SysTick current-value register
                 * is zero. */
                if( ( ulCalculatedLoadValue <= ulStoppedTimerCompensation ) || ( ulCalculatedLoadValue > ulTimerCountsForOneTick ) )
                {
                    ulCalculatedLoadValue = ( ulTimerCountsForOneTick - 1UL );
                }

                portNVIC_SYSTICK_LOAD_REG = ulCalculatedLoadValue;

                /* As the pending tick will be processed as soon as this
                 * function exits, the tick value maintained by the tick is stepped
                 * forward by one less than the time spent waiting. */
                ulCompleteTickPeriods = xExpectedIdleTime - 1UL;
            }
            else
            {
                /* Something other than the tick interrupt ended the sleep. */

                /* Use the SysTick current-value register to determine the
                 * number of SysTick decrements remaining until the expected idle
                 * time would have ended. */
                ulSysTickDecrementsLeft = portNVIC_SYSTICK_CURRENT_VALUE_REG;
                #if ( portNVIC_SYSTICK_CLK_BIT_CONFIG != portNVIC_SYSTICK_CLK_BIT )
                {
                    /* If the SysTick is not using the core clock, the current-
                     * value register might still be zero here.  In that case, the
                     * SysTick didn't load from the reload register, and there are
                     * ulReloadValue decrements remaining in the expected idle
                     * time, not zero. */
                    if( ulSysTickDecrementsLeft == 0 )
                    {
                        ulSysTickDecrementsLeft = ulReloadValue;
                    }
                }
                #endif /* portNVIC_SYSTICK_CLK_BIT_CONFIG */

                /* Work out how long the sleep lasted rounded to complete tick
                 * periods (not the ulReload value which accounted for part
                 * ticks). */
                ulCompletedSysTickDecrements = ( xExpectedIdleTime * ulTimerCountsForOneTick ) - ulSysTickDecrementsLeft;

                /* How many complete tick periods passed while the processor
                 * was waiting? */
                ulCompleteTickPeriods = ulCompletedSysTickDecrements / ulTimerCountsForOneTick;

                /* The reload value is set to whatever fraction of a single tick
                 * period remains. */
                portNVIC_SYSTICK_LOAD_REG = ( ( ulCompleteTickPeriods + 1UL ) * ulTimerCountsForOneTick ) - ulCompletedSysTickDecrements;
            }

            /* Restart SysTick so it runs from portNVIC_SYSTICK_LOAD_REG again,
             * then set portNVIC_SYSTICK_LOAD_REG back to its standard value.  If
             * the SysTick is not using the core clock, temporarily configure it to
             * use the core clock.  This configuration forces the SysTick to load
             * from portNVIC_SYSTICK_LOAD_REG immediately instead of at the next
             * cycle of the other clock.  Then portNVIC_SYSTICK_LOAD_REG is ready
             * to receive the standard value immediately. */
            portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
            portNVIC_SYSTICK_CTRL_REG = portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT;
            #if ( portNVIC_SYSTICK_CLK_BIT_CONFIG == portNVIC_SYSTICK_CLK_BIT )
            {
                portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
            }
            #else
            {
                /* The temporary usage of the core clock has served its purpose,
                 * as described above.  Resume usage of the other clock. */
                portNVIC_SYSTICK_CTRL_REG = portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT;

                if( ( portNVIC_SYSTICK_CTRL_REG & portNVIC_SYSTICK_COUNT_FLAG_BIT ) != 0 )
                {
                    /* The partial tick period already ended.  Be sure the SysTick
                     * counts it only once. */
                    portNVIC_SYSTICK_CURRENT_VALUE_REG = 0;
                }

                portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
                portNVIC_SYSTICK_CTRL_REG = portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT;
            }
            #endif /* portNVIC_SYSTICK_CLK_BIT_CONFIG */

            /* Step the tick to account for any tick periods that elapsed. */
            vTaskStepTick( ulCompleteTickPeriods );

            /* Exit with interrupts enabled. */
            __asm volatile ( "cpsie i" ::: "memory" );
        }
    }

#endif /* #if configUSE_TICKLESS_IDLE */
/*-----------------------------------------------------------*/

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
__attribute__( ( weak ) ) void vPortSetupTimerInterrupt( void )
{
    /* Calculate the constants required to configure the tick interrupt. */
#if ( configUSE_TICKLESS_IDLE == 1 )
    {
        ulTimerCountsForOneTick = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ );
        xMaximumPossibleSuppressedTicks = portMAX_24_BIT_NUMBER / ulTimerCountsForOneTick;
        ulStoppedTimerCompensation = portMISSED_COUNTS_FACTOR / ( configCPU_CLOCK_HZ / configSYSTICK_CLOCK_HZ );
    }
    #endif /* configUSE_TICKLESS_IDLE */

    /* Stop and clear the SysTick. */
    portNVIC_SYSTICK_CTRL_REG = 0UL;
    portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;

    /* Configure SysTick to interrupt at the requested rate. */
    portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;
    portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT_CONFIG | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT);
}
/*-----------------------------------------------------------*/

#if (configNUMBER_OF_CORES>1)
void xCoreSyncSignalHandler(void)
{
    __asm volatile("push    {r3,r14}                           \t\n");
    /* The SysTick runs at the lowest interrupt priority, so when this interrupt
     * executes all interrupts must be unmasked.  There is therefore no need to
     * save and then restore the interrupt mask value as its value is already
     * known. */
    portDISABLE_INTERRUPTS();

    /* A context switch is required.  Context switching is performed in
     * the PendSV interrupt.  Pend the PendSV interrupt. */
    portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;

    portENABLE_INTERRUPTS();
#ifndef MRU_SMP_USE
    /* Clear pending core to core interrupt*/
    CLEAR_ISR_CORE_TO_CORE(vPortGET_CORE_ID(), configCORE_MASTER);
#endif
    __asm volatile(" pop {r3, pc}                              \t\n");
}

#endif

/* This is a naked function. */
static void vPortEnableVFP( void )
{
    __asm volatile
    (
            "   ldr.w r0, =0xE000ED88       \n" /* The FPU enable bits are in the CPACR. */
            "   ldr r1, [r0]                \n"
            "                               \n"
            "   orr r1, r1, #( 0xf << 20 )  \n" /* Enable CP10 and CP11 coprocessors, then save back. */
            "   str r1, [r0]                \n"
            "   bx r14                      \n"
            "   .ltorg                      \n"
    );
}
/*-----------------------------------------------------------*/
#if( configENABLE_MPU == 1 )
void prvSetupMPU(void) {
    extern uint32_t __FreeRTOS_privileged_functions_start__[];
    extern uint32_t __FreeRTOS_privileged_functions_end__[];
    extern uint32_t __FreeRTOS_system_calls_start__[];
    extern uint32_t __FreeRTOS_system_calls_end__[];
    extern uint32_t __FreeRTOS_privileged_data_start__[];
    extern uint32_t __FreeRTOS_privileged_data_end__[];
    extern uint32_t __FreeRTOS_code_data_start__[];
    extern uint32_t __FreeRTOS_code_data_end__[];

    /* The only permitted number of regions are 8 or 16. */
    configASSERT( ( configTOTAL_MPU_REGIONS == 8 ) || ( configTOTAL_MPU_REGIONS == 16 ) );

    /* Ensure that the configTOTAL_MPU_REGIONS is configured correctly. */
    configASSERT( portMPU_TYPE_REG == portEXPECTED_MPU_TYPE_VALUE );

    /* Check the expected MPU is present. */
    if( portMPU_TYPE_REG == portEXPECTED_MPU_TYPE_VALUE )
    {
        __asm volatile ( "dsb" );
        __asm volatile ( "isb" );
        /* Disable MPU before configuration */
        portMPU_CTRL_REG &= ~portMPU_ENABLE;

        /* portCODE_DATA_MEMORY_MAP_REGION */
        portMPU_REGION_BASE_ADDRESS_REG =
                ( ( uint32_t ) __FreeRTOS_code_data_start__ ) |
                ( portMPU_REGION_VALID ) |
                ( portCODE_DATA_MEMORY_MAP_REGION );
        portMPU_REGION_ATTRIBUTE_REG =
                ( prvGetMPURegionSizeSetting( ( uint32_t ) __FreeRTOS_code_data_end__ - ( uint32_t ) __FreeRTOS_code_data_start__)) |
                ( portMPU_REGION_READ_WRITE ) |
                ( ( 0x0BUL & portMPU_RASR_TEX_S_C_B_MASK ) << portMPU_RASR_TEX_S_C_B_LOCATION ) |
                ( portMPU_REGION_ENABLE );

        /* Setup the privileged functions for privileged only access.
         * This is where the kernel code is placed. */
        portMPU_REGION_BASE_ADDRESS_REG =
                ( ( uint32_t ) __FreeRTOS_privileged_functions_start__ ) |
                ( portMPU_REGION_VALID ) |
                ( portPRIVILEGED_FUNCTIONS_REGION );
        portMPU_REGION_ATTRIBUTE_REG =
                ( prvGetMPURegionSizeSetting( ( uint32_t ) __FreeRTOS_privileged_functions_end__
                        - ( uint32_t ) __FreeRTOS_privileged_functions_start__ ) ) |
                ( portMPU_REGION_PRIVILEGED_READ_ONLY ) |
                ( portMPU_REGION_ENABLE );

        /* Setup the system MPU wrappers functions.
         * This is where the system FreeRTOS MPU wrappers code is placed. */
        portMPU_REGION_BASE_ADDRESS_REG =
                ( ( uint32_t ) __FreeRTOS_system_calls_start__ ) |
                ( portMPU_REGION_VALID ) |
                ( portPRIVILEGED_SYSTEM_CALLS_REGION );
        portMPU_REGION_ATTRIBUTE_REG =
                ( prvGetMPURegionSizeSetting( ( uint32_t ) __FreeRTOS_system_calls_end__
                        - ( uint32_t ) __FreeRTOS_system_calls_start__ ) ) |
                ( portMPU_REGION_READ_WRITE ) |
                ( ( 0x0BUL & portMPU_RASR_TEX_S_C_B_MASK ) << portMPU_RASR_TEX_S_C_B_LOCATION ) |
                ( portMPU_REGION_ENABLE );

        /* Setup the privileged data RAM region.
         * This is where the kernel data is placed. */
        portMPU_REGION_BASE_ADDRESS_REG =
                ( ( uint32_t ) __FreeRTOS_privileged_data_start__ ) |
                ( portMPU_REGION_VALID ) |
                ( portPRIVILEGED_DATA_REGION );
        portMPU_REGION_ATTRIBUTE_REG =
                ( prvGetMPURegionSizeSetting( ( uint32_t ) __FreeRTOS_privileged_data_end__
                        - ( uint32_t ) __FreeRTOS_privileged_data_start__ ) ) |
                ( portMPU_REGION_PRIVILEGED_READ_WRITE ) |
                ( ( 0x0BUL & portMPU_RASR_TEX_S_C_B_MASK ) << portMPU_RASR_TEX_S_C_B_LOCATION ) |
                ( portMPU_REGION_EXECUTE_NEVER ) |
                ( portMPU_REGION_ENABLE );

        /* Enable the memory fault exception. */
        portNVIC_SYS_CTRL_STATE_REG |= portNVIC_MEM_FAULT_ENABLE;

        /* Enable the MPU with the background region configured. */
        portMPU_CTRL_REG |= ( portMPU_ENABLE | portMPU_BACKGROUND_ENABLE );

        __asm volatile ( "dsb" );
        __asm volatile ( "isb" );
    }
}
/*-----------------------------------------------------------*/

static uint32_t prvGetMPURegionSizeSetting(uint32_t ulActualSizeInBytes) {
    uint32_t ulRegionSize, ulReturnValue = 4;

    /* 32B/4 is the smallest region size, 4GB/31 is the largest valid value for
     * ulReturnValue. */
    for (ulRegionSize = 32UL; ulReturnValue < 31UL; (ulRegionSize <<= 1UL)) {
        if (ulActualSizeInBytes <= ulRegionSize) {
            break;
        } else {
            ulReturnValue++;
        }
    }

    /* Shift the code by one before returning so it can be written directly
     * into the the correct bit position of the attribute register. */
    return (ulReturnValue << 1UL);
}
/*-----------------------------------------------------------*/
BaseType_t xIsPrivileged(void) {
    __asm volatile (
            "   mrs r0, control                         \n" /* r0 = CONTROL */
            "   tst r0, #1                              \n" /* Perform r0 & 1 (bitwise AND) and update the conditions flag */
            "   ite ne                                  \n"
            "   movne r0, #0                            \n" /* CONTROL[0]!=0. Return false to indicate that the processor is not privileged */
            "   moveq r0, #1                            \n" /* CONTROL[0]==0. Return true to indicate that the processor is privileged */
            "   bx lr                                   \n" /* Return */
            ::: "r0", "memory"
    );
}
/*-----------------------------------------------------------*/

void vResetPrivilege(void) {
    __asm volatile (
            "   mrs r0, control                         \n" /* r0 = CONTROL */
            "   orrs r0, #1                             \n" /* r0 = r0 | 1 */
            "   msr control, r0                         \n" /* CONTROL = r0 */
            "   bx lr                                   \n" /* Return */
            ::: "r0", "memory"
    );
}
/*-----------------------------------------------------------*/

void vPortSwitchToUserMode(void) {
    /* Load the current task's MPU settings from its TCB. */
    xMPU_SETTINGS *xTaskMpuSettings = xTaskGetMPUSettings( NULL);

    /* Mark the task as unprivileged. */
    xTaskMpuSettings->ulTaskFlags &= (~( portTASK_IS_PRIVILEGED_FLAG));

    /* Lower the processor's privilege level. */
    vResetPrivilege();
}
/*-----------------------------------------------------------*/
void vPortStoreTaskMPUSettings( xMPU_SETTINGS * xMPUSettings,
                                const struct xMEMORY_REGION * const xRegions,
                                StackType_t * pxBottomOfStack,
                                configSTACK_DEPTH_TYPE uxStackDepth )
{
    extern uint32_t __FreeRTOS_privileged_data_start__[];
    extern uint32_t __FreeRTOS_privileged_data_end__[];
    extern uint32_t __FreeRTOS_code_data_start__[];
    extern uint32_t __FreeRTOS_code_data_end__[];

    int32_t lIndex;
    uint32_t ul;

    if( xRegions == NULL )
    {
        /* No MPU regions are specified so allow access to all RAM. */
        xMPUSettings->xRegion[ 0 ].ulRegionBaseAddress =
                ( ( uint32_t ) __FreeRTOS_code_data_start__ ) |
                ( portMPU_REGION_VALID ) |
                ( portSTACK_REGION );
        xMPUSettings->xRegion[ 0 ].ulRegionAttribute =
                ( prvGetMPURegionSizeSetting( ( uint32_t ) __FreeRTOS_code_data_end__ - ( uint32_t ) __FreeRTOS_code_data_start__)) |
                ( portMPU_REGION_READ_WRITE ) |
                ( ( 0x0BUL & portMPU_RASR_TEX_S_C_B_MASK ) << portMPU_RASR_TEX_S_C_B_LOCATION ) |
                ( portMPU_REGION_ENABLE );


        xMPUSettings->xRegionSettings[ 0 ].ulRegionStartAddress = ( uint32_t ) __FreeRTOS_code_data_start__;
        xMPUSettings->xRegionSettings[ 0 ].ulRegionEndAddress = ( uint32_t ) __FreeRTOS_code_data_end__;
        xMPUSettings->xRegionSettings[ 0 ].ulRegionPermissions = ( tskMPU_READ_PERMISSION |
                                                                   tskMPU_WRITE_PERMISSION );

        /* Invalidate user configurable regions. */
        for( ul = 1UL; ul <= portNUM_CONFIGURABLE_REGIONS; ul++ )
        {
            xMPUSettings->xRegion[ ul ].ulRegionBaseAddress = ( ( portFIRST_CONFIGURABLE_REGION + ul - 1UL ) | portMPU_REGION_VALID );
            xMPUSettings->xRegion[ ul ].ulRegionAttribute = 0UL;
            xMPUSettings->xRegionSettings[ ul ].ulRegionStartAddress = 0UL;
            xMPUSettings->xRegionSettings[ ul ].ulRegionEndAddress = 0UL;
            xMPUSettings->xRegionSettings[ ul ].ulRegionPermissions = 0UL;
        }
    }
    else
    {
        /* This function is called automatically when the task is created - in
         * which case the stack region parameters will be valid.  At all other
         * times the stack parameters will not be valid and it is assumed that the
         * stack region has already been configured. */
        if( uxStackDepth > 0 )
        {
            /* Define the region that allows access to the stack. */
            xMPUSettings->xRegion[ 0 ].ulRegionBaseAddress =
                ( ( uint32_t ) pxBottomOfStack ) |
                ( portMPU_REGION_VALID ) |
                ( portSTACK_REGION );

            xMPUSettings->xRegion[ 0 ].ulRegionAttribute =
                ( portMPU_REGION_READ_WRITE ) |
                ( portMPU_REGION_EXECUTE_NEVER ) |
                ( prvGetMPURegionSizeSetting( uxStackDepth * ( configSTACK_DEPTH_TYPE ) sizeof( StackType_t ) ) ) |
                ( ( configTEX_S_C_B & portMPU_RASR_TEX_S_C_B_MASK ) << portMPU_RASR_TEX_S_C_B_LOCATION ) |
                ( portMPU_REGION_ENABLE );

            xMPUSettings->xRegionSettings[ 0 ].ulRegionStartAddress = ( uint32_t ) pxBottomOfStack;
            xMPUSettings->xRegionSettings[ 0 ].ulRegionEndAddress = ( uint32_t ) ( ( uint32_t ) ( pxBottomOfStack ) +
                                                                                   ( uxStackDepth * ( configSTACK_DEPTH_TYPE ) sizeof( StackType_t ) ) - 1UL );
            xMPUSettings->xRegionSettings[ 0 ].ulRegionPermissions = ( tskMPU_READ_PERMISSION |
                                                                       tskMPU_WRITE_PERMISSION );
        }

        lIndex = 0;

        for( ul = 1UL; ul <= portNUM_CONFIGURABLE_REGIONS; ul++ )
        {
            if( ( xRegions[ lIndex ] ).ulLengthInBytes > 0UL )
            {
                /* Translate the generic region definition contained in
                 * xRegions into the CM4 specific MPU settings that are then
                 * stored in xMPUSettings. */
                xMPUSettings->xRegion[ ul ].ulRegionBaseAddress =
                    ( ( uint32_t ) xRegions[ lIndex ].pvBaseAddress ) |
                    ( portMPU_REGION_VALID ) |
                    ( portFIRST_CONFIGURABLE_REGION + ul - 1UL ); /* Region number. */

                xMPUSettings->xRegion[ ul ].ulRegionAttribute =
                    ( prvGetMPURegionSizeSetting( xRegions[ lIndex ].ulLengthInBytes ) ) |
                    ( xRegions[ lIndex ].ulParameters ) |
                    ( portMPU_REGION_ENABLE );

                xMPUSettings->xRegionSettings[ ul ].ulRegionStartAddress = ( uint32_t ) xRegions[ lIndex ].pvBaseAddress;
                xMPUSettings->xRegionSettings[ ul ].ulRegionEndAddress = ( uint32_t ) ( ( uint32_t ) xRegions[ lIndex ].pvBaseAddress + xRegions[ lIndex ].ulLengthInBytes - 1UL );
                xMPUSettings->xRegionSettings[ ul ].ulRegionPermissions = 0UL;

                if( ( ( xRegions[ lIndex ].ulParameters & portMPU_REGION_READ_ONLY ) == portMPU_REGION_READ_ONLY ) ||
                    ( ( xRegions[ lIndex ].ulParameters & portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIV_READ_ONLY ) == portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIV_READ_ONLY ) )
                {
                    xMPUSettings->xRegionSettings[ ul ].ulRegionPermissions = tskMPU_READ_PERMISSION;
                }

                if( ( xRegions[ lIndex ].ulParameters & portMPU_REGION_READ_WRITE ) == portMPU_REGION_READ_WRITE )
                {
                    xMPUSettings->xRegionSettings[ ul ].ulRegionPermissions = ( tskMPU_READ_PERMISSION | tskMPU_WRITE_PERMISSION );
                }
            }
            else
            {
                /* Invalidate the region. */
                xMPUSettings->xRegion[ ul ].ulRegionBaseAddress = ( ( portFIRST_CONFIGURABLE_REGION + ul - 1UL ) | portMPU_REGION_VALID );
                xMPUSettings->xRegion[ ul ].ulRegionAttribute = 0UL;
                xMPUSettings->xRegionSettings[ ul ].ulRegionStartAddress = 0UL;
                xMPUSettings->xRegionSettings[ ul ].ulRegionEndAddress = 0UL;
                xMPUSettings->xRegionSettings[ ul ].ulRegionPermissions = 0UL;
            }

            lIndex++;
        }
    }
}
#endif /* configENABLE_MPU == 1 */
/*-----------------------------------------------------------*/

#if ( configASSERT_DEFINED == 1 )

    void vPortValidateInterruptPriority( void )
    {
    uint32_t ulCurrentInterrupt;
    uint8_t ucCurrentPriority;

    /* Obtain the number of the currently executing interrupt. */
    __asm volatile ( "mrs %0, ipsr" : "=r" ( ulCurrentInterrupt )::"memory" );

    /* Is the interrupt number a user defined interrupt? */
        if( ulCurrentInterrupt >= portFIRST_USER_INTERRUPT_NUMBER )
        {
        /* Look up the interrupt's priority. */
        ucCurrentPriority = pcInterruptPriorityRegisters[ulCurrentInterrupt];

        /* The following assertion will fail if a service routine (ISR) for
         * an interrupt that has been assigned a priority above
         * configMAX_SYSCALL_INTERRUPT_PRIORITY calls an ISR safe FreeRTOS API
         * function.  ISR safe FreeRTOS API functions must *only* be called
         * from interrupts that have been assigned a priority at or below
         * configMAX_SYSCALL_INTERRUPT_PRIORITY.
         *
         * Numerically low interrupt priority numbers represent logically high
         * interrupt priorities, therefore the priority of the interrupt must
         * be set to a value equal to or numerically *higher* than
         * configMAX_SYSCALL_INTERRUPT_PRIORITY.
         *
         * Interrupts that  use the FreeRTOS API must not be left at their
         * default priority of  zero as that is the highest possible priority,
         * which is guaranteed to be above configMAX_SYSCALL_INTERRUPT_PRIORITY,
         * and  therefore also guaranteed to be invalid.
         *
         * FreeRTOS maintains separate thread and ISR API functions to ensure
         * interrupt entry is as fast and simple as possible.
         *
         * The following links provide detailed information:
         * https://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html
         * https://www.FreeRTOS.org/FAQHelp.html */
        configASSERT(ucCurrentPriority >= ucMaxSysCallPriority);
    }

    /* Priority grouping:  The interrupt controller (NVIC) allows the bits
     * that define each interrupt's priority to be split between bits that
     * define the interrupt's pre-emption priority bits and bits that define
     * the interrupt's sub-priority.  For simplicity all bits must be defined
     * to be pre-emption priority bits.  The following assertion will fail if
     * this is not the case (if some bits represent a sub-priority).
     *
     * If the application only uses CMSIS libraries for interrupt
     * configuration then the correct setting can be achieved on all Cortex-M
     * devices by calling NVIC_SetPriorityGrouping( 0 ); before starting the
     * scheduler.  Note however that some vendor specific peripheral libraries
     * assume a non-zero priority group setting, in which cases using a value
     * of zero will result in unpredictable behaviour. */
    configASSERT(( portAIRCR_REG & portPRIORITY_GROUP_MASK ) <= ulMaxPRIGROUPValue);
}

#endif /* configASSERT_DEFINED */
