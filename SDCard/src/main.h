#ifndef __MAIN_h_
#define __MAIN_h_

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include <stdint.h>

#include "sdcomm_spi.h"

#include "UMDLPC/system/clocking.h"
#include "UMDLPC/system/pconp.h"
#include "UMDLPC/util/pins.h"

#define _BV(n) (1 << (n))

DEFINE_PIN(SPI_CS, 0, 15);
DEFINE_PIN(BUILTIN_LED, 0, 22);

#endif
