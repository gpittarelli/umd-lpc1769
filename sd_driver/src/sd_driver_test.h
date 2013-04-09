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

#include "spi.h"
#include "sd.h"

#define _BV(n) (1 << (n))

#endif
