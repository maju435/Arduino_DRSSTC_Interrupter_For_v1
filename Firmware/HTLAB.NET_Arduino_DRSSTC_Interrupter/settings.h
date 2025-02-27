#ifndef SETTINGS_H
#define SETTINGS_H

// ########## Interrupter Settings ##########

// Use 20*4 Character LCD Module
#define USE_LCD true

// Use Volume Input (DEFAULT = 0-1023)
#define USE_VR1 true
#define USE_VR2 true
#define USE_VR3 true
#define USE_VR4 true
#define INVERT_VR1 false
#define INVERT_VR2 false
#define INVERT_VR3 false
#define INVERT_VR4 false
#define DEFAULT_VR1 100
#define DEFAULT_VR2 100
#define DEFAULT_VR3 600
#define DEFAULT_VR4 100

#define USE_SW1 false
#define USE_SW2 false
#define INVERT_SW1 false
#define INVERT_SW2 false
#define DEFAULT_SW1 false
#define DEFAULT_SW2 false

#define USE_PUSH1 true
#define USE_PUSH2 true
#define INVERT_PUSH1 false
#define INVERT_PUSH2 false

// Use Setting Mode
#define USE_SETTING_MODE false

// Interrupter Mode Selector
// 0 : 4-Mode [OSC, OSC_OneShot, HighPower_OSC, HighPower_OSC_OneShot]
// 1 : 2-Mode [OSC, OSC_OneShot]
// 2 : 1-Mode [OSC]
#define DEFAULT_MODE_SELECTOR 0

// Beep Active [0:False/1:True]
#define DEFAULT_BEEP_ACTIVE 1

// Default MIDI Channel
#define DEFAULT_MIDI_CH1 1
#define DEFAULT_MIDI_CH2 2

// Max Note Number
#define MIDI_MAX_NOTE_NUM_CH1 84
#define MIDI_MAX_NOTE_NUM_CH2 84

// Use MIDI (UART)
#define USE_MIDI true

// Use MIDIUSB Library
#define USE_MIDIUSB true

// ########## Settings Complete! ##########

// 8 or 64
#define OSC_TIMER_DIVIDER 8

// For Debug
#define DEBUG_SERIAL false

// Wait for Serial Port
#define DEBUG_SERIAL_WAIT false

// Mode (Do not change the value)
#define MODE_OSC 1
#define MODE_OSC_OS 2
#define MODE_OSC_HP 4
#define MODE_OSC_HP_OS 8
#define MODE_BURST 16
#define MODE_MIDI 32
#define MODE_MIDI_FIXED 64

// Pin Number (Do not change the value)
//#define PIN_VR1 A4
#define PIN_VR1 A1
#define PIN_VR2 A2
#define PIN_VR3 A3
#define PIN_VR4 A4
#define PIN_SW1 A0
#define PIN_SW2 A7
#define PIN_PUSH1 2 //Interrupt1
#define PIN_PUSH2 3 //Interrupt0

// EEPROM Address Map
#define ADDR_MODE_SELECTOR 0
#define ADDR_BEEP_ACTIVE 1
#define ADDR_MIDI_CH1 2
#define ADDR_MIDI_CH2 3

#endif
