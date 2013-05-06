#ifndef __MAIN_h_
#define __MAIN_h_

/* Provides uintN_t, uint_fastN_t, etc. (for N {8,16,32}) */
#include <stdint.h>

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

#include "UMDLPC/system/clocking.h"
#include "UMDLPC/util/pins.h"

#include "spi.h"
#include "sd.h"

#define CLOCK_SPEED 64000000

#define _BV(n) (1 << (n))

DEFINE_PIN(STATUS_LED, 0, 22);

#endif
