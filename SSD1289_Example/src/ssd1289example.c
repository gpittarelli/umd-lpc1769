/*
  ===============================================================================
  Name        : ssd1289example.c
  Author      : George Pittarelli
  Version     :
  Description :
  Provides a simple example of the SSD1289 library
  ===============================================================================
*/

#include "ssd1289example.h"
#include "ssd1289.h"
#include "touch.h"
#include "fonts.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

int main(void) {
  // Select 12MHz crystal oscillator
  LPC_SC ->CLKSRCSEL = 1;

  // Init PLL 0 to 100MHz
  PLL_init(25, 2, 3);
//  PLL_init(12, 1, 18);
  SystemCoreClockUpdate();

  TFT_init();
  touch_init();

  TFT_fill(0xffff);
  TFT_box_outline(2, 2, 91, 22, 2, 0x0000);

  TFT_char(FONT_16x16, 'C', 5, 5, 0, 0xFFFF, 16, 16);
  TFT_char(FONT_16x16, 'l', 22, 5, 0, 0xFFFF, 16, 16);
  TFT_char(FONT_16x16, 'e', 39, 5, 0, 0xFFFF, 16, 16);
  TFT_char(FONT_16x16, 'a', 56, 5, 0, 0xFFFF, 16, 16);
  TFT_char(FONT_16x16, 'r', 73, 5, 0, 0xFFFF, 16, 16);

  uint16_t x, y, box_x1=0xFFFF, box_x2, box_y1, box_y2;
  while(1) {
    if (touch_available()) {
      touch_read(&x, &y);

      box_x1 = (x >= 1)? (x-1) : 0;
      box_x2 = (x <= 238)? (x+1) : 239;

      box_y1 = (y >= 1)? (y-1) : 0;
      box_y2 = (y <= 318)? (y+1) : 319;

      TFT_draw_box(box_x1, box_y1, box_x2, box_y2, 61);

      if (x > 2 && x < 92 && y > 2 && y < 24) {
        TFT_fill(0xFFFF);

        TFT_box_outline(2,2,92,24,2,0);

        TFT_char(FONT_16x16, 'C', 5, 5, 0, 0xFFFF, 16, 16);
        TFT_char(FONT_16x16, 'l', 22, 5, 0, 0xFFFF, 16, 16);
        TFT_char(FONT_16x16, 'e', 39, 5, 0, 0xFFFF, 16, 16);
        TFT_char(FONT_16x16, 'a', 56, 5, 0, 0xFFFF, 16, 16);
        TFT_char(FONT_16x16, 'r', 73, 5, 0, 0xFFFF, 16, 16);
      }
    }
  }

  return 0;
}
