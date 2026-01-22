/*
 * Copyright 2017-2025 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is derived from lwIP contribution example with the following copyright:
 *
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 */


/* lwIP includes. */

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, Taking address of near auto variable.
 * The code is not dynamically linked. An absolute stack address is obtained
 * when taking the address of the near auto variable. A source of error in
 * writing dynamic code is that the stack segment may be different from the data
 * segment.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 2.1, A project shall not contain unreachable code.
 * These are safety checks to avoid dereferencing NULL pointers.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Directive 4.6, typedefs that indicate size and
 * signedness should be used in place of the basic numerical types.
 * These are function parameters whose size is platform dependent.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 4.7, Ignoring return value of function
 * The return value of the function can be ignored, as checks are done on parameters
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.3, All declarations or an object or function shall use the
 * same names and type qualifiers.
 * Function should take NULL as parameter. This is a fake finding.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.4, external symbol defined without a prior
 * declaration.
 * The symbol is declared in the hardware access file as external; it is needed
 * for accessing the installed callback, but is not a part of the public API.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.13, Pointer parameter 'dummyptr' could be declared as pointing to const
 * Memory pointed by 'dummyptr' is used for receive.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.1, Unpermitted operand to operator '||'
 * Variable is of essential boolean type
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 10.3, The value of an expression shall not be assigned to an
 * object with a narrower essential type or a different essential type category.
 * These are exit codes returned by functions when an error occurs.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.2, Conversions shall not be performed between a pointer
 * to an incomplete type and any other type
 * This cast is to required write 8bit or 16bit data into corresponding address.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.3, Cast performed between a pointer
 * to object type and a pointer to a different object type.
 * This cast is to required write 8bit or 16bit data into corresponding address.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and integer type.
 * The cast is required to initialize a pointer with an unsigned int define, representing a memory-mapped address.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.5, Conversion from pointer to void to pointer to other type .
 * The conversion is needed to wrap a thread function call.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 14.4, Conditional expression should have essentially Boolean type.
 * This is required for macro constructs in form do {...} while(0).
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 15.5, Return statement before end of function
 * The return statement before end of function is used for simpler code structure
 * and better readability.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 17.7, Ignoring return value of function
 * The return value of the function can be ignored, as checks are done on parameters
 *
 */

#ifdef USING_RTD
#include "OsIf.h"
#include "OsIf_rtd_port.h"
#else
#include "osif.h"
#endif /* USING_RTD */

#include <stdlib.h>
#include <stdio.h>

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/stats.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/mem.h"

#include "lwip/tcpip.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "arch/sys_arch.h"

/** Set this to 1 if you want the stack size passed to sys_thread_new() to be
 * interpreted as number of stack words (FreeRTOS-like).
 * Default is that they are interpreted as byte count (lwIP-like).
 */
#ifndef LWIP_FREERTOS_THREAD_STACKSIZE_IS_STACKWORDS
#define LWIP_FREERTOS_THREAD_STACKSIZE_IS_STACKWORDS  0
#endif

/** Set this to 1 to use a mutex for SYS_ARCH_PROTECT() critical regions.
 * Default is 0 and locks interrupts/scheduler for SYS_ARCH_PROTECT().
 */
#ifndef LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX
#define LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX     0
#endif

/** Set this to 1 to include a sanity check that SYS_ARCH_PROTECT() and
 * SYS_ARCH_UNPROTECT() are called matching.
 */
#ifndef LWIP_FREERTOS_SYS_ARCH_PROTECT_SANITY_CHECK
#define LWIP_FREERTOS_SYS_ARCH_PROTECT_SANITY_CHECK   0
#endif

/** Set this to 1 to let sys_mbox_free check that queues are empty when freed */
#ifndef LWIP_FREERTOS_CHECK_QUEUE_EMPTY_ON_FREE
#define LWIP_FREERTOS_CHECK_QUEUE_EMPTY_ON_FREE       0
#endif

/** Set this to 1 to enable core locking check functions in this port.
 * For this to work, you'll have to define LWIP_ASSERT_CORE_LOCKED()
 * and LWIP_MARK_TCPIP_THREAD() correctly in your lwipopts.h! */
#ifndef LWIP_FREERTOS_CHECK_CORE_LOCKING
#define LWIP_FREERTOS_CHECK_CORE_LOCKING              1
#endif

/** Set this to 0 to implement sys_now() yourself, e.g. using a hw timer.
 * Default is 1, where FreeRTOS ticks are used to calculate back to ms.
 */
