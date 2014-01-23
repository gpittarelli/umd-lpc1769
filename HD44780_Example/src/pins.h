#ifndef __PINS_h_
#define __PINS_h_

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "UMDLPC/util/pins.h"

DEFINE_PIN(LCD_RS, 0, 9);
DEFINE_PIN(LCD_RW, 0, 8);
DEFINE_PIN(LCD_CLK, 0, 7);

#endif
