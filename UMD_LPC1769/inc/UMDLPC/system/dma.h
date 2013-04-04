/* clocking.h
 *
 * Declares handy utility functions for configuring the clock, via the
 * Phase Locked Loop (PLL). Refer to chapter 4 (4.5 in particular) for
 * more information.
 *
 */

#ifndef __UMDLPC_system_dma_h_
#define __UMDLPC_system_dma_h_

typedef struct {
  uint32_t sourceAddr;
  uint32_t destAddr;
  uint32_t nextNode;
  uint32_t dmaControl;
} DMALinkedListNode;

#endif
