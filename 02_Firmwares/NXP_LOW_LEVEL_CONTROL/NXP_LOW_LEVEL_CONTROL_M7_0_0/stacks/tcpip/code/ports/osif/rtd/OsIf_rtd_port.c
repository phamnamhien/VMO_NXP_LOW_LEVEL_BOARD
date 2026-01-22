/*
 * Copyright 2020-2024 NXP
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
 */

/*!
 * @file OsIf_rtd_port.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, Taking address of near auto variable
 * The code is not dynamically linked. An absolute stack address is obtained when
 * taking the address of the near auto variable. A source of error in writing
 * dynamic code is that the stack segment may be different from the data segment.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Directive 4.9, Function-like macro defined.
 * The macros are used to validate input parameters to driver functions.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 7.2, Unsigned integer literal without a 'U' suffix
 * Register address defined by FreeRTOS header files.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.4, external symbol defined without a prior
 * declaration.
 * The symbol is declared in the hardware access file as external; it is needed
 * for accessing the installed callback, but is not a part of the public API.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, Could define variable at block scope
 * The variable is used in pal c file, so it must remain global.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.13, Pointer variable could be declared as pointing to const
 * Type definition is done in FreeRTOS header files.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.3, The value of an expression shall not be assigned to an
 * object with a narrower essential type or a different essential type category.
 * This is a string that will be concatenated to a macro variable to define a new one.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.8, Impermissible cast of composite expression
 * Required in comparisons between constants and numerical types.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.2, Conversion between a pointer to incomplete type and another type
 * The is a fake finding
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and integer type.
 * The cast is required to initialize a pointer with an unsigned long define, representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, Cast from unsigned int to pointer.
 * This is required for initializing pointers to the module's memory map, which is located at a fixed address.
 *
 */

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Devassert.h"
#include "OsIf_rtd_port.h"
#if defined(USING_OS_FREERTOS)
#if defined(S32G2XX)
#include "S32G274A_SCB.h"
#elif defined(S32G3XX)
#include "S32G399A_SCB.h"
#elif defined(S32K314)
#include "S32K314_SCB.h"
#elif defined(S32K344)
#include "S32K344_SCB.h"
#elif defined(S32K341)
#include "S32K341_SCB.h"
#elif defined(S32K342)
#include "S32K342_SCB.h"
#elif defined(S32K322)
#include "S32K322_SCB.h"
#elif defined(S32K324)
#include "S32K324_SCB.h"
#elif defined(S32K394)
#include "S32K394_SCB.h"
#elif defined(S32K358)
#include "S32K358_SCB.h"
#elif defined(S32K356)
#include "S32K356_SCB.h"
#elif defined(S32K374)
#include "S32K37_SCB.h"
#elif defined(S32K376)
#include "S32K37_SCB.h"
#elif defined(S32K388)
#include "S32K388_SCB.h"
#elif defined(S32K389)
#include "S32K389_SCB.h"
#elif defined(S32K396)
#include "S32K39_SCB.h"
#elif defined(S32R45)
#include "S32R45_SCB.h"
#elif defined(S32K1XX)
#include "S32K148_SCB.h"
#elif defined(S32Z2XX)
#if defined(CPU_CORTEX_M33)
#include "S32Z2_SCB.h"
#else /* CPU_CORTEX_M33 */
#include "S32Z2_GIC.h"
#endif /* CPU_CORTEX_M33 */
#elif defined(S32E2XX)
#if defined(CPU_CORTEX_M33)
#include "S32E2_SCB.h"
#else /* CPU_CORTEX_M33 */
#include "S32E2_GIC.h"
#endif /* CPU_CORTEX_M33 */
#elif defined(SAF8544)
#include "SAF85XX_SCB.h"
#elif defined(S32R41)
#include "S32R41_SCB.h"
#elif defined(SJA1110)
#include "SJA1110.h"
#elif defined(S32N55)
#include "S32N55.h"
#include "S32N55_GIC.h"
#else /* S32G2XX */
#error "Unknown platform!"
#endif /* defined(S32G2XX) */

#include "FreeRTOS.h"
#include "task.h"
#elif defined(USING_OS_AUTOSAROS)
#include "Os.h"
#include "StandardTypes.h"
#endif /* USING_OS_FREERTOS */

#include "stdbool.h"

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/
/*! @brief Converts milliseconds to ticks*/
#define MSEC_TO_TICK(msec) (pdMS_TO_TICKS(msec))

