#ifndef __SSD1289_h_
#define __SSD1289_h_

/* Provides uintN_t, uint_fastN_t, etc. (for N {8,16,32}) */
#include <stdint.h>

#include "pins.h"

void TFT_init(void);

void TFT_write_command(uint16_t command);
void TFT_write_data(uint16_t data);
void TFT_write_command_data(uint16_t command, uint16_t data);
void TFT_write_address(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void TFT_fill(uint16_t color);
void TFT_draw_box(uint16_t x1, uint16_t y1, uint16_t x2,
                  uint16_t y2, uint16_t color);
void TFT_box_outline(uint16_t x1, uint16_t y1, uint16_t x2,
                     uint16_t y2, uint16_t width, uint16_t color);
void TFT_dot(uint16_t x, uint16_t y, uint16_t color);

void TFT_string(const uint8_t *font, const char * ch,
                uint16_t x, uint16_t y,
                uint16_t fg_color, uint16_t bg_color,
                uint16_t width, uint16_t height);
void TFT_char(const uint8_t *font, uint8_t ch,
                uint16_t x, uint16_t y,
                uint16_t fg_color, uint16_t bg_color,
                uint16_t width, uint16_t height);

void TFT_shift_test();

#endif
