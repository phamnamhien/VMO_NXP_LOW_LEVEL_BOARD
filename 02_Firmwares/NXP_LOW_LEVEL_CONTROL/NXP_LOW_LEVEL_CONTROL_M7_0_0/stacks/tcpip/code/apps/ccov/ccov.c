/*
 * Copyright 2019 NXP
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
 */
/*!
* @file ccov.c
*/
#include "lwipopts.h"

#if NO_SYS == 0
#include "lwip/igmp.h"
#include "lwip/opt.h"
#include <string.h>
#include "apps/ccov/ccov.h"
#include "apps/netif_shutdown/netif_shutdown.h"
#if LWIP_NETCONN
#include "lwip/api.h"
#include "lwip/sys.h"

#include "netifcfg.h"

#if defined(GMACIF_NUMBER)
#include "gmacif.h"
#elif defined (ETH_43_ETHIF_NUMBER)
#include "ethif_port.h"
#else /* GMACIF_NUMBER */
#include "enetif.h"
#endif /* GMACIF_NUMBER */

#define UDP_MESSAGE_SEND_PORT 18U
#define BOX_SIZE              2U

static const char coverage_test_cmd[]    = "COVERAGE_TEST";
static const char coverage_test_ok_str[] = "COVERAGE TEST OK";

static void CoverageSendTask(void* arg);
static void CoverageRecvTask(void* arg);
static void CoverageRecvTask1(void* arg);
/*-----------------------------------------------------------------------------------*/
/* structure used to save the pointer to netif passed in udpecho_init */
static struct netif *netif_shutdown;

/**
 * Send task for Coverage
 *
 * @param arg unused
 * Start the mutex mbox and other thread needed for Coverage
 * Create a mutex, take a mutex, create a task which takes the same mutex, block it for a while
 * Create a mailbox, create a task which periodically receives the msg, send msg with faster
 * speed in order to overload the mail box
 */
static void CoverageSendTask(void* arg)
{
  TaskHandle_t TaskHandler;
  sys_mutex_t  send_recv_mutex;
  sys_mbox_t   send_recv_box;
  BaseType_t   ret;
  err_t        err;
  uint8_t      i;
  sys_sem_t    *ccov_test_sem = (sys_sem_t*)arg;

  err = sys_mutex_new(&send_recv_mutex);
  LWIP_ASSERT("failed to create send_recv_mutex", err == ERR_OK);
  err = sys_mbox_new(&send_recv_box, BOX_SIZE);
  LWIP_ASSERT("failed to create send_recv_box", err == ERR_OK);

  ret = xTaskCreate(CoverageRecvTask, "RecvTask", 256U, &send_recv_mutex, DEFAULT_THREAD_PRIO, &TaskHandler);
  LWIP_ASSERT("failed to create RecvTask", ret == pdPASS);

#if defined(USING_RTD)
  OsIf_TimeDelay(50U);
#else /* USING_RTD */
  OSIF_TimeDelay(50U);
#endif /* USING_RTD */

  if (send_recv_mutex.mut != NULL)
  {
    sys_mutex_lock(&send_recv_mutex);
#if defined(USING_RTD)
  OsIf_TimeDelay(5000U);
#else /* USING_RTD */
  OSIF_TimeDelay(5000U);
#endif /* USING_RTD */

    sys_mutex_unlock(&send_recv_mutex);
#if defined(USING_RTD)
  OsIf_TimeDelay(1000U);
#else /* USING_RTD */
  OSIF_TimeDelay(1000U);
#endif /* USING_RTD */
  }
  vTaskDelete(TaskHandler);

  ret = xTaskCreate(CoverageRecvTask1, "receiver", 256U, &send_recv_box, DEFAULT_THREAD_PRIO, &TaskHandler);
  LWIP_ASSERT("failed to create receiver task", ret == pdPASS);
  for (i = 0U; i < 10U; i++)
  {
    sys_mbox_post(&send_recv_box, "a");
  }
  vTaskDelete(TaskHandler);

  xTaskCreate(CoverageRecvTask1, "receiver", 256U, &send_recv_box, DEFAULT_THREAD_PRIO, &TaskHandler);
  LWIP_ASSERT("failed to create receiver task", ret == pdPASS);
  for (i = 0U; i < 10U; i++)
  {
    sys_mbox_post_to_front(&send_recv_box, "b");
  }
  vTaskDelete(TaskHandler);

  sys_mutex_free(&send_recv_mutex);

  /* notify the Coverage_task */
  sys_sem_signal(ccov_test_sem);

  for (;;)
  {
#if defined(USING_RTD)
    OsIf_TimeDelay(1000U);
#else /* USING_RTD */
    OSIF_TimeDelay(1000U);
#endif /* USING_RTD */
  }

  sys_mbox_free(&send_recv_box);

}

