/*
 * Copyright 2021 NXP
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
* @file netif_shutdown.c
*/
#include <string.h>

#include "netif_shutdown.h"
#include "netifcfg.h"

#if defined(USBIF)
#include "usbif.h"
#elif defined(GMACIF_NUMBER)
#include "gmacif.h"
#elif defined (ETH_43_ETHIF_NUMBER)
#include "ethif_port.h"
#else /* GMACIF_NUMBER */
#include "enetif.h"
#endif /* GMACIF_NUMBER */

/* Variables */

#if (defined(CPU_CORTEX_M7) || defined(CPU_CORTEX_M33) || defined(CPU_CORTEX_M4F))
    #define BREAKPOINT_INSTR        "BKPT #0"
#elif (defined(CPU_CORTEX_R52))
    #define BREAKPOINT_INSTR        "HLT #0"
#elif (defined(__GNUC__) && ((defined(__powerpc__) || defined(__ppc__) || defined(__PPC__))))
    #define BREAKPOINT_INSTR        "se_illegal"
#else
    #error "Unsupported architecture!"
#endif

void set_software_breakpoint()
{
  __asm__(BREAKPOINT_INSTR);      /* SW breakpoint to stop debugging */
}

bool is_netif_shutdown_command(char *buf_ptr, uint16_t buf_len)
{
  if ((buf_len == (sizeof(netif_shutdown_cmd) - 1)) && (!strncmp(buf_ptr, netif_shutdown_cmd, buf_len)))
  {
    return true;
  }

  return false;
}

void end_tcpip_execution(struct netif *netif)
{
  if (NULL != netif)
  {
    ETHIF_SHUTDOWN(netif);
  }
  
  set_software_breakpoint();
}
