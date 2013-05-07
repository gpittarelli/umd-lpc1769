#ifndef __PINS_h_
#define __PINS_h_

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "UMDLPC/util/pins.h"

DEFINE_PIN(LCD_RS, 3, 25);
DEFINE_PIN(LCD_RW, 3, 26);
DEFINE_PIN(LCD_CLK, 4, 28);

#endif
