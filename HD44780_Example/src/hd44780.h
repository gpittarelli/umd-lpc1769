#ifndef __HD44780_h_
#define __HD44780_h_

/* Provides uintN_t, uint_fastN_t, etc. (for N {8,16,32}) */
#include <stdint.h>

#include "pins.h"

/* Parameter enums */
typedef enum {
  LEFT_TO_RIGHT = 0,
  RIGHT_TO_LEFT = (1 << 1)
} LCDCharacterDirection;

typedef enum {
  DISPLAY_NO_SHIFT = 0,
  DISPLAY_SHIFT = 1
} LCDDisplayShift;

typedef enum {
  DISPLAY_OFF = 0,
  DISPLAY_ON = 1 << 2
} LCDPower;

typedef enum {
  CURSOR_OFF = 0,
  CURSOR_ON = 1 << 1
} LCDCursor;

typedef enum {
  CURSOR_BLINK_OFF = 0,
  CURSOR_BLINK_ON = 1
} LCDCursorBlink;

typedef enum {
  MOVE_CURSOR = 0,
  SHIFT_DISPLAY = 1 << 3
} LCDCursorMove;

typedef enum {
  SHIFT_LEFT = 0,
  SHIFT_RIGHT = 1 << 2
} LCDShiftDirection;

typedef enum {
  BUS_WIDTH_4 = 0,
  BUS_WIDTH_8 = 1 << 4
} LCDBusWidth;

typedef enum {
  LINE_COUNT_1 = 0,
  LINE_COUNT_2 = 1 << 3
} LCDLineCount;

typedef enum {
  FONT_5_8 = 0,
  FONT_5_10 = 1 << 2
} LCDFont;

void LCD_init(void);
void LCD_write_command(uint_fast8_t command);
void LCD_clear();
void LCD_cursor_home();
void LCD_mode(LCDCharacterDirection dir, LCDDisplayShift shift);
void LCD_display_settings(LCDPower power, LCDCursor cursor,
                          LCDCursorBlink cursor_blink);
void LCD_display_shift(LCDCursorMove move_or_shift,
                       LCDShiftDirection dir);
void LCD_function_set(LCDBusWidth bus_width, LCDLineCount line_count,
                      LCDFont font);
void LCD_set_CGRAM(uint_fast8_t addr);
void LCD_set_DDRAM(uint_fast8_t addr);

void LCD_write(uint_fast8_t data);

#endif