#ifndef LWIP_FREERTOS_SYS_NOW_FROM_FREERTOS
#define LWIP_FREERTOS_SYS_NOW_FROM_FREERTOS           1
#endif

#ifndef MS_FACTOR 1000
#define MS_FACTOR 1000
#endif

#if !configSUPPORT_DYNAMIC_ALLOCATION
# error "lwIP FreeRTOS port requires configSUPPORT_DYNAMIC_ALLOCATION"
#endif
#if !INCLUDE_vTaskDelay
# error "lwIP FreeRTOS port requires INCLUDE_vTaskDelay"
#endif
#if !INCLUDE_vTaskSuspend
# error "lwIP FreeRTOS port requires INCLUDE_vTaskSuspend"
#endif
#if LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX || !LWIP_COMPAT_MUTEX
#if !configUSE_MUTEXES
# error "lwIP FreeRTOS port requires configUSE_MUTEXES"
#endif
#endif

#if SYS_LIGHTWEIGHT_PROT && LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX
static SemaphoreHandle_t sys_arch_protect_mutex;
#endif
#if SYS_LIGHTWEIGHT_PROT && LWIP_FREERTOS_SYS_ARCH_PROTECT_SANITY_CHECK
static sys_prot_t sys_arch_protect_nesting;
#endif

/* Masks off all bits but the VECTACTIVE bits in the ICSR register. */
#define portVECTACTIVE_MASK                   ( 0xFFUL )
#define portGIC_NO_ACTIVE_INT                 ( 0xFFUL )

#if defined CPU_CORTEX_R52
portFORCE_INLINE static uint32_t ulPortGetBASEPRI( void )
{
uint32_t ulOriginalBASEPRI;

   __asm volatile
    (
        " mrc p15, 0, %0, c4, c6, 0  " : "=r" (ulOriginalBASEPRI) :: "memory"
    );

   return ulOriginalBASEPRI;
}

portFORCE_INLINE static uint32_t ulPortGet_ICC_RPR( void )
{
uint32_t ulICC_RPR;

   __asm volatile
    (
        " mrc p15, 0, %0, c12, c11, 3  " : "=r" (ulICC_RPR) :: "memory"
    );

   return ulICC_RPR;
}

#endif /* CPU_CORTEX_R52 */


/* Initialize sys arch layer
*/
void
sys_init(void)
{
#if SYS_LIGHTWEIGHT_PROT && LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX
  /* initialize sys_arch_protect global mutex */
  sys_arch_protect_mutex = xSemaphoreCreateRecursiveMutex();
  LWIP_ASSERT("failed to create sys_arch_protect mutex",
    sys_arch_protect_mutex != NULL);
#endif /* SYS_LIGHTWEIGHT_PROT && LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX */
}

#if configUSE_16_BIT_TICKS == 1
#error This port requires 32 bit ticks or timer overflow will fail
#endif

#if LWIP_FREERTOS_SYS_NOW_FROM_FREERTOS
u32_t
sys_now(void)
{
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
}
#endif

u32_t
sys_jiffies(void)
{
  return xTaskGetTickCount();
}

/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.
*/
sys_prot_t
sys_arch_protect(void)
{
#if LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX
  BaseType_t ret;
  LWIP_ASSERT("sys_arch_protect_mutex != NULL", sys_arch_protect_mutex != NULL);

  ret = xSemaphoreTakeRecursive(sys_arch_protect_mutex, portMAX_DELAY);
  LWIP_ASSERT("sys_arch_protect failed to take the mutex", ret == pdTRUE);
#else /* LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX */
  taskENTER_CRITICAL();
#endif /* LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX */
#if LWIP_FREERTOS_SYS_ARCH_PROTECT_SANITY_CHECK
  {
    /* every nested call to sys_arch_protect() returns an increased number */
    sys_prot_t ret = sys_arch_protect_nesting;
    sys_arch_protect_nesting++;
    LWIP_ASSERT("sys_arch_protect overflow", sys_arch_protect_nesting > ret);
    return ret;
  }
#else
  return 1;
#endif
}

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.
*/
void sys_arch_unprotect(sys_prot_t pval)
{
#if LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX
  BaseType_t ret;
#endif
#if LWIP_FREERTOS_SYS_ARCH_PROTECT_SANITY_CHECK
  LWIP_ASSERT("unexpected sys_arch_protect_nesting", sys_arch_protect_nesting > 0);
  sys_arch_protect_nesting--;
  LWIP_ASSERT("unexpected sys_arch_protect_nesting", sys_arch_protect_nesting == pval);
#endif

#if LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX
  LWIP_ASSERT("sys_arch_protect_mutex != NULL", sys_arch_protect_mutex != NULL);

  ret = xSemaphoreGiveRecursive(sys_arch_protect_mutex);
  LWIP_ASSERT("sys_arch_unprotect failed to give the mutex", ret == pdTRUE);
#else /* LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX */
  taskEXIT_CRITICAL();
#endif /* LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX */
  LWIP_UNUSED_ARG(pval);
}

