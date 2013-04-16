/*
 ===============================================================================
 Name        : sd_dma.c
 Author      : George Pittarelli
 Version     :
 Description :

   Demonstrates reading from an SD Card with DMA from the SSP0
   interface.

 ===============================================================================
 */

#include "sd_dma.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

uint8_t block1[SD_BLOCK_LEN], block2[SD_BLOCK_LEN * 4];

LPC_GPDMACH_TypeDef * const SD_DMA_CHANNEL = LPC_GPDMACH0;

int main(void) {
  volatile uint32_t i = 0;
  volatile char c;

  // Select 12MHz crystal oscillator
  LPC_SC ->CLKSRCSEL = 1;

  // Set PLL 0 to give us 64MHz
  PLL_init(16, 1, 6);
  SystemCoreClockUpdate();

  // Power GPDMA
  LPC_SC->PCONP |= PC_GPDMA;

  // Setup GPIO pins
  STATUS_LED_OUTPUT();

  for (; i < SystemCoreClock/20; ++i) {}
    ;

  c = sd_init();

  for (i = 0; i < SystemCoreClock/20; ++i) {}
    ;

  sd_read_block(block1, 0);

  sd_read_block_dma(block2, 1, SD_DMA_CHANNEL);

  block2[0]++;

  sd_write_block_dma(block2, 2, SD_DMA_CHANNEL);

  sd_read_block(block1, 2);

  while (1) {
    if (i > CLOCK_SPEED/2) {
      STATUS_LED_TOGGLE();
      i = 0;
    }
    i++;
  }
  return 0;
}
