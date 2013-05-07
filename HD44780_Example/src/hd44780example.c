/*
  ==============================================================================
  Name        : hd7780example.c
  Author      : George Pittarelli
  Version     :
  Description :
  Provides a simple example of the HD7780 library
  ==============================================================================
*/

#include "hd44780example.h"
#include "hd44780.h"

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

int main(void) {
  // Select 12MHz crystal oscillator
  LPC_SC ->CLKSRCSEL = 1;

  // Init PLL 0 to 100MHz
  PLL_init(25, 2, 3);
//  PLL_init(12, 1, 18); // 16MHz
  SystemCoreClockUpdate();

  LCD_init();

  LCD_write('a');
  LCD_write('b');
  LCD_write('c');
  LCD_write('d');

  while(1) {

  }

  return 0;
}
