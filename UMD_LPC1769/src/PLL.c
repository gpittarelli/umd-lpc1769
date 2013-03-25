/*
 * PLL.c
 *
 *  Created on: Mar 16, 2013
 *      Author: Administrator
 */

#include "PLL.h"

// m should be in range [6, 512]
// n should be in range [1, 128]
// cclkdiv should be in range [1, 256]
void PLL_init(uint_fast16_t m, uint_fast16_t n, uint_fast16_t cclkdiv) {
  // PLL 0 Setup
  // F_CCO = (2 * m * CLOCK_SOURCE_FREQ)/n
  LPC_SC ->PLL0CFG = (m - 1) | ((n - 1) << 16);

  // Enable PLL0
  LPC_SC ->PLL0CON = 0x01;

  LPC_SC ->PLL0FEED = 0xAA;
  LPC_SC ->PLL0FEED = 0x55;

  // Wait for PLL0 Lock
  while (!(LPC_SC ->PLL0STAT & (1 << 26)))
    ;

  LPC_SC ->CCLKCFG = (cclkdiv - 1);

  // Connect PLL0
  LPC_SC ->PLL0CON = 0x03;

  LPC_SC ->PLL0FEED = 0xAA;
  LPC_SC ->PLL0FEED = 0x55;
}