/*-----------------------------------------------------------------------------------
  Creates a new mutex. The mutex is allocated to the memory that 'mutex'
  points to (which can be both a pointer or the actual OS structure).
  If the mutex has been created, ERR_OK should be returned. Returning any
  other error will provide a hint what went wrong, but except for assertions,
  no real error handling is implemented.
*/
void
sys_arch_msleep(u32_t delay_ms)
{
  TickType_t delay_ticks = (delay_ms * configTICK_RATE_HZ) / MS_FACTOR;
  vTaskDelay(delay_ticks);
}

#if !LWIP_COMPAT_MUTEX

/* Create a new mutex*/
err_t
sys_mutex_new(sys_mutex_t *mutex)
{
  LWIP_ASSERT("mutex != NULL", mutex != NULL);

  mutex->mut = xSemaphoreCreateRecursiveMutex();
  if(mutex->mut == NULL) {
    SYS_STATS_INC(mutex.err);
    return ERR_MEM;
  }
  SYS_STATS_INC_USED(mutex);
  return ERR_OK;
}

/*-----------------------------------------------------------------------------------
  Blocks the thread until the mutex can be grabbed.
*/
void
sys_mutex_lock(sys_mutex_t *mutex)
{
  BaseType_t ret;
  LWIP_ASSERT("mutex != NULL", mutex != NULL);
  LWIP_ASSERT("mutex->mut != NULL", mutex->mut != NULL);

  ret = xSemaphoreTakeRecursive(mutex->mut, portMAX_DELAY);
  LWIP_ASSERT("failed to take the mutex", ret == pdTRUE);
}

/*-----------------------------------------------------------------------------------
  Releases the mutex previously locked through 'sys_mutex_lock()'
*/
void
sys_mutex_unlock(sys_mutex_t *mutex)
{
  BaseType_t ret;
  LWIP_ASSERT("mutex != NULL", mutex != NULL);
  LWIP_ASSERT("mutex->mut != NULL", mutex->mut != NULL);

  ret = xSemaphoreGiveRecursive(mutex->mut);
  LWIP_ASSERT("failed to give the mutex", ret == pdTRUE);
}

/*-----------------------------------------------------------------------------------
  Deallocates a mutex
*/
void
sys_mutex_free(sys_mutex_t *mutex)
{
  LWIP_ASSERT("mutex != NULL", mutex != NULL);
  LWIP_ASSERT("mutex->mut != NULL", mutex->mut != NULL);

  SYS_STATS_DEC(mutex.used);
  vSemaphoreDelete(mutex->mut);
  mutex->mut = NULL;
}

#endif /* !LWIP_COMPAT_MUTEX */

err_t
sys_sem_new(sys_sem_t *sem, u8_t initial_count)
{
  LWIP_ASSERT("sem != NULL", sem != NULL);
  LWIP_ASSERT("initial_count invalid (not 0 or 1)",
    (initial_count == 0) || (initial_count == 1));

  sem->sem = xSemaphoreCreateBinary();
  if(sem->sem == NULL) {
    SYS_STATS_INC(sem.err);
    return ERR_MEM;
  }
  SYS_STATS_INC_USED(sem);

  if(initial_count == 1) {
    BaseType_t ret = xSemaphoreGive(sem->sem);
    LWIP_ASSERT("sys_sem_new: initial give failed", ret == pdTRUE);
  }
  return ERR_OK;
}

