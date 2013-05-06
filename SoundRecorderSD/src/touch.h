#ifndef __TOUCH_h
#define __TOUCH_h

#include <stdint.h>

#include "pins.h"

void touch_init(void);

uint_fast8_t touch_read(uint16_t *out_x, uint16_t *out_y);
uint_fast8_t touch_available(void);

#endif
