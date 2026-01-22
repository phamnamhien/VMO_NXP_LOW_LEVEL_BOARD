/*
 * Copyright (c) 2017 Simon Goldschmidt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Simon Goldschmdit <goldsimon@gmx.de>
 *
 */


#ifndef LWIP_FREERTOS_ARCH_SYS_ARCH_H
#define LWIP_FREERTOS_ARCH_SYS_ARCH_H

#include "lwip/opt.h"
#include "lwip/arch.h"

/** This is returned by _fromisr() sys functions to tell the outermost function
 * that a higher priority task was woken and the scheduler needs to be invoked.
 */
#define ERR_NEED_SCHED 123

/* This port includes FreeRTOS headers in sys_arch.c only.
 *  FreeRTOS uses pointers as object types. We use wrapper structs instead of
 * void pointers directly to get a tiny bit of type safety.
 */

void sys_arch_msleep(u32_t delay_ms);
#define sys_msleep(ms) sys_arch_msleep(ms)

#if SYS_LIGHTWEIGHT_PROT
typedef uint32_t sys_prot_t;



#endif /* SYS_LIGHTWEIGHT_PROT */
/*
 * The stack be used either in a bare metal environment (NO_SYS=1) or in an OS environment (NO_SYS=0).
 * This implementation supports FreeRTOS as operating system (USING_OS_FREERTOS)
*/

/* Sanity checks */
#if NO_SYS
#if defined(USING_OS_FREERTOS)
#error NO_SYS cannot be used with USING_OS_FREERTOS
#endif /* defined(USING_OS_FREERTOS) */
#else /* NO_SYS */
#if defined(USING_OS_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portmacro.h"
#else /* defined(USING_OS_FREERTOS) */
#error USING_OS_FREERTOS needs to be enabled when NO_SYS=0
#endif /* defined(USING_OS_FREERTOS) */
#endif /* NO_SYS */


#if defined(USING_OS_FREERTOS)

struct _sys_mut {
  void *mut;
};
typedef struct _sys_mut sys_mutex_t;

#define SYS_MBOX_NULL               (QueueHandle_t)0
#define SYS_SEM_NULL                (SemaphoreHandle_t)0
#define SYS_MUTEX_NULL              SYS_SEM_NULL



#if LWIP_SOCKET_SET_ERRNO
#ifndef set_errno
#define set_errno(err) do { if (err) { vTaskSetThreadLocalStoragePointer(NULL, 0, (void *)(err)); } } while(0)
#define errno ((int)pvTaskGetThreadLocalStoragePointer(NULL, 0))
#endif /* set_errno */
#endif /* LWIP_SOCKET_SET_ERRNO */
#define sys_mutex_valid_val(mutex)   ((mutex).mut != NULL)
#define sys_mutex_valid(mutex)       (((mutex) != NULL) && sys_mutex_valid_val(*(mutex)))
#define sys_mutex_set_invalid(mutex) ((mutex)->mut = NULL)

struct _sys_sem {
  void *sem;
};
typedef struct _sys_sem sys_sem_t;
#define sys_sem_valid_val(sema)   ((sema).sem != NULL)
#define sys_sem_valid(sema)       (((sema) != NULL) && sys_sem_valid_val(*(sema)))
#define sys_sem_set_invalid(sema) ((sema)->sem = NULL)

struct _sys_mbox {
  void *mbx;
};
typedef struct _sys_mbox sys_mbox_t;
#define sys_mbox_valid_val(mbox)   ((mbox).mbx != NULL)
#define sys_mbox_valid(mbox)       (((mbox) != NULL) && sys_mbox_valid_val(*(mbox)))
#define sys_mbox_set_invalid(mbox) ((mbox)->mbx = NULL)

struct _sys_thread {
  void *thread_handle;
};
typedef struct _sys_thread sys_thread_t;
void sys_thread_delete(sys_thread_t thread);
void sys_mbox_post_to_front(sys_mbox_t *mbox, void *data);


sys_sem_t* sys_arch_netconn_sem_get(void);
void sys_arch_netconn_sem_alloc(void);




#endif /* defined(USING_OS_FREERTOS) */
/* Threading options */
void sys_arch_netconn_sem_free(void);
#define LWIP_NETCONN_THREAD_SEM_GET()   sys_arch_netconn_sem_get()
#define LWIP_NETCONN_THREAD_SEM_ALLOC() sys_arch_netconn_sem_alloc()
#define LWIP_NETCONN_THREAD_SEM_FREE()  sys_arch_netconn_sem_free()
void sys_mark_tcpip_thread(void);
#define LWIP_MARK_TCPIP_THREAD()   sys_mark_tcpip_thread()

#if LWIP_TCPIP_CORE_LOCKING
void sys_lock_tcpip_core(void);
#define LOCK_TCPIP_CORE()          sys_lock_tcpip_core()
void sys_unlock_tcpip_core(void);
#define UNLOCK_TCPIP_CORE()        sys_unlock_tcpip_core()
#endif

#endif /* LWIP_FREERTOS_ARCH_SYS_ARCH_H */

