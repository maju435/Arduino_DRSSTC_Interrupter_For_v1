#include <MIDI.h>

//
// #################################################
//
//    HTLAB.NET Arduino DRSSTC Interrupter
//      https://htlab.net/electronics/teslacoil/
//
//    Copyright (C) 2017 - 2019
//      Hideto Kikuchi / PJ (@pcjpnet) - http://pc-jp.net/
//      Tsukuba Science Inc. - http://www.tsukuba-kagaku.co.jp/
//
//    !!!!! NOT ALLOWED COMMERCIAL USE !!!!!
//
// #################################################
//

//
// ########## Compatible Boards ##########
//    - Arduino Micro (USB-MIDI is available)
//    - Arduino Leonardo (USB-MIDI is available)
//

//
// ########## Pin Assignments ##########
//  D0(RX)  - MIDI IN
//  D1(TX)  - (MIDI OUT)
//  D2  - PUSH1 (Interrupt1)
//  D3  - PUSH2 (Interrupt0)
//  D4  - LCD (RS)
//  D5  - LCD (ENA)
//  D6  - LCD (DB4)
//  D7  - LCD (DB5)
//  D8  - LCD (DB6)
//  D9  - LCD (DB7)
//  D10 - OUT1
//  D11 - OUT2
//  D12 - LCD Backlight
//  D13 - PIEZO SPEAKER
//  A0  - VR1
//  A1  - VR2
//  A2  - VR3
//  A3  - VR4
//  A4  - SW1 (MIDI)
//  A5  - SW2 (MODE)
//

//
// ########## Require Libraries ##########
//    - MIDI Library 4.3
//

//
// ########## Optional Libraries ##########
//    - MIDIUSB Library 1.0.3 (for Arduino Leonardo, Micro)
//

// LCD
//#include <LiquidCrystal.h>
//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Settings
#include "settings.h"

// Pin I/O
#include "lib_input.h"
#include "lib_output.h"

// Oscillator
#include "lib_osc.h"

// MIDI Parameter
#include "lib_midi.h"

// Use MIDI Library
#include <MIDI.h>
#if (MIDI_LIBRARY_VERSION_MAJOR < 4) || \
  ((MIDI_LIBRARY_VERSION_MAJOR == 4) && (MIDI_LIBRARY_VERSION_MINOR < 3))
  #error This version of MIDI library is not supported. Please use version 4.3 or later.
#endif
#if USE_MIDI
  MIDI_CREATE_DEFAULT_INSTANCE();
#endif

// Use MIDIUSB Library (for Arduino Leonardo, Micro)
#if USE_MIDIUSB && defined(USBCON) && defined(__AVR_ATmega32U4__)
  #include <midi_UsbTransport.h>
  static const unsigned sUsbTransportBufferSize = 16;
  typedef midi::UsbTransport<sUsbTransportBufferSize> UsbTransport;
  UsbTransport sUsbTransport;
  MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, USBMIDI);
#endif

// Use Setting Mode
#if USE_SETTING_MODE
  #include <EEPROM.h>
#endif


// Global Variables
volatile uint8_t int_mode = MODE_OSC;
volatile uint8_t menu_state = MODE_OSC;
char lcd_line1[20];
char lcd_line2[20];
char lcd_line3[20];
char lcd_line4[20];

// Setting Variables
volatile bool beep_active = (bool)DEFAULT_BEEP_ACTIVE;
volatile uint8_t mode_selector = DEFAULT_MODE_SELECTOR;
volatile uint8_t midi_ch[2] = {DEFAULT_MIDI_CH1, DEFAULT_MIDI_CH2};

// OSC Mode Variables
volatile uint16_t osc_fq = 5;
volatile uint16_t osc_us = 1;
volatile uint16_t osc_per = 49999;
volatile uint16_t osc_per_read = 49999;

// One Shot Mode Variables
volatile uint16_t oneshot_ch1_ontime = 0;
volatile uint16_t oneshot_ch2_ontime = 0;

