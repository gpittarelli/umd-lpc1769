/*
 ===============================================================================
 Name        : analogrepeatdma.c
 Author      : George Pittarelli
 Description :

   Uses peripheral-to-peripheral DMA to transfer samples from the ADC
   to the DAC automatically.

 ===============================================================================
 */

#include "analogrepeatdma.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

DMALinkedListNode repeat_node;

void load_dma_node(LPC_GPDMACH_TypeDef *channel, DMALinkedListNode *node) {
  channel->DMACCSrcAddr  = node->sourceAddr;
  channel->DMACCDestAddr = node->destAddr;
  channel->DMACCControl  = node->dmaControl;
  channel->DMACCLLI      = node->nextNode;
}

int main(void) {
  // Select 12MHz crystal oscillator
  LPC_SC->CLKSRCSEL = 1;

  // We need to input and output samples at 44.1khz
  // And the ADC samples with
  // Run PLL 0 at 44.1MHz
  PLL_init(147, 8, 10);
  SystemCoreClockUpdate();

  // Peripheral power (Note: DAC is always powered)
  LPC_SC->PCONP |= PC_ADC | PC_GPDMA;

  // Undivided peripheral clock for DAC, ADC
  LPC_SC->PCLKSEL0 |= (1 << 22) | (1 << 24);

  // Configure pins
  //   P0.23 as AD0.0 (1 at bit 14)
  //   P0.26 as AOUT  (2 at bit 20)
  LPC_PINCON->PINSEL1 = (1 << 14) | (2 << 20);
  BLINKING_LED_OUTPUT();

  // We want to sample at 44.1khz, and a full sample takes 65 cycles,
  // so we want an ADC clock of 44,100*65 = 2,866,500.
  //
  // A/D Control Register
  //  1 in bit 0 - Select AD0.0 to be sampled
  //       bits 15:8 - Set clock divider
  //  0 in bit 16 - Disable burst mode (enabled later)
  //  1 in bit 21 - Not in power-down mode
  //  0 in bits 26:24 - don't start a conversion yet
  LPC_ADC->ADCR = _BV(0) | (((SystemCoreClock/2866500) - 1) << 8)
                 | _BV(16) | _BV(21);

  // A/D Interrupt Enable Register
  //  1 in bit 0 - Interrupt when conversion on ADC channel 0 completes
  LPC_ADC->ADINTEN = _BV(0);

  // DAC Control Register
  //  Enable counter (1 at bit 2)
  //  Enable DMA (1 at bit 3)
  LPC_DAC->DACCTRL = _BV(2) | _BV(3);

  // DAC Counter Value
  //  44.1MHz / 1000 = 44.1kHz
//  LPC_DAC->DACCNTVAL = (1000 - 1);
  LPC_DAC->DACCNTVAL = (1000 - 1);

  // Setup DMA node to circle back on itself
  repeat_node.sourceAddr = (uint32_t) &(LPC_ADC->ADDR0);
  repeat_node.destAddr = (uint32_t) &(LPC_DAC->DACR);
  repeat_node.nextNode = (uint32_t) &repeat_node;

  // Linked List control information (Same as DMA channel control register)
  //  Transfer size: 0xfff (bits 11:0)
  //  Source burst size: 1 (0, bits 14:12)
  //  Destination burst size: 1 (0, bits 17:15)
  //  Source transfer width: word (2, bits 20:18)
  //  Destination transfer width: word (2, bits 23:21)
  //  Source increment: don't increment (0, bit 26)
  //  Destination increment: don't increment (0, bit 27)
  repeat_node.dmaControl = 0xfff | (2 << 18) | (2 << 21);

  // enable DMA
  LPC_GPDMA->DMACConfig = _BV(0);

  // DMA Channel 0 Config (used for playback)
  //  Leave channel disabled (0 at bit 0)
  //  Source peripheral: ADC (4, bits  5:1)
  //  Destination peripheral: DAC (7, bits  10:6)
  //  Transfer Type: peripheral-to-peripheral (3, bits 13:11)
  LPC_GPDMACH0->DMACCConfig = (4 << 1) | (7 << 6) | (3 << 11);

  load_dma_node(LPC_GPDMACH0, &repeat_node);
  LPC_GPDMACH0->DMACCConfig |= _BV(0);

  volatile uint32_t i = 0;
  while (1) {
    if (i >= 10000000) {
      BLINKING_LED_TOGGLE();
    }
    ++i;
  }
  return 0;
}
