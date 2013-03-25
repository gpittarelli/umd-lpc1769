/*
 ===============================================================================
 Name        : main.c
 Author      : 
 Version     :
 Copyright   : Copyright (C) 
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include <stdio.h>

#include "main.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

// DMA DAC Buffer
__attribute__ ((section(".ahb_ram")))
static uint32_t dac_dma_buffer[DMA_BUFFER_LEN] = {
    0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0,
    0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0,
    0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0,
    0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0,
    0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0, 0, 0xFFC0,
    0, 0xFFC0, 0, 0xFFC0
    /*
    0x8000, 0x8C80, 0x9900, 0xA500, 0xB100, 0xBC40,
    0xC700, 0xD100, 0xDA40, 0xE2C0, 0xEA40, 0xF0C0, 0xF600, 0xFA40, 0xFD40,
    0xFF40, 0xFFC0, 0xFF40, 0xFD40, 0xFA40, 0xF600, 0xF0C0, 0xEA40, 0xE2C0,
    0xDA40, 0xD100, 0xC700, 0xBC40, 0xB100, 0xA500, 0x9900, 0x8C80, 0x8000,
    0x7380, 0x6700, 0x5B00, 0x4F00, 0x43C0, 0x3900, 0x2F00, 0x25C0, 0x1D40,
    0x15C0, 0x0F40, 0x0A00, 0x05C0, 0x02C0, 0x00C0, 0x0040, 0x00C0, 0x02C0,
    0x05C0, 0x0A00, 0x0F40, 0x15C0, 0x1D40, 0x25C0, 0x2F00, 0x3900, 0x43C0,
    0x4F00, 0x5B00, 0x6700, 0x7380

    0x8000, 0x8C80, 0x9900, 0xA500, 0xB100, 0xBC40,
    0xC700, 0xD100, 0xDA40, 0xE2C0, 0xEA40, 0xF0C0, 0xF600, 0xFA40, 0xFD40,
    0xFF40, 0xFFC0, 0xFF40, 0xFD40, 0xFA40, 0xF600, 0xF0C0, 0xEA40, 0xE2C0,
    0xDA40, 0xD100, 0xC700, 0xBC40, 0xB100, 0xA500, 0x9900, 0x8C80, 0x8000,
    0x7380, 0x6700, 0x5B00, 0x4F00, 0x43C0, 0x3900, 0x2F00, 0x25C0, 0x1D40,
    0x15C0, 0x0F40, 0x0A00, 0x05C0, 0x02C0, 0x00C0, 0x0040, 0x00C0, 0x02C0,
    0x05C0, 0x0A00, 0x0F40, 0x15C0, 0x1D40, 0x25C0, 0x2F00, 0x3900, 0x43C0,
    0x4F00, 0x5B00, 0x6700, 0x7380 */
};

// DMA Linked List Nodes, in AHB SRAM
// Two nodes which we will point to each other so they continually
// go back and forth
__attribute__ ((section(".ahb_ram")))
static DMALinkedListNode dmaLLNode;

uint_fast16_t pos = 0;
void ADC_IRQHandler(void) {
  /*
  uint_fast16_t analog_val = (LPC_ADC ->ADDR0 >> 4) & 0x0fff;

  // ADC is 12 bits, but DAC is only 10, so chop off the bottom 2 bits
  analog_val >>= 2;

  // Write analog_val to DAC
  LPC_DAC ->DACR = ((LPC_DAC ->DACR) & ~(0x3ff << 6)) | (analog_val << 6);
  */
  // Equivalent to above (slight efficiency improvement):

  //asm volatile("bfc.w r2, #6, #10");
  //LPC_DAC ->DACR &= ~(0x3ff << 6);
  //LPC_DAC ->DACR = (LPC_ADC ->ADDR0 & (0x03ff << 6));

  /* Write to next pos in dma buffer: */
  //dac_dma_buffer[pos & (DMA_BUFFER_LEN - 1)] = (LPC_ADC ->ADDR0 & (0x03ff << 6));
  uint_fast16_t volatile analog_val = (LPC_ADC ->ADDR0 >> 4) & 0x0fff;
  analog_val = (analog_val - 0) * (0xffff - 200) / (0xfff - 0) + 200;
  LPC_DAC->DACCNTVAL = analog_val;
  //++pos;
}

