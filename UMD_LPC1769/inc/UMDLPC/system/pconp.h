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
  PCTIM0 = (1 << 1),
  PCTIM1 = (1 << 2),
  PCUART0 = (1 << 3),
  PCUART1 = (1 << 4),
  PCPWM1 = (1 << 6),
  PCI2C0 = (1 << 7),
  PCSPI = (1 << 8),
  PCRTC = (1 << 9),
  PCSSP1 = (1 << 10),
  PCADC = (1 << 12),
  PCCAN1 = (1 << 13),
  PCCAN2 = (1 << 14),
  PCGPIO = (1 << 15),
  PCRIT = (1 << 16),
  PCMCPWM = (1 << 17),
  PCQEI = (1 << 18),
  PCI2C1 = (1 << 19),
  PCSSP0 = (1 << 21),
  PCTIM2 = (1 << 22),
  PCTIM3 = (1 << 23),
  PCUART2 = (1 << 24),
  PCUART3 = (1 << 25),
  PCI2C2 = (1 << 26),
  PCI2S = (1 << 27),
  PCGPDMA = (1 << 29),
  PCENET = (1 << 30),
  PCUSB = (1 << 31)
};

#endif