// Burst OSC Mode Variables
volatile bool burst_phase = false;
volatile uint16_t burst_ontime = 10;
volatile uint16_t burst_offtime = 510; 
volatile uint16_t burst_ontime_count = 0;
volatile uint16_t burst_offtime_count = 0; 

// MIDI Mode Variables
//volatile bool use_midi_volume = USE_MIDI_VOLUME;
//volatile bool osc_mode_omni = OSC_MODE_OMNI;
volatile bool osc_mono_midi_on[2] = {false, false};
volatile uint8_t osc_mono_midi_note[2] = {0, 0};
//volatile uint8_t osc_mono_midi_volume[2] = {64, 64};
//volatile uint8_t osc_mono_midi_expression[2] = {127, 127};
volatile uint16_t osc_mono_ontime_us[2] = {0, 0};
volatile uint16_t osc_mono_ontime_fixed_us[2] = {0, 0};

// 1Mhz generator
volatile unsigned long t=1, f, k=512;// default 1 μs (1 000 000 Hz), meander, pulse 

// Arduino Setup Function
void setup() {

  // Pin Init
  input_init();
  output_init();

  // MIDI Tasks
  midi_task();

//  pinMode(11, OUTPUT); // 1Mhz gen.
//  Timer1.initialize(t); // period   
//  Timer1.pwm(11, k); // k - fill factor 0-1023

//  pinMode(3, OUTPUT); // 1Mhz gen.
//  Timer3.initialize(t); // period   
//  Timer3.pwm(3, k); // k - fill factor 0-1023

  // LCD
  #if USE_LCD
    lcd.begin(20,4);
    lcd.setCursor(0,0);
    lcd.print("####################");
    lcd.setCursor(0,1);
    lcd.print("# HTLAB.NET DRSSTC #");
    lcd.setCursor(0,2);
    lcd.print("# Interrupter v1.0 #");
    lcd.setCursor(0,3);
    lcd.print("####################");
    uint32_t wait_start = millis();
    while(millis() < wait_start + 2000){
      // MIDI Tasks
      midi_task();
    }
    lcd.clear();
  #endif

  // For Debug
  #if DEBUG_SERIAL
    Serial.begin(115200);
    #if DEBUG_SERIAL_WAIT
      while (!Serial);
      Serial.println("[INFO] Arduino Start");
    #endif
  #endif

  // Load Settings
  #if USE_SETTING_MODE
    if(EEPROM.read(ADDR_MODE_SELECTOR) >= 3) {
      EEPROM.write(ADDR_MODE_SELECTOR, DEFAULT_MODE_SELECTOR);
    } else {
      mode_selector = EEPROM.read(ADDR_MODE_SELECTOR);
    }
    if(EEPROM.read(ADDR_BEEP_ACTIVE) >= 2) {
      EEPROM.write(ADDR_BEEP_ACTIVE, DEFAULT_BEEP_ACTIVE);
    } else {
      beep_active = (bool)EEPROM.read(ADDR_BEEP_ACTIVE);
    }
    if(EEPROM.read(ADDR_MIDI_CH1) > 16 || EEPROM.read(ADDR_MIDI_CH1) == 0) {
      EEPROM.write(ADDR_MIDI_CH1, DEFAULT_MIDI_CH1);
    } else {
      midi_ch[0] = EEPROM.read(ADDR_MIDI_CH1);
    }
    if(EEPROM.read(ADDR_MIDI_CH2) > 16 || EEPROM.read(ADDR_MIDI_CH2) == 0) {
      EEPROM.write(ADDR_MIDI_CH2, DEFAULT_MIDI_CH2);
    } else {
      midi_ch[1] = EEPROM.read(ADDR_MIDI_CH2);
    }
  #endif

  // Setting Mode
  #if USE_SETTING_MODE && USE_LCD
    if(!(INVERT_PUSH1 ^ (bool)digitalRead(2))) {
      char lcd_line[20];
      lcd.setCursor(0,0);
      lcd.print("[SETTING MODE 1]");
      while(INVERT_PUSH2 ^ (bool)digitalRead(3)) {
        // Read Inputs
        input_task();
        #if !INVERT_VR1
          mode_selector = (adc_vr_1 / 3 >> 7);
          beep_active = (bool)(adc_vr_2_inv >> 9);
        #else
          mode_selector = (adc_vr_1_inv / 3 >> 7);
          beep_active = (bool)(adc_vr_2 >> 9);
        #endif
        char* beep_str[] = {"Fals", "True"};
        sprintf(lcd_line, "%1u-MODE BEEP:%s", (4 / (mode_selector + 1)), beep_str[(uint8_t)beep_active]);
        lcd.setCursor(0,1);
        lcd.print(lcd_line);
        // MIDI Tasks
        midi_task();
      }
      // Save Settings
      EEPROM.update(ADDR_MODE_SELECTOR, mode_selector);
      EEPROM.update(ADDR_BEEP_ACTIVE, (uint8_t)beep_active);
      lcd.setCursor(0,0);
      lcd.print("[ SETTING DONE ]");
      lcd.setCursor(0,1);
      lcd.print("                ");
      uint32_t wait_start = millis();
      while(millis() < wait_start + 2000){
        // MIDI Tasks
        midi_task();
      }
    } else if(!(INVERT_PUSH2 ^ (bool)digitalRead(3))) {
      char lcd_line[20];
      lcd.setCursor(0,0);
      lcd.print("[SETTING MODE 2]");
      while(INVERT_PUSH1 ^ (bool)digitalRead(2)) {
        // Read Inputs
        input_task();
        #if !INVERT_VR1
          midi_ch[0] = (adc_vr_1 >> 6) + 1;
        #else
          midi_ch[0] = (adc_vr_1_inv >> 6) + 1;
        #endif
        #if !INVERT_VR2
          midi_ch[1] = (adc_vr_2 >> 6) + 1;
        #else
          midi_ch[1] = (adc_vr_2_inv >> 6) + 1;
        #endif
        sprintf(lcd_line, "Ch%2u Ch%2u       ", midi_ch[0], midi_ch[1]);
        lcd.setCursor(0,1);
        lcd.print(lcd_line);
        // MIDI Tasks
        midi_task();
      }
      // Save Settings
      EEPROM.update(ADDR_MIDI_CH1, midi_ch[0]);
      EEPROM.update(ADDR_MIDI_CH2, midi_ch[1]);
      lcd.setCursor(0,0);
      lcd.print("[ SETTING DONE ]");
      lcd.setCursor(0,1);
      lcd.print("                ");
      uint32_t wait_start = millis();
      while(millis() < wait_start + 2000) {
        // MIDI Tasks
        midi_task();
      }
    }
  #endif

  // Use MIDI Library
  #if USE_MIDI
    MIDI.setHandleNoteOn(isr_midi_noteon);
    MIDI.setHandleNoteOff(isr_midi_noteoff);
    MIDI.setHandleControlChange(isr_midi_controlchange);
    MIDI.setHandleActiveSensing(isr_midi_activesensing);
    MIDI.setHandleSystemReset(isr_midi_systemreset);
    MIDI.begin(MIDI_CHANNEL_OMNI);
  #endif
  #if USE_MIDIUSB && defined(USBCON) && defined(__AVR_ATmega32U4__)
    USBMIDI.setHandleNoteOn(isr_midi_noteon);
    USBMIDI.setHandleNoteOff(isr_midi_noteoff);
    USBMIDI.setHandleControlChange(isr_midi_controlchange);
    USBMIDI.setHandleActiveSensing(isr_midi_activesensing);
    USBMIDI.setHandleSystemReset(isr_midi_systemreset);
    USBMIDI.begin(MIDI_CHANNEL_OMNI);
  #endif
  #if DEBUG_SERIAL
    Serial.println("[INFO] MIDI Library Load Complete");
  #endif

  // Oscillator Tasks
  osc_timer_init();
  #if DEBUG_SERIAL
    Serial.println("[INFO] Oscillator Tasks Complete");
  #endif
  
  // MIDI Tasks
  midi_task();

  // Mode Init Tasks
  mode_init(menu_state);

}