void
sys_sem_signal(sys_sem_t *sem)
{
  BaseType_t ret;
  LWIP_ASSERT("sem != NULL", sem != NULL);
  LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

  ret = xSemaphoreGive(sem->sem);
  /* queue full is OK, this is a signal only... */
  LWIP_ASSERT("sys_sem_signal: sane return value",
    (ret == pdTRUE) || (ret == errQUEUE_FULL));
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout_ms)
{
  BaseType_t ret;
  LWIP_ASSERT("sem != NULL", sem != NULL);
  LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

  if(!timeout_ms) {
    /* wait infinite */
    ret = xSemaphoreTake(sem->sem, portMAX_DELAY);
    LWIP_ASSERT("taking semaphore failed", ret == pdTRUE);
  } else {
    TickType_t timeout_ticks = (timeout_ms * configTICK_RATE_HZ) / MS_FACTOR;
    ret = xSemaphoreTake(sem->sem, timeout_ticks);
    if (ret == errQUEUE_EMPTY) {
      /* timed out */
      return SYS_ARCH_TIMEOUT;
    }
    LWIP_ASSERT("taking semaphore failed", ret == pdTRUE);
  }

  /* Old versions of lwIP required us to return the time waited.
     This is not the case any more. Just returning != SYS_ARCH_TIMEOUT
     here is enough. */
  return 1;
}

/*-----------------------------------------------------------------------------------*/
/* Deallocates a semaphore
*/
void sys_sem_free(sys_sem_t *sem)
{
  LWIP_ASSERT("sem != NULL", sem != NULL);
  LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

  SYS_STATS_DEC(sem.used);
  vSemaphoreDelete(sem->sem);
  sem->sem = NULL;
}

err_t
sys_mbox_new(sys_mbox_t *mbox, int size)
{
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_ASSERT("size > 0", size > 0);

  mbox->mbx = xQueueCreate((UBaseType_t)size, sizeof(void *));
  if(mbox->mbx == NULL) {
    SYS_STATS_INC(mbox.err);
    return ERR_MEM;
  }
  SYS_STATS_INC_USED(mbox);
  return ERR_OK;
}