/*-----------------------------------------------------------------------------------*/
static void CoverageRecvTask(void* arg)
{
  sys_mutex_t* send_recv_mutex = (sys_mutex_t*)arg;

#if defined(USING_RTD)
  OsIf_TimeDelay(50U);
#else /* USING_RTD */
  OSIF_TimeDelay(50U);
#endif /* USING_RTD */

  if (send_recv_mutex != NULL)
  {
#if defined(USING_RTD)
  OsIf_TimeDelay(500U);
#else /* USING_RTD */
  OSIF_TimeDelay(500U);
#endif /* USING_RTD */
    sys_mutex_lock(send_recv_mutex);
    sys_mutex_unlock(send_recv_mutex);
  }

  for (;;)
  {
#if defined(USING_RTD)
    OsIf_TimeDelay(1000U);
#else /* USING_RTD */
    OSIF_TimeDelay(1000U);
#endif /* USING_RTD */
  }
}

/* Dummy handler for packets, returns a different value from forward frame, as it is not
 * processed here.
 */
static unsigned int dummy_ethif_handler(uint8_t eth_instance, ETHIF_BUFFER_t *buff)
{
  /* to avoid warnings */
  (void) eth_instance;
  (void) buff;

  /* Do not return forward frame, as it will discard packets */
  return FORWARD_FRAME + 1U;
}

/* additional protocol network handler */
static rx_buff_process_condition_handler_t handler = &dummy_ethif_handler;

/*-----------------------------------------------------------------------------------*/
static void CoverageRecvTask1(void* arg)
{
  uint8_t i;
  uint32_t buff;
  uint32_t *buffptr = &buff;
  sys_mbox_t* send_recv_box = (sys_mbox_t*)arg;

#if defined(USING_RTD)
    OsIf_TimeDelay(20000U);
#else /* USING_RTD */
    OSIF_TimeDelay(20000U);
#endif /* USING_RTD */
  for (i = 0U; i < 10U; i++)
  {
    sys_arch_mbox_fetch(send_recv_box, (void **)&buffptr, 5000U);
#if defined(USING_RTD)
    OsIf_TimeDelay(50U);
#else /* USING_RTD */
    OSIF_TimeDelay(50U);
#endif /* USING_RTD */
  }

  for (;;)
  {
#if defined(USING_RTD)
    OsIf_TimeDelay(1000U);
#else /* USING_RTD */
    OSIF_TimeDelay(1000U);
#endif /* USING_RTD */
  };
}
/*-----------------------------------------------------------------------------------*/

/**
 * Main loop task for Coverage
 *
 * @param arg unused
 * Continuously check for UDP msg from computer
 * when receive COVERAGE_TEST start task for ethernet interface Coverage
 * when receive NETIF_SHUTDOWN turn off the netif for netif shut down fcn test
 * binded to port 18 (UDP_MESSAGE_SEND_PORT)
 */