// Arduino Main Loop
void loop() {

  // Input Tasks
  input_task();
  menu_state = menu_select(mode_selector);
//  menu_state = MODE_BURST;
  if (int_mode != menu_state) {
    mode_init(menu_state);
    int_mode = menu_state;
  }

  // OSC Tasks
  switch (menu_state) {
    // OSC Mode
    case MODE_OSC:
      osc_fq = (adc_vr_1 >> 1)*0.7 + 50;
      osc_us = (adc_vr_2 >> 1) + 1;
      osc_per_read = (250000 / osc_fq) - 1;
      if (osc_per != osc_per_read) {
        #if DEBUG_SERIAL
          Serial.print("[OSC] Change Frequency : ");
          Serial.print(osc_fq);
          Serial.print(" Hz/ ");
          Serial.println(osc_per_read);
        #endif
        osc_per = osc_per_read;
        osc_timer_set(0, osc_per);
        osc_timer_set(1, osc_per);
      }
      break;
    // One Shot Mode
    case MODE_OSC_OS:
      oneshot_ch1_ontime = (adc_vr_1 >> 1) + 1;
      oneshot_ch2_ontime = (adc_vr_2 >> 1) + 1;
      break;
    // High Power OSC Mode
    case MODE_OSC_HP:
      osc_fq = (adc_vr_1 >> 4) + 1;
      osc_us = (adc_vr_2 >> 3) * 50;
      osc_per_read = (250000 / osc_fq) - 1;
      if (osc_per != osc_per_read) {
        osc_per = osc_per_read;
        osc_timer_set(0, osc_per);
        osc_timer_set(1, osc_per);
      }
      break;
    // High Power One Shot Mode
    case MODE_OSC_HP_OS:
      oneshot_ch1_ontime = ((adc_vr_1 >> 3) + 1) * 100;
      oneshot_ch2_ontime = ((adc_vr_2 >> 3) + 1) * 100;
      break;
    // Burst OSC Mode
    case MODE_BURST:
      burst_offtime = (adc_vr_3_inv >> 1) + 10;
      burst_ontime = (adc_vr_4 >> 1) + 10;
      osc_fq = (adc_vr_1 >> 1)*0.7 + 50;
      osc_us = (adc_vr_2 >> 2) + 1;
      osc_per_read = (250000 / osc_fq) - 1;
      if (osc_per != osc_per_read) {
        osc_per = osc_per_read;
        osc_timer_set(0, osc_per);
      }
      break;
    // MIDI Mode
    case MODE_MIDI:
      osc_mono_ontime_us[0] = (adc_vr_1 >> 2) + 1;
      osc_mono_ontime_us[1] = (adc_vr_2 >> 2) + 1;
      break;
    // MIDI FIXED Mode
    case MODE_MIDI_FIXED:
      osc_mono_ontime_us[0] = (adc_vr_1 >> 2) + 1;
      osc_mono_ontime_us[1] = (adc_vr_2 >> 2) + 1;
      if (osc_mono_midi_on[0]){
        osc_mono_ontime_fixed_us[0] = osc_mono_ontime_us[0] * ontime_fix_per(note_to_hz[osc_mono_midi_note[0]]) / 100;
      }
      if (osc_mono_midi_on[1]){
        osc_mono_ontime_fixed_us[1] = osc_mono_ontime_us[1] * ontime_fix_per(note_to_hz[osc_mono_midi_note[1]]) / 100;
      }
      break;
  }

  // MIDI Tasks
  midi_task();

  // LCD
  #if USE_LCD
    show_lcd(menu_state);
  #endif

  // MIDI Tasks
  midi_task();

}


