#ifndef SPI_H
#define SPI_H

#include <stdlib.h>

#include "../inc/LPC17xx.h"
#include "../inc/core_cm3.h"

#include "UMDLPC/util/util.h"

#define SSP_RNE (1<<2)
#define SSP_BSY (1<<4)

void spi_init();
void spi_txrx(uint8_t* tx, uint8_t* rx, uint16_t len);

#endif
