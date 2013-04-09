/*
 ===============================================================================
 Name        : sd_driver_test.c
 Author      : George Pittarelli
 Version     :
 Description :

 Uses the provided
 ===============================================================================
 */

#include "sd_driver_test.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

uint8_t block1[SD_BLOCK_LEN], block2[SD_BLOCK_LEN];

int main(void) {
  volatile uint32_t i = 0;
  volatile char c;

  // Select 12MHz crystal oscillator
  LPC_SC ->CLKSRCSEL = 1;

  // Set PLL 0 to give us 64MHz
  PLL_init(16, 1, 6);

  // Setup GPIO pins
  STATUS_LED_OUTPUT();

  // Select pin modes
  // LPC_PINCON->PINSEL1 = 0;

  for (; i < CLOCK_SPEED/20; ++i) {

  }

  c = sd_init();

  for (i = 0; i < CLOCK_SPEED/20; ++i) {

  }

  sd_read_block(block1, 0);
  sd_read_block(block2, 1);

  while (1) {
    if (i > CLOCK_SPEED/2) {
      STATUS_LED_TOGGLE();
      i = 0;
    }
    i++;
  }
  return 0;
}