void midi_task() {
  // MIDI Tasks
  for (uint8_t i = 0; i < 200; i++) {
    #if USE_MIDI
      MIDI.read();
    #endif
    #if USE_MIDIUSB && defined(USBCON) && defined(__AVR_ATmega32U4__)
      USBMIDI.read();
    #endif
  }
}


// MODE Init
void mode_init(byte mode) {
  detachInterrupt(1); // Pin2
  detachInterrupt(0); // Pin3
  switch (mode) {
    // OSC Mode
    case MODE_OSC:
      osc_timer_disable(0);
      osc_timer_disable(1);
      osc_timer_init_64();
      osc_timer_enable(0, osc_per);
      break;
    // One Shot Mode
    case MODE_OSC_OS:
      osc_timer_disable(0);
      osc_timer_disable(1);
      #if USE_PUSH1
        #if !INVERT_PUSH1
          attachInterrupt(1, isr_sw1, FALLING);
        #else
          attachInterrupt(1, isr_sw1, RISING);
        #endif
      #endif
      #if USE_PUSH2
        #if !INVERT_PUSH2
          attachInterrupt(0, isr_sw2, FALLING);
        #else
          attachInterrupt(0, isr_sw2, RISING);
        #endif
      #endif
      break;
    // OSC High Power Mode
    case MODE_OSC_HP:
      osc_timer_disable(0);
      osc_timer_disable(1);
      osc_timer_init_64();
      osc_timer_enable(0, osc_per);
      break;
    // High Power One Shot Mode
    case MODE_OSC_HP_OS:
      osc_timer_disable(0);
      osc_timer_disable(1);
      #if USE_PUSH1
        #if !INVERT_PUSH1
          attachInterrupt(1, isr_sw1, FALLING);
        #else
          attachInterrupt(1, isr_sw1, RISING);
        #endif
      #endif
      #if USE_PUSH2
        #if !INVERT_PUSH2
          attachInterrupt(0, isr_sw2, FALLING);
        #else
          attachInterrupt(0, isr_sw2, RISING);
        #endif
      #endif
      break;
    // Burst OSC Mode
    case MODE_BURST:
      osc_timer_disable(0);
      osc_timer_disable(1);
      osc_timer_init_64();
      osc_timer_enable(0, osc_per);
      osc_timer_enable(1, 249);
      break;
    // MIDI Mode
    // MIDI FIXED Mode
    case MODE_MIDI:
    case MODE_MIDI_FIXED:
      osc_timer_disable(0);
      osc_timer_disable(1);
      osc_mono_midi_on[0] = false;
      osc_mono_midi_on[1] = false;
      osc_timer_init();
      break;
  }
  #if DEBUG_SERIAL
    Serial.print("[INFO] Change Mode : ");
    Serial.println(mode);
  #endif
}


