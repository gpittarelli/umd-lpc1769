/* clocking.h
 *
 * Declares handy utility functions for configuring the clock, via the
 * Phase Locked Loop (PLL). Refer to chapter 4 (4.5 in particular) for
 * more information.
 *
 */

#ifndef __UMDLPC_system_clocking_h_
#define __UMDLPC_system_clocking_h_

#include "LPC17xx.h"
#include <cr_section_macros.h>
#include <NXP/crp.h>
#include <stdint.h>

#include "assert.h"

/* PLL_init(m, n, cclkdiv)
 * Takes multiplier (m), divider (n) and CPU clock divider (cclkdiv)
 * and initializes the PLL with those settings.
 */
void PLL_init(uint_fast16_t m, uint_fast16_t n, uint_fast16_t cclkdiv);

/* PLL_bypass()
 * Disconnect the PLL
 */
void PLL_bypass();

enum ClockSource {
  INTERNAL_RC = 0,
  MAIN_OSCILLATOR = 1,
  RTC_OSCILLATOR = 2
};

#endif
