#include "touch.h"
#include <stdio.h>
#include "UMDLPC/util/util.h"

/* Calibration values */
static const uint32_t MIN_X = 0x210, MAX_X = 0x1dd0,
                      MIN_Y = 0x2a0, MAX_Y = 0x1f00;

void touch_init(void) {
  // Out is in and in is out!
  // (As in, the touch controller's in/out)
  TOUCH_IN_OUTPUT();
  TOUCH_CLK_OUTPUT();
  TOUCH_OUT_INPUT();
  TOUCH_PENIRQ_INPUT();
}

__attribute__((always_inline))
void static _pulse_delay() {
  __NOP();
  __NOP();
  __NOP();
}

__attribute__((always_inline))
void static _clock() {
  TOUCH_CLK_ON();
  _pulse_delay();
  TOUCH_CLK_OFF();
  _pulse_delay();
}

__attribute__((always_inline))
void static _clock_rev() {
  TOUCH_CLK_OFF();
  _pulse_delay();
  TOUCH_CLK_ON();
  _pulse_delay();
}

void touch_write_data(uint_fast8_t data) {
  TOUCH_CLK_OFF();

  for (uint_fast8_t mask = 0x80; mask; mask >>= 1)
  {
    if (data & mask) {
      TOUCH_IN_ON();
    } else {
      TOUCH_IN_OFF();
    }

    _clock_rev();
  }

  TOUCH_CLK_OFF();
}

uint16_t touch_read_data(void) {
  uint16_t data = 0;

//  while (!(TOUCH_BUSY_READ()));

  for(uint_fast8_t count = 12; count; --count)
  {
    _clock();

    if (TOUCH_OUT_READ()) {
      data |= 1;
    }

    data <<= 1;
  }

  return data;
}

__attribute__((always_inline))
void static touch_single_read(uint16_t *out_x, uint16_t *out_y) {
  touch_write_data(0x90);

  __NOP(); __NOP(); __NOP(); __NOP();

  *out_x = touch_read_data();

  touch_write_data(0xD0);

  __NOP(); __NOP(); __NOP(); __NOP();

  *out_y = touch_read_data();
}

uint_fast8_t touch_read(uint16_t *out_x, uint16_t *out_y) {
  uint16_t temp_x, temp_y;
  uint32_t tx = 0, ty = 0;

  // Throwaway reading: (first ADC reading is usually inaccurate)
  touch_single_read(&temp_x, &temp_y);

  uint_fast8_t datacount = 0;

  for (uint_fast8_t i = 8; i; --i) {
    touch_single_read(&temp_x, &temp_y);

    // 0 values usually mean the pen was lifted up while we were sampling
    if (temp_y && temp_x) {
      ty += temp_x;
      tx += temp_y;

      ++datacount;
    }
  }

  if (datacount && tx && ty) {
    tx /= datacount;
    ty /= datacount;

    if (tx <= MIN_X) {
      tx = 0;
    } else {
      tx -= MIN_X;

      tx *= 240;
      tx /= (MAX_X - MIN_X);
    }

    if (ty <= MIN_Y) {
      ty = 0;
    } else {
      ty -= MIN_X;

      ty *= 320;
      ty /= (MAX_Y - MIN_Y);
    }

    if (tx > 239) {
      tx = 239;
    }

    if (ty > 319) {
      ty = 319;
    }

    *out_x = tx;
    *out_y = 319 - ty;

    return 1;
  }

  return 0;
}

uint_fast8_t touch_available(void) {
  return !(TOUCH_PENIRQ_READ());
}