// SHOW LCD
void show_lcd(byte mode) {
  char osc_duty[5];
  char burst_fq[5];
  dtostrf((float)(100.00 * ((float)osc_us * pow(10.00, -6)) / (1.00 / (osc_us * pow(10.00, -6) + (float)osc_fq))), 4, 1, osc_duty);
  dtostrf((float)(1.00 / (((float)burst_offtime + (float)burst_ontime) * pow(10.00, -3))), 4, 1, burst_fq);
  
  switch (mode) {
    // OSC Mode
    case MODE_OSC:
      sprintf(lcd_line1, "<> INTERRUPTER MODE");
//      sprintf(lcd_line2, "Fq:%4uHz OnT:%3uus", osc_fq, osc_us);
//      sprintf(lcd_line3, "Dt:%s%%", osc_duty);
      
      sprintf(lcd_line2, "Fq:%3uHz           ", osc_fq);
      sprintf(lcd_line3, "On:%3uus          ", osc_us);
      sprintf(lcd_line4, "Dt:%s%%         ", osc_duty);
      break;

    // One Shot Mode
    case MODE_OSC_OS:
      sprintf(lcd_line1, "<> ONE SHOT MODE   ");
      sprintf(lcd_line2, "1:%3uus 2:%3uus ", oneshot_ch1_ontime, oneshot_ch2_ontime);
      sprintf(lcd_line3, "%s", "                   ");
      sprintf(lcd_line4, "%s", "                   ");
      break;

    // High Power OSC Mode
    case MODE_OSC_HP:
      sprintf(lcd_line1, "<> SKP MODE        ");
      sprintf(lcd_line2, "Fq:%2uHz            ", osc_fq);
      sprintf(lcd_line3, "On:%5uus       ", osc_us);
      sprintf(lcd_line4, "Dt:%s%%        ", osc_duty);
      break;

    // High Power One Shot Mode
    case MODE_OSC_HP_OS:
      sprintf(lcd_line1, "<> SKP ONESHOT     ");
      sprintf(lcd_line2, "1:%5uu2:%5uu   ", oneshot_ch1_ontime, oneshot_ch2_ontime);
      sprintf(lcd_line3, "%s", "                   ");
      sprintf(lcd_line4, "%s", "                   ");
      break;

    // Burst OSC Mode
    case MODE_BURST:
      sprintf(lcd_line1, "<> BURST MODE       ", burst_offtime, burst_ontime);
//      sprintf(lcd_line2, "Fq:%3uHz On:%3uus", osc_fq, osc_us);
//      sprintf(lcd_line3, "Bfq:%sHz Dt:%s%%", burst_fq, osc_duty);
//      sprintf(lcd_line4, "Off:%3ums On:%3ums", burst_offtime, burst_ontime);
      
      sprintf(lcd_line2, "Bf:%sHz |Fq:%3uHz", burst_fq, osc_fq);
      sprintf(lcd_line3, "Bon:%3ums |On:%3uus", burst_ontime, osc_us);
      sprintf(lcd_line4, "Off:%3ums |Dt:%s%%", burst_offtime, osc_duty);
      break;

    // MIDI Mode
    case MODE_MIDI:
      sprintf(lcd_line1, "MIDI MODE[%2u/%2u]", midi_ch[0], midi_ch[1]);
      sprintf(lcd_line2, "%s", "                   ");
      sprintf(lcd_line3, "%s", "                   ");
      sprintf(lcd_line4, "%s", "                   ");
      show_lcd_midi_status();
      break;

    // MIDI Fixed Mode
    case MODE_MIDI_FIXED:
      sprintf(lcd_line1, "F%2u:%3uu %2u:%3uu", midi_ch[0], osc_mono_ontime_fixed_us[0], midi_ch[1], osc_mono_ontime_fixed_us[1]);
      sprintf(lcd_line2, "%s", "                   ");
      sprintf(lcd_line3, "%s", "                   ");
      sprintf(lcd_line4, "%s", "                   ");
      show_lcd_midi_status();
      break;
  }
  
  lcd.setCursor(0,0);
  lcd.print(lcd_line1);
  lcd.setCursor(0,1);
  lcd.print(lcd_line2);
  lcd.setCursor(0,2);
  lcd.print(lcd_line3);
  lcd.setCursor(0,3);
  lcd.print(lcd_line4);
}


