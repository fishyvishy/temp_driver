/******************************************************************************
    @file:  lcd_view.h

    @brief: Abstraction of LCD display
******************************************************************************/

#ifndef LCD_VIEW_H
#define LCD_VIEW_H

#include "Arduino.h"
#include "global_error.h"

// LCD Display dependencies
#include <Adafruit_LiquidCrystal.h>
#include <Wire.h>

#include "view_state.h"

extern GlobalError system_error;

class LCDView {
 public:
  Adafruit_LiquidCrystal lcd;

  /**
   * @brief Constructor for the LCDView class for SPI control of lcd
   *
   * @param DAT Data pin
   * @param CLK Clock pin
   * @param LAT Latch pin
   * @param state pointer to the viewState object
   * @see ViewState
   */
  LCDView(int DAT, int CLK, int LAT, ViewState* state)
      : lcd(DAT, CLK, LAT), state(state) {};

  /**
   * @brief Initializes the LCD display
   */
  void begin() {
    lcd.begin(16, 2);
    lcd.createChar(0, graphene_icon);
    this->reset();
  }

  /**
   * @brief Resets the LCD display
   */
  void reset() {
    lcd.clear();
    lcd.home();
    lcd.print(F("Barrera2D"));
    lcd.write(byte(0));
    lcd.print(F("Lab"));
    lcd.setCursor(0, 1);
    lcd.print(F("ACDAC 02 AD9106"));
  }

  /**
   * @brief Updates the LCD display depending on view state
   */
  void update() {
    if (state->mode == ViewState::Mode::ERROR) {
      display_error();
    } else if (state->mode == ViewState::Mode::REMOTE) {
      display_remote();
    } else
      (state->mode == ViewState::Mode::NORMAL) ? display_normal()
                                               : display_focus();
    state->update = false;
  }

 private:
  ViewState* state;

  // Graphene Icon bitmap
  byte graphene_icon[8] = {0b00010, 0b00101, 0b00101, 0b01010,
                           0b01010, 0b10100, 0b10100, 0b01000};

  /**
   * @brief Displays voltage and phase for all channels
   */
  void display_normal() {
    lcd.clear();
    for (int i = 0; i < 4; i++) {
      int volt = state->getVolts(i + 1);
      int phase = (int)state->getPhase(i + 1);

      lcd.setCursor((8 * i) % 16, (int)i / 2);
      lcd.print(volt);
      lcd.print(F(":"));
      lcd.print(phase);
    }
  }

  /**
   * @brief Displays channel specific data
   */
  void display_focus() {
    lcd.clear();
    int chan = 0;
    switch (state->mode) {
      case ViewState::Mode::FOCUS1:
        chan = 1;
        break;
      case ViewState::Mode::FOCUS2:
        chan = 2;
        break;
      case ViewState::Mode::FOCUS3:
        chan = 3;
        break;
      case ViewState::Mode::FOCUS4:
        chan = 4;
        break;
    }

    float voltage = state->getVolts(chan);
    float phase = state->getPhase(chan);

    lcd.setCursor(0, 0);
    lcd.print(F("CH"));
    lcd.print(chan);

    lcd.setCursor(4, 0);
    lcd.print(state->freq);
    lcd.print(F("Hz"));

    lcd.setCursor(0, 1);
    lcd.print(voltage, 1);
    lcd.print(F("mV"));

    lcd.setCursor(8, 1);
    lcd.print(phase);
  }

  /**
   * @brief Displays error message
   */
  void display_error() {
    int code = system_error.get_error(true);
    const char* msg = get_error_ptr(code);
    strcpy_P(system_error.message_buffer, msg);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Error "));
    lcd.print(code);
    lcd.setCursor(0, 1);
    lcd.print(system_error.message_buffer);
  }

  /**
   * @brief Disables updates for remote mode
   */
  void display_remote() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    lcd.print(F("Remote Access"));
  }
};

#endif