#ifndef __MAIN_h_
#define __MAIN_h_

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

#include <stdint.h>

#include "UMDLPC/system/clocking.h"
#include "UMDLPC/system/pconp.h"
#include "UMDLPC/system/dma.h"
#include "UMDLPC/util/pins.h"
#include "UMDLPC/util/util.h"

#define _BV(n) (1 << (n))

#define CLOCK_SPEED 44100000
#define DMA_LL_POOL_SIZE 64
// ~.125 seconds of audio @ 44.1khz
#define AUDIO_BUFFER_LEN 5012

DEFINE_PIN(BLINKING_LED, 0, 22);

#endif