void show_lcd_midi_status() {
  //if (osc_mono_midi_on[0] && osc_mono_midi_on[1]) {
    sprintf(lcd_line2, "%3u:%3uu%3u:%3uu", osc_mono_midi_note[0], osc_mono_ontime_us[0], osc_mono_midi_note[1], osc_mono_ontime_us[1]);
  //} else if (!osc_mono_midi_on[0] && osc_mono_midi_on[1]) {
  //  sprintf(lcd_line2, "   :%3uu%3u:%3uu", osc_mono_ontime_us[0], osc_mono_midi_note[1], osc_mono_ontime_us[1]);
  //} else if (osc_mono_midi_on[0] && !osc_mono_midi_on[1]) {
  //  sprintf(lcd_line2, "%3u:%3uu   :%3uu", osc_mono_midi_note[0], osc_mono_ontime_us[0], osc_mono_ontime_us[1]);
  //} else if (!osc_mono_midi_on[0] && !osc_mono_midi_on[1]) {
  //  sprintf(lcd_line2, "   :%3uu   :%3uu", osc_mono_ontime_us[0], osc_mono_ontime_us[1]);
  //}
}


void isr_sw1() {
  output_single_pulse(0, oneshot_ch1_ontime);
  detachInterrupt(1);
  delayMicroseconds(16000);
  #if !INVERT_PUSH1
    attachInterrupt(1, isr_sw1, FALLING);
  #else
    attachInterrupt(1, isr_sw1, RISING);
  #endif
}