int main(void) {
  int j;
  for (j=0; j < DMA_BUFFER_LEN; ++j) {
    dac_dma_buffer[j] = (j%2)? 0xFFC0 : 0x0000;
  }

  // Select 12MHz crystal oscillator
  LPC_SC ->CLKSRCSEL = 1;

  // PLL 0 Setup
  // M(multipler) value is 147
  // N(divider) value is 8
  // F_CCO = (2 * 147 * 12MHz)/8 = 441MHz
  LPC_SC ->PLL0CFG = (147 - 1) | ((8 - 1) << 16);

  // F_CCO = (2 * 96 * 12MHz)/6 = 384MHz
  //LPC_SC ->PLL0CFG = (96 - 1) | ((6 - 1) << 16);

  // Enable PLL0
  LPC_SC ->PLL0CON = 0x01;

  LPC_SC ->PLL0FEED = 0xAA;
  LPC_SC ->PLL0FEED = 0x55;

  // Wait for PLL0 Lock
  while (!(LPC_SC ->PLL0STAT & (1 << 26)))
    ;

  // Connect PLL0
  LPC_SC ->PLL0CON = 0x03;

  LPC_SC ->PLL0FEED = 0xAA;
  LPC_SC ->PLL0FEED = 0x55;

  // Set clock divider to 12, so final running clock speed will be:
  // (441 / 100) = 4.41MHz
  LPC_SC ->CCLKCFG = (100 - 1);

  // Peripheral power
  //  Power ADC (bit 12)
  //  (DAC always powered)
  //  GPDMA (bit 29)
  LPC_SC ->PCONP |= _BV(12) | _BV(29);

  // Choose undivided peripheral clock for:
  //                    ADC (CLK), DAC (CLK/8)
  LPC_SC ->PCLKSEL0 |= (1 << 22) | (1 << 24);

  // Setup IO pins (not used currently, handy to keep around)
  LPC_GPIO0 ->FIODIR = _BV(22);
  LPC_GPIO0 ->FIOSET = _BV(22);

  // Configure pins
  //   P0.23 as AD0.0 (1 at bit 14)
  //   P0.26 as AOUT  (2 at bit 20)
  LPC_PINCON ->PINSEL1 |= (1 << 14) | (2 << 20);

  // DMA Configuration register
  //   Enable DMA (1 at bit 0)
  //   Little-endian (default, 0 at bit 1)
  LPC_GPDMA->DMACConfig = _BV(0);

  // A/D Control Register
  //  1 in bit 0 - Select AD0.0 to be sampled
  //  0 in bits 15:8 - Don't divide clock
  //  0 in bit 16 - Disable burst mode (enabled later)
  //  1 in bit 21 - Not in power-down mode
  //  0 in bits 26:24 - don't start a conversion yet
  LPC_ADC ->ADCR = _BV(0) | _BV(21);

  // A/D Interrupt Enable Register
  //  1 in bit 0 - Interrupt when conversion on ADC channel 0 completes
  LPC_ADC ->ADINTEN = _BV(0);

  // DAC Control Register
  //  Enable counter (1 at bit 2)
  //  Enable DMA (1 at bit 3)
  LPC_DAC->DACCTRL = _BV(2) | _BV(3);

  // DAC Counter Value
  //  32MHz / 8 = 4MHz
  //  4MHz / 0xffff = 61.04...Hz
  LPC_DAC->DACCNTVAL = 49;

  // Setup DMA linked list
  dmaLLNode.sourceAddr = (unsigned long int) dac_dma_buffer;
  dmaLLNode.destAddr = (unsigned long int) &LPC_DAC->DACR;
  dmaLLNode.nextNode = (unsigned long int) (&dmaLLNode) & 0xFFFFFFFC;
  // Linked List contorol information (Same as DMA channel control register)
  //  Transfer size: 64 (DMA_BUFFER_LEN, bits 11:0)
  //  Source burst size: 1 (0 (default), bits 14:12)
  //  Destination burst size: 1 (0 (default), bits 17:15)
  //  Source transfer width: halfword (1, bits 20:18)
  //  Destination transfer width: halfword (1, bits 23:21)
  //  Source increment: increment (1, bit 26)
  //  Destination increment: don't increment (0, bit 27)
  //  Terminal count interrupt: disabled (0, bit 31)
  dmaLLNode.dmaControl = DMA_BUFFER_LEN | (1 << 18) | (1 << 21) | _BV(26);

  // DMA Channel 0, initialize to linked list for initial DMA
  LPC_GPDMACH0->DMACCSrcAddr = dmaLLNode.sourceAddr;
  LPC_GPDMACH0->DMACCDestAddr = dmaLLNode.destAddr;
  LPC_GPDMACH0->DMACCControl = dmaLLNode.dmaControl;
  LPC_GPDMACH0->DMACCLLI = dmaLLNode.nextNode;

  // DMA Channel 0 Config
  //  Enable channel (1 at bit 0)
  //  Source peripheral: 0 (default, bits  5:1)
  //  Destination peripheral: DAC (7, bits  10:6)
  //  Transfer Type: memory-to-peripheral (1, bits 13:11)
  LPC_GPDMACH0->DMACCConfig = (7 << 6) | (1 << 11);
  LPC_GPDMACH0->DMACCConfig |= _BV(0);

  volatile int i = 0;
  for (;;) {
    if (i >= 1000000) {
      LPC_GPIO0 ->FIOPIN ^= (1 << 22);
      i = 0;
    }
    ++i;
  }
  return 0;
}
