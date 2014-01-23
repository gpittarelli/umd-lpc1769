#include "hd44780.h"

#define CLOCK_DELAY 100

__attribute__((always_inline))
void static _delay_loop_2(uint16_t __count) {
  while (__count--) {
    __asm__ volatile (
       "nop" "\n"
       "nop"
       ::
      );
  }
}

__attribute__ ((always_inline))
void static _delay_ms(uint_fast16_t __ms) {
  uint16_t __ticks;
  double __tmp;
  __tmp = ((SystemCoreClock) / 4e3) * __ms;
  if (__tmp < 1.0)
    __ticks = 1;
  else if (__tmp > 65535)
  {
    //      __ticks = requested delay in 1/10 ms
    __ticks = (uint16_t) (__ms * 10.0);
    while(__ticks)
    {
      // wait 1/10 ms
      _delay_loop_2(((SystemCoreClock) / 4e3) / 10);
      __ticks --;
    }
    return;
  }
  else
    __ticks = (uint16_t)__tmp;
  _delay_loop_2(__ticks);
}

void LCD_write_command(uint_fast8_t command) {
  LPC_GPIO1->FIOPIN = command << 18;

  LCD_CLK_OFF();
  _delay_loop_2(CLOCK_DELAY);
  LCD_CLK_ON();
  _delay_loop_2(CLOCK_DELAY);
  LCD_CLK_OFF();
  _delay_loop_2(CLOCK_DELAY * 37);
}

void LCD_init(void) {
  LCD_RS_OUTPUT();
  LCD_RW_OUTPUT();
  LCD_CLK_OUTPUT();

  // Set all data bus pins output, off
  LPC_GPIO1->FIODIR |= 0xff << 18;
  LPC_GPIO1->FIOCLR |= 0xff << 18;

  _delay_ms(100);

  LCD_RS_OFF();
  LCD_RW_OFF();
  LCD_CLK_OFF();

  _delay_loop_2(CLOCK_DELAY);

  LCD_function_set(BUS_WIDTH_8, LINE_COUNT_2, FONT_5_8);
  _delay_ms(5);

  LCD_function_set(BUS_WIDTH_8, LINE_COUNT_2, FONT_5_8);
  _delay_ms(1);

  LCD_function_set(BUS_WIDTH_8, LINE_COUNT_2, FONT_5_8);
  LCD_function_set(BUS_WIDTH_8, LINE_COUNT_2, FONT_5_8);

  LCD_display_settings(DISPLAY_ON, CURSOR_ON, CURSOR_BLINK_ON);

  LCD_clear();

  LCD_mode(RIGHT_TO_LEFT, DISPLAY_NO_SHIFT);
}

void LCD_clear() { LCD_write_command(1); }
void LCD_cursor_home() { LCD_write_command(2); }

void LCD_move_cursor(uint_fast8_t x, uint_fast8_t y) {
  LCD_set_DDRAM(x + (0x40 * y));
}

void LCD_mode(LCDCharacterDirection dir, LCDDisplayShift shift) {
  LCD_write_command((1 << 2) | dir | shift);
}

void LCD_display_settings(LCDPower power, LCDCursor cursor,
                          LCDCursorBlink cursor_blink) {
  LCD_write_command((1 << 3) | power | cursor | cursor_blink);
}

void LCD_display_shift(LCDCursorMove move_or_shift,
                       LCDShiftDirection dir) {
  LCD_write_command((1 << 4) | move_or_shift | dir);
}

void LCD_function_set(LCDBusWidth bus_width, LCDLineCount line_count,
                      LCDFont font) {
  LCD_write_command((1 << 5) | bus_width | line_count | font);
}

void LCD_set_CGRAM(uint_fast8_t addr) {
  LCD_write_command((1 << 6) | addr);
}

void LCD_set_DDRAM(uint_fast8_t addr) {
  LCD_write_command((1 << 7) | addr);
}

void LCD_write(uint_fast8_t data) {
  LCD_RS_ON();
  LCD_write_command(data);
  LCD_RS_OFF();
}
