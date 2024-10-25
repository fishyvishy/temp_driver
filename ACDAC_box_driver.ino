/******************************************************************************
    @file:  ACDAC_box_driver.ino

    @brief: Entry point for the ACDAC box driver firmware. See SOP for more info

    Wiring:
    - EVAL-AD9106 mounted on Arduino UNO R3 using hardware SPI pins
        - On board operational amplifiers enabled
        - 12V @ 1A power to P15 DC jack
        - CS on digital pin 10
    - LCD HD44780 compatible Display with Adafruit SPI backpack
        - CS on digital pin 3
        - CLK on digital pin 6
        - DAT on digital pin 5
******************************************************************************/

#include "lcd_view.h"
#include "model.h"

#include <Vrekrer_scpi_parser.h>
#include "command_handlers.h"
#include "global_error.h"
#include "view_state.h"

ViewState viewState;
SCPI_Parser parser;

const int AD9106_CS = 10;
const int LCD_DAT = 5;
const int LCD_CLK = 6;
const int LCD_LAT = 3;

Model model(AD9106_CS);
LCDView view(LCD_DAT, LCD_CLK, LCD_LAT, &viewState);

GlobalError system_error(&GlobalErrorHandler);

void setup() {
  registerCommands();
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  model.begin();
  view.begin();
  viewState.reset();
}

void loop() {
  parser.ProcessInput(Serial, "\n");
  if (viewState.update) {
    view.update();
  }
}

// Register SCPI commands to functions
void registerCommands() {
  // Root Commands
  parser.RegisterCommand(F("*IDN?"), &handleIdentify);
  parser.RegisterCommand(F("*RST"), &handleReset);
  parser.RegisterCommand(F("FREQ"), &handleSetFreq);
  parser.RegisterCommand(F("FREQ?"), &handleGetFreq);

  // System Commands
  parser.SetCommandTreeBase(F("SYS"));
  parser.SetErrorHandler(&SCPIErrorHandler);
  parser.RegisterCommand(F(":ERRor?"), &GetLastEror);
  parser.RegisterCommand(F("REGister?"), &handleGetReg);
  parser.RegisterCommand(F("REGister"), &handleSetReg);
  parser.RegisterCommand(F(":DISPlay:MODE"), &changeMode);

  // Pattern Commands
  parser.SetCommandTreeBase(F("PATtern"));
  parser.RegisterCommand(F(":STOP"), &handleStop);
  parser.RegisterCommand(F(":START"), &handleStart);
  parser.RegisterCommand(F(":UPDate"), &handleUpdate);

  // Channel Commands
  parser.SetCommandTreeBase(F("CHANnel#"));
  parser.RegisterCommand(F(":VOLTage"), &handleSetVoltage);
  parser.RegisterCommand(F(":VOLTage?"), &handleGetVoltage);
  parser.RegisterCommand(F(":PHASe"), &handleSetPhase);
  parser.RegisterCommand(F(":PHase?"), &handleGetPhase);
}

// Global Error handler function
void GlobalErrorHandler() {
  viewState.setMode(ViewState::Mode::ERROR);
}