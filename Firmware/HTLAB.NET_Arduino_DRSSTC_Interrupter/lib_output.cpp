#include "lib_output.h"

void output_init() {

  // Pin Settings
  pinMode(30, OUTPUT);
  pinMode(31, OUTPUT);
  pinMode(28, OUTPUT);
  pinMode(29, OUTPUT);

  // LCD Backlight
//  digitalWrite(12, HIGH);
  digitalWrite(10, HIGH);

}


void output_single_pulse(uint8_t pin, uint16_t ontime) {

  // Arduino Leonardo, Micro (ATmega32u4)
  // D10-11 = PB6-7
  // Arduino MEGA (2560)
  // D28-29 = PA6-7
  // D30-31 = PC6-7
  PORTA |= _BV(pin + 6);
  if (beep_active) PORTC |= _BV(7);
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  delayMicroseconds(ontime);
  PORTA &= ~_BV(pin + 6);
  if (beep_active) PORTC &= ~_BV(7);

}


void output_dual_pulse(uint16_t ontime) {

  // Arduino Leonardo, Micro (ATmega32u4)
  // D10-11 = PB6-7
  // Arduino MEGA (2560)
  // D28-29 = PA6-7
  // D30-31 = PC6-7
  PORTA |= _BV(6) | _BV(7);
  if (beep_active) PORTC |= _BV(7);
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  delayMicroseconds(ontime);
  PORTA &= ~(_BV(6) | _BV(7));
  if (beep_active) PORTC &= ~_BV(7);

}
