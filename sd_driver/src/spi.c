#include <stdlib.h>

#include "../inc/LPC17xx.h"
#include "../inc/core_cm3.h"
#include "spi.h"

void spi_init() {
  // Power SSP0
  LPC_SC->PCONP |= (1 << 21);

  // Peripheral clock - select undivided clock for SSP0
  LPC_SC->PCLKSEL1 &= ~(3 << 16);
  LPC_SC->PCLKSEL1 |= (1 << 16);

  // Select pin functions
  //   P0.15 as SCK0 (2 at 31:30)
  LPC_PINCON->PINSEL0 &= ~(3 << 30);
  LPC_PINCON->PINSEL0 |= (2 << 30);

  //   P0.16 as SSEL0 (2 at 1:0)
  //   P0.17 as MISO0 (2 at 3:2)
  //   P0.18 as MOSI0 (2 at 5:4)
  LPC_PINCON->PINSEL1 &= ~(3 | (3 << 2) | (3 << 4));
  LPC_PINCON->PINSEL1 |= 2 | (2 << 2) | (2 << 4);

  // SSP0 Control Register 0
  //   8-bit transfers (7 at 3:0)
  //   SPI (0 at 5:4)
  //   Polarity and Phase default to Mode 0
  LPC_SSP0->CR0 = 7;

  // SSP0 Prescaler
  // The SD spec requires a slow start at 200khz
  LPC_SSP0->CPSR = SystemCoreClock / 200000;

  // SPI Control Register 1
  //   Defaults to Master
  //   Start serial communications (bit 1)
  LPC_SSP0->CR1 |= (1 << 1);
}

void spi_txrx(uint8_t* tx, uint8_t* rx, uint16_t len)
{
	/* Embed: transmit and receive len bytes
	 * Remember:
	 *   SPI transmits and receives at the same time
	 *   If tx == NULL and you are only receiving then transmit all 0xFF
	 *   If rx == NULL and you are only transmitting then dump all recieved bytes
	 */
  volatile uint_fast8_t dummy;
  while (len--) {
    if (tx == NULL) {
      LPC_SSP0->DR = 0xff;
    } else {
      LPC_SSP0->DR = *tx++;
    }

    while ( (LPC_SSP0->SR & (SSP_BSY | SSP_RNE)) != SSP_RNE );

    if (rx == NULL) {
      dummy = LPC_SSP0->DR;
    } else {
      *rx++ = LPC_SSP0->DR;
    }
  }
}
