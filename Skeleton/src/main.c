/*
 ===============================================================================
 Name        : main.c
 Author      :
 Version     :
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

#include "main.h"
#include "UMDLPC/system/clocking.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

int main(void) {
  // Select 12MHz crystal oscillator
  LPC_SC ->CLKSRCSEL = 1;

  // Bypass PLL 0
  PLL_bypass();

  // Peripheral power
  // LPC_SC->PCONP = 0;

  // Peripheral clocks
  // LPC_SC->PCLKSEL0 = 0;
  // LPC_SC->PCLKSEL1 = 0;

  // Setup GPIO pins
  // LPC_GPIO0->FIODIR = 0;
  // LPC_GPIO0->FIOSET = 0;

  // Select pin modes
  // LPC_PINCON->PINSEL1 = 0;

  while (1) {

  }
  return 0;
}
