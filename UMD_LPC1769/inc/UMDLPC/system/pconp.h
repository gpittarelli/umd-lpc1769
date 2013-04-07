/* pconp.h
 *
 * Declares a convenient enum for configuring the Power Control for
 * Peripherals (PCONP) register.
 *
 * See section 4.8.9
 */

#ifndef __UMDLPC_system_pconp_h_
#define __UMDLPC_system_pconp_h_

enum PCONPBits {
  PC_TIM0 = (1 << 1),
  PC_TIM1 = (1 << 2),
  PC_UART0 = (1 << 3),
  PC_UART1 = (1 << 4),
  PC_PWM1 = (1 << 6),
  PC_I2C0 = (1 << 7),
  PC_SPI = (1 << 8),
  PC_RTC = (1 << 9),
  PC_SSP1 = (1 << 10),
  PC_ADC = (1 << 12),
  PC_CAN1 = (1 << 13),
  PC_CAN2 = (1 << 14),
  PC_GPIO = (1 << 15),
  PC_RIT = (1 << 16),
  PC_MCPWM = (1 << 17),
  PC_QEI = (1 << 18),
  PC_I2C1 = (1 << 19),
  PC_SSP0 = (1 << 21),
  PC_TIM2 = (1 << 22),
  PC_TIM3 = (1 << 23),
  PC_UART2 = (1 << 24),
  PC_UART3 = (1 << 25),
  PC_I2C2 = (1 << 26),
  PC_I2S = (1 << 27),
  PC_GPDMA = (1 << 29),
  PC_ENET = (1 << 30),
  PC_USB = (1 << 31)
};

#endif