/*-----------------------------------------------------------------------------------
  Posts the "msg" to the mailbox. This function have to block until
  the "msg" is really posted.
*/
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  /* check if we are in the interrupt to call proper function*/
#if defined CPU_CORTEX_M7 || defined CPU_CORTEX_M33 || defined CPU_CORTEX_M4F
  if ((portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0)
  {
#elif defined CPU_CORTEX_R52
  if(portGIC_NO_ACTIVE_INT  == ulPortGet_ICC_RPR() )
  {
#endif /* CPU_CORTEX_M7 || defined CPU_CORTEX_M33 */
    while (pdTRUE != xQueueSend(mbox->mbx, &msg, 10000)) /* time is arbitrary */
    {
    ;
    }
  }
  else
  {
    BaseType_t xHigherPriorityTaskWoken;
    BaseType_t ret;
    ret = xQueueSendFromISR(mbox->mbx, &msg, &xHigherPriorityTaskWoken);
    LWIP_ASSERT ("Queue is full", pdTRUE == ret);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

/*-----------------------------------------------------------------------------------
  Try to post the "msg" to the mailbox. Returns ERR_MEM if this one
  is full, else, ERR_OK if the "msg" is posted.
*/
  err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
  {
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    err_t status = ERR_OK;


    /* check if we are in the interrupt to call proper function*/
  #if defined CPU_CORTEX_M7 || defined CPU_CORTEX_M33 || defined CPU_CORTEX_M4F
    if ((portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0)
    {
  #elif defined CPU_CORTEX_R52
    	if(portGIC_NO_ACTIVE_INT  == ulPortGet_ICC_RPR() )
    {
  #endif /* CPU_CORTEX_M7 || defined CPU_CORTEX_M33 || CPU_CORTEX_M4F */
      if (pdTRUE != xQueueSend(mbox->mbx, &msg, 10000)) /* time is arbitrary */
      {
          status = ERR_MEM;
      }
    }
    else
    {
      BaseType_t xHigherPriorityTaskWoken;
      BaseType_t ret;
      ret = xQueueSendFromISR(mbox->mbx, &msg, &xHigherPriorityTaskWoken);
      LWIP_ASSERT ("Queue is full", pdTRUE == ret);
    }
    return status;
  }

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
  BaseType_t ret;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

  ret = xQueueSendToBackFromISR(mbox->mbx, &msg, &xHigherPriorityTaskWoken);
  if (ret == pdTRUE) {
    if (xHigherPriorityTaskWoken == pdTRUE) {
      return ERR_NEED_SCHED;
    }
    return ERR_OK;
  } else {
    LWIP_ASSERT("mbox trypost failed", ret == errQUEUE_FULL);
    SYS_STATS_INC(mbox.err);
    return ERR_MEM;
  }
}


/*-----------------------------------------------------------------------------------
  Posts the "msg" to the front of the mailbox. This function have to block until
  the "msg" is really posted.
*/
void sys_mbox_post_to_front(sys_mbox_t *mbox, void *msg)
{
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  /* check if we are in the interrupt to call proper function*/
#if defined CPU_CORTEX_M7 || defined CPU_CORTEX_M33 || defined CPU_CORTEX_M4F
  if ((portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0)
  {
#elif defined CPU_CORTEX_R52
	  if(portGIC_NO_ACTIVE_INT  == ulPortGet_ICC_RPR() )
  {
#endif /* CPU_CORTEX_M7 || defined CPU_CORTEX_M33 || CPU_CORTEX_M4F */
    while (pdTRUE != xQueueSendToFront(mbox->mbx, &msg, 10000)) /* time is arbitrary */
    {
    ;
    }
  }
  else
  {
      BaseType_t xHigherPriorityTaskWoken;
      BaseType_t ret;
      ret = xQueueSendToFrontFromISR(mbox->mbx, &msg, &xHigherPriorityTaskWoken);
      LWIP_ASSERT ("Queue is full", pdTRUE == ret);
  }
}


/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP.
*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout_ms)
{
  BaseType_t ret;
  void *msg_dummy;
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

  if (!msg) {
    msg = &msg_dummy;
  }

  if (!timeout_ms) {
    /* wait infinite */
    ret = xQueueReceive(mbox->mbx, &(*msg), portMAX_DELAY);
    LWIP_ASSERT("mbox fetch failed", ret == pdTRUE);
  } else {
    TickType_t timeout_ticks = (timeout_ms * configTICK_RATE_HZ) / MS_FACTOR;
    ret = xQueueReceive(mbox->mbx, &(*msg), timeout_ticks);
    if (ret == errQUEUE_EMPTY) {
      /* timed out */
      *msg = NULL;
      return SYS_ARCH_TIMEOUT;
    }
    LWIP_ASSERT("mbox fetch failed", ret == pdTRUE);
  }

  /* Old versions of lwIP required us to return the time waited.
     This is not the case any more. Just returning != SYS_ARCH_TIMEOUT
     here is enough. */
  return 1;
}

/*-----------------------------------------------------------------------------------*/
/*
  This is similar to sys_arch_mbox_fetch, however if a message is not
  present in the mailbox, it immediately returns with the code
  SYS_MBOX_EMPTY. On success 0 is returned.
*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
  BaseType_t ret;
  void *msg_dummy;
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

  if (!msg) {
    msg = &msg_dummy;
  }

  ret = xQueueReceive(mbox->mbx, &(*msg), 0);
  if (ret == errQUEUE_EMPTY) {
    *msg = NULL;
    return SYS_MBOX_EMPTY;
  }
  LWIP_ASSERT("mbox fetch failed", ret == pdTRUE);

  return 0;
}

/*-----------------------------------------------------------------------------------
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

#if LWIP_FREERTOS_CHECK_QUEUE_EMPTY_ON_FREE
  {
    UBaseType_t msgs_waiting = uxQueueMessagesWaiting(mbox->mbx);
    LWIP_ASSERT("mbox quence not empty", msgs_waiting == 0);

    if (msgs_waiting != 0) {
      SYS_STATS_INC(mbox.err);
    }
  }
#endif

  vQueueDelete(mbox->mbx);

  SYS_STATS_DEC(mbox.used);
}

/*-----------------------------------------------------------------------------------
  Starts a new thread named "name" with priority "prio" that will begin its
  execution in the function "thread()". The "arg" argument will be passed as an
  argument to the thread() function. The stack size to used for this thread is
  the "stacksize" parameter. The id of the new thread is returned. Both the id
  and the priority are system dependent.
*/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
  TaskHandle_t rtos_task;
  BaseType_t ret;
  sys_thread_t lwip_thread;
  size_t rtos_stacksize;

  LWIP_ASSERT("invalid stacksize", stacksize > 0);
#if LWIP_FREERTOS_THREAD_STACKSIZE_IS_STACKWORDS
  rtos_stacksize = (size_t)stacksize;
#else
  rtos_stacksize = (size_t)stacksize / sizeof(StackType_t);
#endif

  /* lwIP's lwip_thread_fn matches FreeRTOS' TaskFunction_t, so we can pass the
     thread function without adaption here. */
  ret = xTaskCreate(thread, name, (configSTACK_DEPTH_TYPE)rtos_stacksize, arg, prio, &rtos_task);
  LWIP_ASSERT("task creation failed", ret == pdTRUE);

  lwip_thread.thread_handle = rtos_task;
  return lwip_thread;
}

#if LWIP_NETCONN_SEM_PER_THREAD
#if configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0

sys_sem_t *
sys_arch_netconn_sem_get(void)
{
  void* ret;
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  LWIP_ASSERT("task != NULL", task != NULL);

  ret = pvTaskGetThreadLocalStoragePointer(task, 0);
  return ret;
}

void
sys_arch_netconn_sem_alloc(void)
{
  void *ret;
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  LWIP_ASSERT("task != NULL", task != NULL);

  ret = pvTaskGetThreadLocalStoragePointer(task, 0);
  if(ret == NULL) {
    sys_sem_t *sem;
    err_t err;
    /* need to allocate the memory for this semaphore */
    sem = mem_malloc(sizeof(sys_sem_t));
    LWIP_ASSERT("sem != NULL", sem != NULL);
    err = sys_sem_new(sem, 0);
    LWIP_ASSERT("err == ERR_OK", err == ERR_OK);
    LWIP_ASSERT("sem invalid", sys_sem_valid(sem));
    vTaskSetThreadLocalStoragePointer(task, 0, sem);
  }
}

void sys_arch_netconn_sem_free(void)
{
  void* ret;
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  LWIP_ASSERT("task != NULL", task != NULL);

  ret = pvTaskGetThreadLocalStoragePointer(task, 0);
  if(ret != NULL) {
    sys_sem_t *sem = ret;
    sys_sem_free(sem);
    mem_free(sem);
    vTaskSetThreadLocalStoragePointer(task, 0, NULL);
  }
}

#else /* configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0 */
#error LWIP_NETCONN_SEM_PER_THREAD needs configNUM_THREAD_LOCAL_STORAGE_POINTERS
#endif /* configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0 */

#endif /* LWIP_NETCONN_SEM_PER_THREAD */

#if LWIP_FREERTOS_CHECK_CORE_LOCKING
#if LWIP_TCPIP_CORE_LOCKING

/** Flag the core lock held. A counter for recursive locks. */
static u8_t lwip_core_lock_count;
static TaskHandle_t lwip_core_lock_holder_thread;

void
sys_lock_tcpip_core(void)
{
   sys_mutex_lock(&lock_tcpip_core);
   if (lwip_core_lock_count == 0) {
     lwip_core_lock_holder_thread = xTaskGetCurrentTaskHandle();
   }
   lwip_core_lock_count++;
}

void
sys_unlock_tcpip_core(void)
{
   lwip_core_lock_count--;
   if (lwip_core_lock_count == 0) {
       lwip_core_lock_holder_thread = 0;
   }
   sys_mutex_unlock(&lock_tcpip_core);
}

#endif /* LWIP_TCPIP_CORE_LOCKING */

#if !NO_SYS
static TaskHandle_t lwip_tcpip_thread;
#endif

void
sys_mark_tcpip_thread(void)
{
#if !NO_SYS
  lwip_tcpip_thread = xTaskGetCurrentTaskHandle();
#endif
}

void
sys_check_core_locking(void)
{
  /* Embedded systems should check we are NOT in an interrupt context here */
  /* E.g. core Cortex-M3/M4 ports:
         configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );

     Instead, we use more generic FreeRTOS functions here, which should fail from ISR: */
  taskENTER_CRITICAL();
  taskEXIT_CRITICAL();

#if !NO_SYS
  if (lwip_tcpip_thread != 0) {
    TaskHandle_t current_thread = xTaskGetCurrentTaskHandle();

#if LWIP_TCPIP_CORE_LOCKING
    LWIP_ASSERT("Function called without core lock",
                current_thread == lwip_core_lock_holder_thread && lwip_core_lock_count > 0);
#else /* LWIP_TCPIP_CORE_LOCKING */
    LWIP_ASSERT("Function called from wrong thread", current_thread == lwip_tcpip_thread);
#endif /* LWIP_TCPIP_CORE_LOCKING */
  }
#endif /* !NO_SYS */
}

/* Wrap a thread method and its argument for being passed to sys_thread_function */
struct threadfunc {
  lwip_thread_fn function;
  void *arg;
};
#endif /* LWIP_FREERTOS_CHECK_CORE_LOCKING*/
/*-----------------------------------------------------------------------------------
  Deletes a thread
*/
void sys_thread_delete(sys_thread_t thread)
{
    vTaskDelete(thread.thread_handle);
}







