/* pins.h
 *
 * Define a useful macro for giving GPIO pins nicer, semantic names.
 *
 * The only macro exported is DEFINE_PIN. Use as:
 *  DEFINE_PIN(PIN_NAME, port number, pin number)
 * this will define the following:
 *  PIN_NAME_DEASSERT(), _OFF(), _LOW() to bring the pin low
 *  PIN_NAME_ASSERT(), _ON(), _HIGH() to bring the pin high
 *  PIN_NAME_INPUT() to make the pin an input
 *  PIN_NAME_OUTPUT() to make the pin an output
 *  PIN_NAME_READ() to read the current state of the pin
 *                  (returns a 1 or 0, typed as a uint_fast8_t)
 */

#ifndef __UMDLPC_util_pins_h_
#define __UMDLPC_util_pins_h_

#include <stdint.h>

#define DEFINE_PIN(name, port, pin) \
inline static void name##_DEASSERT() { \
  LPC_GPIO##port ->FIOCLR |= (1 << pin); \
} \
 \
inline static void name##_OFF() { \
  LPC_GPIO##port ->FIOCLR |= (1 << pin); \
} \
 \
inline static void name##_LOW() { \
  LPC_GPIO##port ->FIOCLR |= (1 << pin); \
} \
 \
inline static void name##_ASSERT() { \
  LPC_GPIO##port ->FIOSET |= (1 << pin); \
} \
 \
inline static void name##_ON() { \
  LPC_GPIO##port ->FIOSET |= (1 << pin); \
} \
 \
inline static void name##_HIGH() { \
  LPC_GPIO##port ->FIOSET |= (1 << pin); \
} \
 \
inline static void name##_INPUT() { \
  LPC_GPIO##port ->FIODIR &= ~(1 << pin); \
} \
 \
inline static void name##_OUTPUT() { \
  LPC_GPIO##port ->FIODIR |= (1 << pin); \
} \
 \
inline static uint_fast8_t name##_READ() { \
  return ( LPC_GPIO##port ->FIOPIN >> pin) & 1;  \
}

#endif