#if defined(USING_OS_FREERTOS)
/* define a macro to access the FreeRTOS mutex/semaphore handle from an
 * osif mutex_t or semaphore_t */
#if configSUPPORT_STATIC_ALLOCATION == 1
#define SEM_HANDLE(sem)    ((sem).handle)
#else /* configSUPPORT_STATIC_ALLOCATION == 0, it's dynamic allocation */
#define SEM_HANDLE(sem)    (sem)
#endif
#endif /* USING_OS_FREERTOS */

/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/
static volatile uint32 u32OsIf_Milliseconds = 0U;

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/
#if defined(USING_OS_FREERTOS)
#if (defined(CPU_CORTEX_M7) || defined(CPU_CORTEX_M33) || defined(CPU_CORTEX_M4F))
/* Cortex M device - read ICSR[IPSR] value */
static inline boolean OsIf_IsIsrContext(void)
{
    boolean is_isr = FALSE;
    uint32 ipsr_code = (uint32)((uint32)((uint32)S32_SCB->ICSR & (uint32)S32_SCB_ICSR_VECTACTIVE_MASK) >>
                            (uint32)S32_SCB_ICSR_VECTACTIVE_SHIFT);
    if (0U != ipsr_code)
    {
        is_isr = TRUE;
    }

    return is_isr;
}
#elif defined (CPU_CORTEX_R52)
static inline boolean OsIf_IsIsrContext(void)
{
    /*! Table of base addresses for GICR INT instances. */
    S32_GICR_Type * const GICRBase = S32_GICR;
    bool is_isr = false;

    uint32_t isactiveri = GICRBase->GICR_ISACTIVER;
    if (isactiveri != 0u)
    {
        is_isr = true;
    }

    return is_isr;
}
#else /* (defined(CPU_CORTEX_M7) || defined(CPU_CORTEX_M33) || defined(CPU_CORTEX_M4F)) */
    #error "Unsupported architecture."
