/*
 ===============================================================================
 Name        : semihosting.c
 Author      : George Pittarelli
 Version     :
 Description :
    A simple example of using semi hosting to print messages to a debug
    console
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include <stdio.h>
#include <stdint.h>

#include "semihosting.h"
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

  puts("Hello, World!");

  uint32_t i = 0;
  while (1) {
    printf("A number: %u\n", i++);
  }
  return 0;
}
