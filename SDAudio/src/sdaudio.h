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

#include "sd.h"
#include "pins.h"

#define _BV(n) (1 << (n))

#define CLOCK_SPEED 44100000
#define DMA_LL_POOL_SIZE 64
#define AUDIO_BUFFER_LEN SD_BLOCK_LEN

#endif
