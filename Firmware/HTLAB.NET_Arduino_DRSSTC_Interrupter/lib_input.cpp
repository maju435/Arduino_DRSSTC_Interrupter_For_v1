#include "lib_input.h"

volatile uint16_t adc_vr_1 = DEFAULT_VR1;
volatile uint16_t adc_vr_2 = DEFAULT_VR2;
volatile uint16_t adc_vr_3 = DEFAULT_VR3;
volatile uint16_t adc_vr_4 = DEFAULT_VR4;
volatile uint16_t adc_vr_1_inv = 1013 - DEFAULT_VR1;
volatile uint16_t adc_vr_2_inv = 1013 - DEFAULT_VR2;
volatile uint16_t adc_vr_3_inv = 1013 - DEFAULT_VR3;
volatile uint16_t adc_vr_4_inv = 1013 - DEFAULT_VR4;
volatile bool gpio_sw_1 = !(DEFAULT_SW1);
volatile bool gpio_sw_2 = !(DEFAULT_SW2);
volatile bool gpio_push_1;
volatile bool gpio_push_2;
volatile bool button_click = true;
volatile int choosen_menu_item = 0;

void input_init() {
  #if USE_VR1
    pinMode(PIN_VR1, INPUT);
  #endif
  #if USE_VR2 
    pinMode(PIN_VR2, INPUT);
  #endif
  #if USE_VR3
    pinMode(PIN_VR3, INPUT);
  #endif
  #if USE_VR4
    pinMode(PIN_VR4, INPUT);
  #endif
  #if USE_SW1
    pinMode(PIN_SW1, INPUT_PULLUP);
  #endif
  #if USE_SW2
    pinMode(PIN_SW2, INPUT_PULLUP);
  #endif
  #if USE_PUSH1
    pinMode(PIN_PUSH1, INPUT_PULLUP);
  #endif
  #if USE_PUSH2
    pinMode(PIN_PUSH2, INPUT_PULLUP);
  #endif
}


void input_task() {
  // Read Volume
  #if USE_VR1
     #if !INVERT_VR1
       adc_vr_1 = ((float)adc_vr_1 * 0.9) + ((float)analogRead(PIN_VR1) * 0.1);
       adc_vr_1_inv = ((float)adc_vr_1_inv * 0.9) + ((float)(1023 - analogRead(PIN_VR1)) * 0.1);
     #else
       adc_vr_1 = ((float)adc_vr_1 * 0.9) + ((float)(1023 - analogRead(PIN_VR1)) * 0.1);
       adc_vr_1_inv = ((float)adc_vr_1_inv * 0.9) + ((float)analogRead(PIN_VR1) * 0.1);
     #endif
  #endif
  #if USE_VR2
    #if !INVERT_VR2
      adc_vr_2 = ((float)adc_vr_2 * 0.9) + ((float)analogRead(PIN_VR2) * 0.1);
      adc_vr_2_inv = ((float)adc_vr_2_inv * 0.9) + ((float)(1023 - analogRead(PIN_VR2)) * 0.1);
    #else
      adc_vr_2 = ((float)adc_vr_2 * 0.9) + ((float)(1023 - analogRead(PIN_VR2)) * 0.1);
      adc_vr_2_inv = ((float)adc_vr_2_inv * 0.9) + ((float)analogRead(PIN_VR2) * 0.1);
    #endif
  #endif
  #if USE_VR3
    #if !INVERT_VR3
      adc_vr_3 = ((float)adc_vr_3 * 0.9) + ((float)analogRead(PIN_VR3) * 0.1);
      adc_vr_3_inv = ((float)adc_vr_3_inv * 0.9) + ((float)(1023 - analogRead(PIN_VR3)) * 0.1);
    #else
      adc_vr_3 = ((float)adc_vr_3 * 0.9) + ((float)(1023 - analogRead(PIN_VR3)) * 0.1);
      adc_vr_3_inv = ((float)adc_vr_3_inv * 0.9) + ((float)analogRead(PIN_VR3) * 0.1);
    #endif
  #endif
  #if USE_VR4
    #if !INVERT_VR4
      adc_vr_4 = ((float)adc_vr_4 * 0.9) + ((float)analogRead(PIN_VR4) * 0.1);
      adc_vr_4_inv = ((float)adc_vr_4_inv * 0.9) + ((float)(1023 - analogRead(PIN_VR4)) * 0.1);
    #else
      adc_vr_4 = ((float)adc_vr_4 * 0.9) + ((float)(1023 - analogRead(PIN_VR4)) * 0.1);
      adc_vr_4_inv = ((float)adc_vr_4_inv * 0.9) + ((float)analogRead(PIN_VR4) * 0.1);
    #endif
  #endif
  #if USE_SW1
    #if !INVERT_SW1
      gpio_sw_1 = (bool)(analogRead(PIN_SW1) < 60);
    #else
      gpio_sw_1 = !(bool)(analogRead(PIN_SW1) < 60);
    #endif
  #endif
  #if USE_SW2
    #if !INVERT_SW2
      gpio_sw_2 = (bool)(60 < analogRead(PIN_SW1) < 600);
    #else
      gpio_sw_2 = !(bool)(60 < analogRead(PIN_SW1) < 600);
    #endif
  #endif
  #if USE_PUSH1
    #if !INVERT_PUSH1
      gpio_push_1 = (bool)digitalRead(PIN_PUSH1);
    #else
      gpio_push_1 = !(bool)digitalRead(PIN_PUSH1);
    #endif
  #endif
  #if USE_PUSH2
    #if !INVERT_PUSH2
      gpio_push_2 = (bool)digitalRead(PIN_PUSH2);
    #else
      gpio_push_2 = !(bool)digitalRead(PIN_PUSH2);
    #endif
  #endif
}

uint8_t menu_select(uint8_t mode_selector) {
  int menu_item_list[] = {
    MODE_OSC,
    MODE_OSC_OS,
    MODE_OSC_HP,
    MODE_OSC_HP_OS,
    MODE_BURST,
    MODE_MIDI,
    MODE_MIDI_FIXED
  };

  int x = analogRead(PIN_SW1);

  if (button_click) {
    button_click = false;

    if (x < 10) {
    //      lcd.print ("LEFT   ");
      if (choosen_menu_item == 0) {
        choosen_menu_item = 6;
      } else {
        choosen_menu_item--;
      }
    }
    else if (x < 50) {
    //  lcd.print ("UP ");
      if (choosen_menu_item == 6) {
        choosen_menu_item = 0;
      } else {
        choosen_menu_item++;
      }
    }
    else if (x < 100) {
//      lcd.print ("Down    ");
      if (choosen_menu_item == 0) {
        choosen_menu_item = 6;
      } else {
        choosen_menu_item--;
      }
    }
    else if (x < 200){
//      lcd.print ("Right  ");
      if (choosen_menu_item == 6) {
        choosen_menu_item = 0;
      } else {
        choosen_menu_item++;
      }
    }
    else if (x < 400){
    //  lcd.print ("RED  ");
    }
  }

  if (x > 900) {
    button_click = true;
  }

  return menu_item_list[choosen_menu_item];
}
