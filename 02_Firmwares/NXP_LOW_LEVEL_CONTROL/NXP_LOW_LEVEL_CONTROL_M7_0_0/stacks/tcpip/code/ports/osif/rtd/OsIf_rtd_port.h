/*
 * Copyright 2020 NXP
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

#ifndef OSIF_RTD_PORT_H
#define OSIF_RTD_PORT_H

#include "StandardTypes.h"
#include "OsIf.h"

#if defined(USING_OS_FREERTOS)
/* FreeRTOS implementation */
#include "FreeRTOS.h"
#include "semphr.h"

#define OSIF_WAIT_FOREVER 0xFFFFFFFFu

#if (configSUPPORT_STATIC_ALLOCATION == 1)
typedef struct
{
    SemaphoreHandle_t handle;
    StaticSemaphore_t buffer;
} semaphore_t;

typedef semaphore_t mutex_t;

#else /* configSUPPORT_STATIC_ALLOCATION == 1*/

/*! @brief Type for a mutex. */
typedef SemaphoreHandle_t mutex_t;
/*! @brief Type for a semaphore. */
typedef SemaphoreHandle_t semaphore_t;
#endif /* configSUPPORT_STATIC_ALLOCATION == 1 */

#else /* USING_OS_FREERTOS */

/* Bare-metal implementation */
/*! @brief Type for a mutex. */
typedef uint8_t mutex_t;
/*! @brief Type for a semaphore. */
typedef volatile uint8_t semaphore_t;
#endif /* USING_OS_FREERTOS */

void OsIf_Millisecond(void);

/*!
 * @brief Blocks execution for a number of milliseconds.
 *
 * @param delayMs delay in milliseconds.
 */
void OsIf_TimeDelay(const uint32 delayMs);

/*!
 * @brief Returns the number of miliseconds elapsed since
 * starting the internal timer.
 *
 * @return number of miliseconds elapsed since starting
 * the internal timer.
 */
uint32 OsIf_GetMilliseconds(void);


#if defined(USING_OS_FREERTOS)
/*!
 * @brief Waits for a mutex and locks it.
 *
 * @param[in] pMutex reference to the mutex object
 * @param[in] timeout timeout value in milliseconds
 * @return  One of the possible status codes:
 * - E_OK: mutex lock operation success
 * - E_NOT_OK: mutex already owned by current thread
 *             or timeout
 */
StatusType OsIf_MutexLock(const mutex_t * const pMutex,
                          const uint32 timeout);

/*!
 * @brief Unlocks a previously locked mutex.
 *
 * @param[in] pMutex reference to the mutex object
 * @return One of the possible status codes:
 * - E_OK: mutex unlock operation success
 * - E_NOT_OK: mutex unlock failed
 */
StatusType OsIf_MutexUnlock(const mutex_t * const pMutex);

/*!
 * @brief Create an unlocked mutex.
 *
 * @param[in] pMutex reference to the mutex object
 * @return  One of the possible status codes:
 * - E_OK: mutex created
 * - E_NOT_OK: mutex could not be created
 */
StatusType OsIf_MutexCreate(mutex_t * const pMutex);

/*!
 * @brief Destroys a previously created mutex.
 *
 * @param[in] pMutex reference to the mutex object
 * @return  E_OK status code
 */
StatusType OsIf_MutexDestroy(const mutex_t * const pMutex);

/*!
 * @brief Decrement a semaphore with timeout.
 *
 * @param[in] pSem reference to the semaphore object
 * @param[in] timeout time-out value in milliseconds
 * @return  One of the possible status codes:
 * - E_OK: semaphore wait operation success
 * - E_NOT_OK: semaphore wait timed out
 */
StatusType OsIf_SemaWait(semaphore_t * const pSem,
                         const uint32 timeout);

/*!
 * @brief Increment a semaphore
 *
 * @param[in] pSem reference to the semaphore object
 * @return  One of the possible status codes:
 * - E_OK: semaphore post operation success
 * - E_NOT_OK: semaphore could not be incremented
 */
StatusType OsIf_SemaPost(semaphore_t * const pSem);

/*!
 * @brief Creates a semaphore with a given value.
 *
 * @param[in] pSem reference to the semaphore object
 * @param[in] initValue initial value of the semaphore
 * @return  One of the possible status codes:
 * - E_OK: semaphore created
 * - E_NOT_OK: semaphore could not be created
 */
StatusType OsIf_SemaCreate(semaphore_t * const pSem,
                           const uint8 initValue);

/*!
 * @brief Destroys a previously created semaphore.
 *
 * @param[in] pSem reference to the semaphore object
 * @return E_OK status code
 */
StatusType OsIf_SemaDestroy(const semaphore_t * const pSem);
#endif /* USING_OS_FREERTOS */
#endif /* OSIF_RTD_PORT_H */
