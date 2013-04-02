/*
 ===============================================================================
 Name        : main.c
 Author      : George Pittarelli
 Version     : 0.1
 Description :
     Read from an SD card over SPI, using the SSP0 peripheral (Chapter 18)
     A useful summary of the SD spec:
     http://alumni.cs.ucr.edu/~amitra/sdcard/Additional/sdcard_appnote_foust.pdf
 ===============================================================================
 */

#include "main.h"

#define CLOCK_SPEED 24000000

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

void sd_command(uint_fast8_t command, uint_fast32_t arguments) {
  // Write command:
  LPC_SSP0->DR = (1 << 7) | command;

  LPC_SSP0->DR = arguments >> 24;
  LPC_SSP0->DR = arguments >> 16;
  LPC_SSP0->DR = arguments >>  8;
  LPC_SSP0->DR = arguments;

  // Valid CRC for the only packet which will use CRC, and the stop bit
  LPC_SSP0->DR = 0x95;
}

int main(void) {
  // Run at 24MHz, from the 12MHz crystal oscillator
  // (remember to update CLOCK_SPEED)
  LPC_SC->CLKSRCSEL = MAIN_OSCILLATOR;
  PLL_init(12, 1, 12);

  // Peripheral power
  LPC_SC->PCONP = PCSSP0;

  // Peripheral clocks
  // LPC_SC->PCLKSEL0 = 0;
  // LPC_SC->PCLKSEL1 = 0;

  // Select pin functions
  //   P0.15 as SCK0 (2 at 31:30)
  LPC_PINCON->PINSEL0 = (2 << 30);
  //   P0.17 as MISO0 (2 at 3:2)
  //   P0.18 as MOSI0 (2 at 5:4)
  LPC_PINCON->PINSEL1 = (2 << 2) | (2 << 4);

  // Setup GPIO pins
  //  LPC_GPIO0->FIODIR = 0;
  //  LPC_GPIO0->FIOSET = 0;
  // SD Card CS pin, which we'll control manually:
  SPI_CS_OUTPUT();

  // SSP0 Control Register 0
  //   8-bit transfers (7 at 3:0)
  //   SPI (0 at 5:4)
  LPC_SSP0->CR0 = 7;

  // SSP0 Prescaler
  // The SD spec requires a slow start at 400khz
  LPC_SSP0->CPSR = CLOCK_SPEED / 400000;

  // SPI Control Register 1
  //   Defaults to Master
  //   Start serial communications (bit 1)
  LPC_SSP0->CR1 |= _BV(1);

  SPI_CS_ASSERT();

  // Clock the sd card at least 74 times before any actual communication
  // (For internal SD card initialization)
  for (uint_fast8_t i=0; i < 74; ++i) {
    // If FIFO is full, wait for space
    while (!(LPC_SSP0->SR & _BV(1)))
      ;

    // Writing any value to DR adds a frame to the fifo.
    // The data doesn't matte, all we want is a packet to be sent, which
    // includes clocking the clock pin
    LPC_SSP0->DR = 0xFF;
  }

  // Wait for FIFO output to be complete
  while (!(LPC_SSP0->SR & _BV(0)))
    ;

  SPI_CS_DEASSERT();

  // Delay 16 more clocks
  for (uint_fast8_t i=0; i < 74; ++i) {
    while (!(LPC_SSP0->SR & _BV(1)))
      ;
    LPC_SSP0->DR = 0xff;
  }


  // Wait for FIFO output to be complete
  while (!(LPC_SSP0->SR & _BV(0)))
    ;

  LPC_SSP0->DR = CMD0;



  while (1) {

  }
  return 0;
}