void isr_sw2() {
  output_single_pulse(1, oneshot_ch2_ontime);
  detachInterrupt(0);
  delayMicroseconds(16000);
  #if !INVERT_PUSH2
    attachInterrupt(0, isr_sw2, FALLING);
  #else
    attachInterrupt(0, isr_sw2, RISING);
  #endif
}


// Interrupt Tasks
ISR (TIMER1_COMPA_vect) {
  switch (menu_state) {
    // OSC Mode Main Timer
    case MODE_OSC:
      output_dual_pulse(osc_us);
      break;
    // High Power OSC Mode
    case MODE_OSC_HP:
      output_dual_pulse(osc_us);
      break;
    // Burst OSC Mode
    case MODE_BURST:
      if (burst_phase) {
        output_dual_pulse(osc_us);
      }
      break;
    // MIDI Mode
    case MODE_MIDI:
      output_single_pulse(0, osc_mono_ontime_us[0]);
      break;
    // MIDI FIXED Mode
    case MODE_MIDI_FIXED:
      output_single_pulse(0, osc_mono_ontime_fixed_us[0]);
      break;
  }
}


// Interrupt Tasks
ISR (TIMER3_COMPA_vect) {
  switch (menu_state) {
    // OSC Mode
    case MODE_OSC:
      break;
    // High Power OSC Mode
    case MODE_OSC_HP:
      break;
    // Burst OSC Mode
    case MODE_BURST:
      if (burst_phase) {
        if (burst_ontime_count >= burst_ontime) {
          burst_ontime_count = 0;
          burst_phase = false;
        } else {
          burst_ontime_count++;
        }
      } else {
        if (burst_offtime_count >= burst_offtime) {
          burst_offtime_count = 0;
          burst_phase = true;
        } else {
          burst_offtime_count++;
        }
      }
      break;
    // MIDI Mode
    case MODE_MIDI:
      output_single_pulse(1, osc_mono_ontime_us[1]);
      break;
    // MIDI FIXED Mode
    case MODE_MIDI_FIXED:
      output_single_pulse(1, osc_mono_ontime_fixed_us[1]);
      break;
  }
}


// Interrupt MIDI NoteON Tasks
void isr_midi_noteon(uint8_t ch, uint8_t num, uint8_t vel) {

  if (menu_state != MODE_MIDI && menu_state != MODE_MIDI_FIXED) return;  // MIDI Mode Only

  if (ch == midi_ch[0] && num <= MIDI_MAX_NOTE_NUM_CH1) {
    osc_timer_enable(0, timer_period[num] - 1);
    osc_mono_midi_note[0] = num;
    osc_mono_midi_on[0] = true;
  }
  if (ch == midi_ch[1] && num <= MIDI_MAX_NOTE_NUM_CH2) {
    osc_timer_enable(1, timer_period[num] - 1);
    osc_mono_midi_note[1] = num;
    osc_mono_midi_on[1] = true;
  }

  // For Debug
  #if DEBUG_SERIAL
    Serial.print("[MIDI] Note On - CH:");
    Serial.print(ch);
    Serial.print(" NUM:");
    Serial.print(num);
    Serial.print(" VEL:");
    Serial.print(vel);
    Serial.print(" / ");
    Serial.print(note_to_hz[num]);
    Serial.println(" Hz");
  #endif
}


