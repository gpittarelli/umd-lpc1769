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

static void inline spi_write_nowait(uint_fast8_t byte) {
  LPC_SSP0->DR = byte;
}

void spi_write(uint_fast8_t byte) {
  spi_write_nowait(byte);

  // Wait for FIFO output to be complete
  while (!(LPC_SSP0->SR & _BV(0)))
    ;
}

void sd_command(uint_fast8_t command, uint_fast32_t arguments) {
  // Write command:
  spi_write_nowait((1 << 7) | command);

  spi_write_nowait(arguments >> 24);
  spi_write_nowait(arguments >> 16);
  spi_write_nowait(arguments >> 8);
  spi_write_nowait(arguments);

  // the last byte is (CRC << 1) | 1
  // this value is precomupted to have the right CRC for the first packet,
  // which is the only packet that will be CRC-verified
  spi_write(0x95);
}

static uint32_t systick_delay_timer = 0;
void SysTick_Handler (void) /* SysTick Interrupt Handler (1ms)   */
{
  BUILTIN_LED_TOGGLE();
  if (systick_delay_timer){
    --systick_delay_timer;
  }
}

void systick_delay_millis(uint32_t millis)
{
  systick_delay_timer = millis;
  while (systick_delay_timer);
}

int main(void) {
  // Run at 24MHz, from the 12MHz crystal oscillator
  // (remember to update CLOCK_SPEED)
  LPC_SC->CLKSRCSEL = MAIN_OSCILLATOR;
  PLL_init(12, 1, 12);

  SysTick_Config(SystemCoreClock/100 - 1);

  // Peripheral power
  LPC_SC->PCONP = PC_SSP0;

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
  // We'll control the SD card CS manually (vs letting SSP0 control it)
  SPI_CS_OUTPUT();
  BUILTIN_LED_OUTPUT();

  // SSP0 Control Register 0
  //   8-bit transfers (7 at 3:0)
  //   SPI (0 at 5:4)
  LPC_SSP0->CR0 = 7;

  // SSP0 Prescaler
  // The SD spec requires a slow start at 400khz
  LPC_SSP0->CPSR = CLOCK_SPEED / 400000;

  systick_delay_millis(1);

  // SPI Control Register 1
  //   Defaults to Master
  //   Start serial communications (bit 1)
  LPC_SSP0->CR1 |= _BV(1);

  SPI_CS_ASSERT();

  // Clock the sd card at least 74 times before any actual communication
  // (For internal SD card initialization)
  for (uint_fast8_t i=0; i < 10; ++i) {
    spi_write(0xFF);
  }

  SPI_CS_DEASSERT();

  // Delay 16 more clocks
  spi_write_nowait(0xFF);
  spi_write(0xFF);

  spi_write(CMD0);


  uint32_t i = 0;
  while (1) {
    spi_write(i++);
  }
  return 0;
}