static void Coverage_thread(void *arg)
{
  sys_sem_t ccov_test_sem;
  struct netconn *conn;
  struct netbuf *buf;
  struct netbuf *send_buf;
  TaskHandle_t TaskHandler = NULL;
  err_t err;
  BaseType_t ret;
  char *buf_ptr;
  uint16_t buf_len;
  LWIP_UNUSED_ARG(arg);

#if LWIP_IPV6
  conn = netconn_new(NETCONN_UDP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, UDP_MESSAGE_SEND_PORT);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_UDP);
  netconn_bind(conn, IP_ADDR_ANY, UDP_MESSAGE_SEND_PORT);
#endif /* LWIP_IPV6 */

  LWIP_ERROR("udpecho: invalid conn", (conn != NULL), return;);

  for (;;)
  {
    /* Polling for msg from connection */
    err = netconn_recv(conn, &buf);
    if (err == ERR_OK)
    {
      /* no need netconn_connect here, since the netbuf contains the address */
      err = netbuf_data(buf, (void**)&buf_ptr, &buf_len);
      LWIP_ASSERT("netbuf_data failed", err == ERR_OK);

      if ((buf_len == (sizeof(coverage_test_cmd) - 1)) && (!strncmp(buf_ptr, coverage_test_cmd, buf_len)))
      {
        /* if receive message "COVERAGE_TEST", start to overload the mailbox */
        err = sys_sem_new(&ccov_test_sem, 0U);
        LWIP_ASSERT("failed to create ccov_test_sem", err == ERR_OK);

        ETHIF_REGISTER_RX_BUFF_PROCESS_CONDITION_HANDLER(handler);

        /* start thread */
        ret = xTaskCreate(CoverageSendTask, "SendTask", 256U, &ccov_test_sem, DEFAULT_THREAD_PRIO, &TaskHandler);
        LWIP_ASSERT("failed to create SendTask", ret == pdPASS);

        #if defined(S32NZ55)
          /* PrefetchAbort_Handler occurs when using sys_sem_wait() function, workaround with S32N55 platform */
        #else
          /* wait for coverage task to finish notify its ending */
          err = sys_sem_wait(&ccov_test_sem);
        #endif
        
        sys_sem_free(&ccov_test_sem);
        vTaskDelete(TaskHandler);
        TaskHandler = NULL;

        /* notify the remote side about the status of the coverage testing */
        send_buf = netbuf_new();
        LWIP_ASSERT("netbuf_new failed", send_buf != NULL);

        err = netbuf_ref(send_buf, coverage_test_ok_str, sizeof(coverage_test_ok_str) - 1);
        LWIP_ASSERT("netbuf_take failed", err == ERR_OK);

        err = netconn_sendto(conn, send_buf, netbuf_fromaddr(buf), netbuf_fromport(buf));
        LWIP_ASSERT("netbuf_sendto failed", err == ERR_OK);

        netbuf_delete(send_buf);
      }
      else if (is_netif_shutdown_command(buf_ptr, buf_len))
      {
        /* if receive message "NETIF_SHUTDOWN", call function ETHIF_SHUTDOWN() with the
         * netif used for this echo server
         */
        send_tx_pbuffs_dummy_char();
        // send_rx_pbuffs_dummy_char();
        ETHIF_REGISTER_RX_BUFF_PROCESS_CONDITION_HANDLER(NULL);

        netbuf_delete(buf);
        buf_ptr = NULL;
        buf_len = 0;
        
        sys_arch_protect();
        end_tcpip_execution(netif_shutdown);
        sys_arch_unprotect(0);
      }

      netbuf_delete(buf);
      buf_ptr = NULL;
      buf_len = 0;
    }
  }
}

/*-----------------------------------------------------------------------------------*/
void Coverage_init(struct netif *netif)
{
  netif_shutdown = netif;
  sys_thread_new("Coverage_thread", Coverage_thread, NULL, 512, DEFAULT_THREAD_PRIO);
}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
#endif /* NO_SYS == 0 */