// Interrupt MIDI NoteOFF Tasks
void isr_midi_noteoff(uint8_t ch, uint8_t num, uint8_t vel) {

  if (menu_state != MODE_MIDI && menu_state != MODE_MIDI_FIXED) return;  // MIDI Mode Only

  if (ch == midi_ch[0]) {
    if (osc_mono_midi_note[0] == num) {
      osc_timer_disable(0);
      osc_mono_midi_on[0] = false;
      if (menu_state == MODE_MIDI_FIXED) {
        osc_mono_ontime_fixed_us[0] = 0;
      }
    }
  }
  if (ch == midi_ch[1]) {
    if (osc_mono_midi_note[1] == num) {
      osc_timer_disable(1);
      osc_mono_midi_on[1] = false;
      if (menu_state == MODE_MIDI_FIXED) {
        osc_mono_ontime_fixed_us[1] = 0;
      }
    }
  }

  // For Debug
  #if DEBUG_SERIAL
    Serial.print("[MIDI] NoteOff - CH:");
    Serial.print(ch);
    Serial.print(" NUM:");
    Serial.print(num);
    Serial.print(" VEL:");
    Serial.println(vel);
  #endif
}



void isr_midi_controlchange(uint8_t ch, uint8_t num, uint8_t val) {

  if (menu_state != MODE_MIDI && menu_state != MODE_MIDI_FIXED) return;  // MIDI Mode Only

  switch(num) {
    case 7:   // CC#7   Channel Volume
      //if (ch == midi_ch[0]) {
        //osc_mono_midi_volume[0] = val;
      //}
      //if (ch == midi_ch[1]) {
        //osc_mono_midi_volume[1] = val;
      //}
      break;
    case 11:  // CC#11  Expression
      //if (ch == midi_ch[0]) {
        //osc_mono_midi_expression[0] = val;
      //}
      //if (ch == midi_ch[1]) {
        //osc_mono_midi_expression[1] = val;
      //}
      break;
    case 121: // CC#121 Reset All Controllers
      //osc_mono_midi_volume[0] = 64;
      //osc_mono_midi_volume[1] = 64;
      //osc_mono_midi_expression[0] = 127;
      //osc_mono_midi_expression[0] = 127;
    case 120: // CC#120 All Sound Off
    case 123: // CC#123 All Notes Off
      osc_timer_disable(0);
      osc_timer_disable(1);
      osc_mono_midi_on[0] = false;
      osc_mono_midi_on[1] = false;
      break;
    case 124: // CC#124 Omni Mode Off
      //osc_mode_omni = false;
      break;
    case 125: // CC#125 Omni Mode On
      //osc_mode_omni = true;
      break;
  }

  // For Debug
  #if DEBUG_SERIAL
    Serial.print("[MIDI] CC - CH:");
    Serial.print(ch);
    Serial.print(" NUM:");
    Serial.print(num);
    Serial.print(" VAL:");
    Serial.println(val);
  #endif
}


void isr_midi_activesensing() {

  if (menu_state != MODE_MIDI && menu_state != MODE_MIDI_FIXED) return;  // MIDI Mode Only

  // For Debug
  #if DEBUG_SERIAL
    Serial.println("[MIDI] Active Sensing");
  #endif
}


void isr_midi_systemreset() {

  if (menu_state != MODE_MIDI && menu_state != MODE_MIDI_FIXED) return;  // MIDI Mode Only

  osc_timer_disable(0);
  osc_timer_disable(1);
  osc_mono_midi_on[0] = false;
  osc_mono_midi_on[1] = false;
  //osc_mode_omni = OSC_MODE_OMNI;
  //osc_mono_midi_volume[0] = 64;
  //osc_mono_midi_volume[1] = 64;
  //osc_mono_midi_expression[0] = 127;
  //osc_mono_midi_expression[0] = 127;

  // For Debug
  #if DEBUG_SERIAL
    Serial.println("[MIDI] System Reset");
  #endif
}


// Hz to ON-Time Percent
uint16_t ontime_fix_per(uint16_t hz) {
  return (10000 / (160 + hz)) + 40;
}