#endif /* defined(CPU_CORTEX_M7) || defined(CPU_CORTEX_M33) */
#endif /* USING_OS_FREERTOS */

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/
void OsIf_Millisecond(void)
{
    u32OsIf_Milliseconds++;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OsIf_TimeDelay
 * Description   : This function blocks (sleep) the current thread for a number
 *                 of milliseconds.
 *END**************************************************************************/
void OsIf_TimeDelay(const uint32 delayMs)
{
#if defined(USING_OS_FREERTOS)
    /* One dependency for FreeRTOS config file */
    /* INCLUDE_vTaskDelay */
    vTaskDelay(pdMS_TO_TICKS(delayMs));

#else /* USING_OS_FREERTOS */

    uint32 endTimeMs = 0U;

    endTimeMs = delayMs + OsIf_GetMilliseconds();

    while (OsIf_GetMilliseconds() < endTimeMs)
    {
        /* Wait for delay */
    }
#endif /* USING_OS_FREERTOS */
}


/*FUNCTION**********************************************************************
 *
 * Function Name : OsIf_GetMilliseconds
 * Description   : This function returns the number of miliseconds elapsed since
 *                 starting the internal timer (since scheduler was started).
 *END**************************************************************************/
uint32 OsIf_GetMilliseconds(void)
{
#if defined(USING_OS_FREERTOS)
    /*
    * Return the tick count in milliseconds
    * Note: if configTICK_RATE_HZ is less than 1000, the return value will be truncated
    * to 32-bit wide for large values of the tick count.
    */
    return (uint32)((((uint64)xTaskGetTickCount()) * 1000u) / configTICK_RATE_HZ);

#elif defined(USING_OS_AUTOSAROS)
    uint32 u32Counter = 0U;
    StatusType Status;
    TickType value;

    Status = GetCounterValue(OsCounter_0, &value);
    /* Convert from us to ms.
     * TODO: Change this to be more configurable based on the SystemTimer tick and counter configuration. */
    u32Counter = ((uint32)value / 1000U);

    return u32Counter;
#else
    /* This assumes that 1 tick = 1 millisecond */
    return u32OsIf_Milliseconds;
#endif /* USING_OS_FREERTOS */
}

#if defined(USING_OS_FREERTOS)
/*FUNCTION**********************************************************************
 *
 * Function Name : OsIf_MutexLock
 * Description   : This function obtains the mutex lock or returns error if
 *                 timeout.
 *END**************************************************************************/
StatusType OsIf_MutexLock(const mutex_t * const pMutex,
                          const uint32 timeout)
{
    /* The (pMutex == NULL) case is a valid option, signaling that the mutex does
     * not need to be locked - do not use DevAssert in this case */

    uint32 timeoutTicks;
    StatusType osif_ret_code = (StatusType)E_OK;
    TaskHandle_t mutex_holder_handle;
    TaskHandle_t current_task_handle;
    BaseType_t operation_status = pdFAIL;

    if (NULL_PTR != pMutex)
    {
        SemaphoreHandle_t mutex_handle = SEM_HANDLE(*pMutex);
        /* Two dependencies for FreeRTOS config file */
        /* INCLUDE_xQueueGetMutexHolder */
        /* INCLUDE_xTaskGetCurrentTaskHandle */
        mutex_holder_handle = xSemaphoreGetMutexHolder(mutex_handle);
        current_task_handle = xTaskGetCurrentTaskHandle();

        /* If pMutex has been locked by current task, return error. */
        if (mutex_holder_handle == current_task_handle)
        {
            osif_ret_code = (StatusType)E_NOT_OK;
        }
        else
        {
            /* Convert timeout from millisecond to tick. */
            if (OSIF_WAIT_FOREVER == timeout)
            {
                timeoutTicks = portMAX_DELAY;
            }
            else
            {
                timeoutTicks = MSEC_TO_TICK(timeout);
            }

            /* Try to take the semaphore */
            operation_status = xSemaphoreTake(mutex_handle, timeoutTicks);

            osif_ret_code = ((pdPASS == operation_status) ? (StatusType)E_OK : (StatusType)E_NOT_OK);
        }
    }

    return osif_ret_code;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OsIf_MutexUnlock
 * Description   : This function unlocks the mutex, fails if the current thread
 *                 is not the mutex holder.
 *END**************************************************************************/
StatusType OsIf_MutexUnlock(const mutex_t * const pMutex)
{
    /* The (pMutex == NULL) case is a valid option, signaling that the mutex does
     * not need to be unlocked - do not use DevAssert in this case */

    StatusType osif_ret_code = (StatusType)E_OK;
    TaskHandle_t mutex_holder_handle;
    TaskHandle_t current_task_handle;
    BaseType_t operation_status = pdFAIL;

    if (NULL_PTR != pMutex)
    {
        SemaphoreHandle_t mutex_handle = SEM_HANDLE(*pMutex);
        /* Two dependencies for FreeRTOS config file */
        /* INCLUDE_xQueueGetMutexHolder */
        /* INCLUDE_xTaskGetCurrentTaskHandle */
        mutex_holder_handle = xSemaphoreGetMutexHolder(mutex_handle);
        current_task_handle = xTaskGetCurrentTaskHandle();

        /* If pMutex is not locked by current task, return error. */
        if (mutex_holder_handle != current_task_handle)
        {
            osif_ret_code = (StatusType)E_NOT_OK;
        }
        else
        {
            operation_status = xSemaphoreGive(mutex_handle);
            osif_ret_code = ((operation_status == pdPASS) ? (StatusType)E_OK : (StatusType)E_NOT_OK);
        }
    }

    return osif_ret_code;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OsIf_MutexCreate
 * Description   : This function creates (registers) a mutex object to the OS.
 *END**************************************************************************/
StatusType OsIf_MutexCreate(mutex_t * const pMutex)
{
    /* The (pMutex == NULL) case is a valid option, signaling that the mutex does
     * not need to be created - do not use DevAssert in this case */

    StatusType osif_ret_code = (StatusType)E_OK;

    if (NULL_PTR != pMutex)
    {
#if (configSUPPORT_STATIC_ALLOCATION == 1)

        pMutex->handle = xSemaphoreCreateMutexStatic(&(pMutex->buffer));
        if (NULL_PTR == pMutex->handle)
        {
            /* Mutex was not created successfully */
            osif_ret_code = (StatusType)E_NOT_OK;
        }

#else /* configSUPPORT_STATIC_ALLOCATION == 1 */

        *pMutex = xSemaphoreCreateMutex();
        if (NULL_PTR == *pMutex)
        {
            /* Mutex was not created successfully */
            osif_ret_code = (StatusType)E_NOT_OK;
        }

#endif /* configSUPPORT_STATIC_ALLOCATION == 1 */
    }

    return osif_ret_code;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OsIf_MutexDestroy
 * Description   : This function removes the mutex from the OS (and frees memory).
 *END**************************************************************************/
StatusType OsIf_MutexDestroy(const mutex_t * const pMutex)
{
    /* The (pMutex == NULL) case is a valid option, signaling that the mutex does
     * not need to be destroyed - do not use DevAssert in this case */

    if (NULL_PTR != pMutex)
    {
        SemaphoreHandle_t mutex_handle = SEM_HANDLE(*pMutex);
        DevAssert(mutex_handle);
        vSemaphoreDelete(mutex_handle);
    }

    return (StatusType)E_OK;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OsIf_SemaWait
 * Description   : This function performs the 'wait' (decrement) operation on a
 *                 semaphore; returns error if operation timeout.
 *END**************************************************************************/
StatusType OsIf_SemaWait(semaphore_t * const pSem,
                         const uint32 timeout)
{
    DevAssert(NULL_PTR != pSem);

    SemaphoreHandle_t sem_handle = SEM_HANDLE(*pSem);
    uint32 timeoutTicks;
    BaseType_t operation_status;
    StatusType osif_ret_code;

    /* Convert timeout from millisecond to ticks. */
    if (OSIF_WAIT_FOREVER == timeout)
    {
        timeoutTicks = portMAX_DELAY;
    }
    else
    {
        timeoutTicks = MSEC_TO_TICK(timeout);
    }

    /* Try to take the semaphore */
    operation_status = xSemaphoreTake(sem_handle, timeoutTicks);

    osif_ret_code = ((pdPASS == operation_status) ? (StatusType)E_OK : (StatusType)E_NOT_OK);

    return osif_ret_code;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OsIf_SemaPost
 * Description   : This function performs the 'post' (increment) operation on a
 *                 semaphore.
 *END**************************************************************************/
StatusType OsIf_SemaPost(semaphore_t * const pSem)
{
    DevAssert(NULL_PTR != pSem);

    BaseType_t operation_status = pdFAIL;
    StatusType osif_ret_code;
    SemaphoreHandle_t sem_handle = SEM_HANDLE(*pSem);
    /* Check if the post operation is executed from ISR context */
    boolean is_isr = OsIf_IsIsrContext();

    if (is_isr)
    {
        /* Execution from exception handler (ISR) */
        BaseType_t taskWoken = pdFALSE;
        operation_status = xSemaphoreGiveFromISR(sem_handle, &taskWoken);

        if (pdPASS == operation_status)
        {
            /* Perform a context switch if necessary */
            portYIELD_FROM_ISR(taskWoken);
        }
    }
    else
    {
        /* Execution from task */
        operation_status = xSemaphoreGive(sem_handle);
    }

    /* pdFAIL in case that the semaphore is full */
    osif_ret_code = ((pdPASS == operation_status) ? (StatusType)E_OK : (StatusType)E_NOT_OK);

    return osif_ret_code;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OsIf_SemaCreate
 * Description   : This function creates (registers) a semaphore object to the OS.
 *END**************************************************************************/
StatusType OsIf_SemaCreate(semaphore_t * const pSem,
                           const uint8 initValue)
{
    DevAssert(NULL_PTR != pSem);

    StatusType osif_ret_code = (StatusType)E_OK;

#if (configSUPPORT_STATIC_ALLOCATION == 1)

    pSem->handle = xSemaphoreCreateCountingStatic(0xFFu, initValue, &(pSem->buffer));
    if (NULL_PTR == pSem->handle)
    {
        /* Semaphore was not created successfully */
        osif_ret_code = (StatusType)E_NOT_OK;
    }

#else /* configSUPPORT_STATIC_ALLOCATION == 1 */

    *pSem = xSemaphoreCreateCounting(0xFFu, initValue);
    if (NULL_PTR == *pSem)
    {
        /* Semaphore was not created successfully */
        osif_ret_code = (StatusType)E_NOT_OK;
    }

#endif /* configSUPPORT_STATIC_ALLOCATION == 1 */

    return osif_ret_code;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OsIf_SemaDestroy
 * Description   : This function removes a semaphore object from the OS (frees
 *                 memory).
 *END**************************************************************************/
StatusType OsIf_SemaDestroy(const semaphore_t * const pSem)
{
    DevAssert(NULL_PTR != pSem);

    SemaphoreHandle_t sem_handle = SEM_HANDLE(*pSem);

    DevAssert(sem_handle);
    vSemaphoreDelete(sem_handle);

    return (StatusType)E_OK;
}
#endif /* USING_OS_FREERTOS */
