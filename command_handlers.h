/******************************************************************************
    @file:  control_functions.h

    @brief: Command handling for SCPI controls
******************************************************************************/

#ifndef COMMAND_HANDLERS_H
#define COMMAND_HANDLERS_H

#include <Vrekrer_scpi_parser.h>
#include "global_error.h"
#include "lcd_view.h"
#include "model.h"

extern Model model;
extern LCDView view;
extern SCPI_Parser parser;
extern GlobalError system_error;
extern ViewState viewState;

/*********************************************************/
// Helper Functions
/*********************************************************/

/**
 * @brief Check the number of parameters
 * @param expected Expected number of parameters
 * @param params Parameter list
 * @return 0 if the number of parameters is correct, 1 otherwise
 */
int check_param_num(int expected, int received) {
  if (expected == received) {
    return 0;
  }
  if (expected < received) {
    system_error.set_error(GenericError::TooManyParams);
  } else {
    system_error.set_error(GenericError::TooFewParams);
  }
  return 1;
}

/**
 * @brief Get the suffix of a command as an integer
 * @param commands SCPI Commands object
 * @return The suffix as an integer
 */
int get_int_suffix(SCPI_C& commands) {
  String header = String(commands.First());
  int suffix = -1;
  // Scan past non-numeric characters
  sscanf(header.c_str(), "%*[^0-9]%u", &suffix);
  return suffix;
}

/*********************************************************/
// SCPI Command Handlers
/*********************************************************/

/**
 * @brief Send identification string over interface
 */
void handleIdentify(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(0, params.Size()))
    return;
  interface.println(F("BARRERA, ACDAC02, AD9106, 2.00"));
}

/**
 * @brief Reset the model
 */
void handleReset(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(0, params.Size()))
    return;
  model.reset();
  view.reset();
  viewState.reset();
}

// Pattern Handlers
void handleStop(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(0, params.Size()))
    return;
  model.stop_pattern();
}

void handleStart(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(0, params.Size()))
    return;
  model.start();
  viewState.update = true;
}

void handleUpdate(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(0, params.Size()))
    return;
  model.update();
  if (viewState.mode != ViewState::Mode::REMOTE)
    viewState.update = true;
}

/**
 * @brief Set the voltage on a channel
 */
void handleSetVoltage(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(1, params.Size()))
    return;

  int chan = get_int_suffix(commands);
  if (chan < 1 | chan > 4) {
    system_error.set_error(GenericError::BadSuffix);
    return;
  }

  float voltage = atof(params[0]);
  if (model.setVoltage(chan, voltage)) {
    viewState.setVolts(chan, &voltage);
  }
}

/**
 * @brief Get the voltage on a channel
 *
 * TODO: get from model instead of viewState
 */
void handleGetVoltage(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(0, params.Size()))
    return;

  int chan = get_int_suffix(commands);
  if (chan < 1 | chan > 4) {
    system_error.set_error(GenericError::BadSuffix);
    return;
  }
  interface.println(viewState.getVolts(chan));
}

/**
 * @brief Get the value of a register on the AD9106
 */
void handleGetReg(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(1, params.Size()))
    return;

  uint16_t add = strtoul(params[0], NULL, 16);
  uint16_t val = model.readReg(add);
  interface.println(val, HEX);
}

/**
 * @brief Set the value of a register on the AD9106
 */
void handleSetReg(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(2, params.Size()))
    return;

  model.stop_pattern();
  uint16_t add = (uint16_t)strtol(params[0], NULL, 16);
  int16_t val = (int16_t)strtol(params[1], NULL, 16);
  model.writeReg(add, val);
}

/**
 * @brief Set the frequency
 */
void handleSetFreq(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(1, params.Size()))
    return;

  float freq = atof(params.First());
  if (freq < 0 || freq > 100000) {
    system_error.set_error(GenericError::ParamOutOfRange);
    return;
  }

  model.setFreq(freq);
  viewState.freq = model.getFreq();
}

/**
 * @brief Get the frequency
 */
void handleGetFreq(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(0, params.Size()))
    return;
  float freq = model.getFreq();
  interface.println(freq);
}

/**
 * @brief Set the phase of a channel
 */
void handleSetPhase(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(1, params.Size()))
    return;

  int chnl = get_int_suffix(commands);
  if (chnl < 1 | chnl > 4) {
    system_error.set_error(GenericError::BadSuffix);
    return;
  }

  float phase = atof(params.First());
  if (phase < -180 | phase > 180) {
    system_error.set_error(GenericError::ParamOutOfRange);
    return;
  }
  model.setPhase(chnl, phase);
  viewState.setPhase(chnl, &phase);
}

/**
 * @brief Get the phase of a channel
 */
void handleGetPhase(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(0, params.Size()))
    return;
  int chnl = get_int_suffix(commands);
  if (chnl < 1 | chnl > 4) {
    system_error.set_error(GenericError::BadSuffix);
    return;
  }
  interface.println(model.getPhase(chnl));
}

/*********************************************************/
// Display Commands
/*********************************************************/

/**
 * @brief Change the display mode of the lcd
 */
void changeMode(SCPI_C commands, SCPI_P params, Stream& interface) {
  if (check_param_num(1, params.Size()))
    return;
  int mode = strtol(params[0], NULL, 10);
  if (mode < 0 | mode > 5) {
    system_error.set_error(GenericError::BadSuffix);
    return;
  }
  if (mode == 5)
    viewState.setMode(ViewState::Mode::REMOTE);
  else
    viewState.setMode(static_cast<ViewState::Mode>(mode));
}

/*********************************************************/
// SCPI Error handling
/*********************************************************/

/**
 * @brief Get the last error (most recent) from the buffer
 */
void GetLastEror(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  if (check_param_num(0, parameters.Size()))
    return;
  int err_code = system_error.get_error();
  strcpy_P(system_error.message_buffer, get_error_ptr(err_code));
  interface.print(err_code);
  interface.print(F(" - "));
  interface.println(system_error.message_buffer);
  interface.flush();

  if (err_code != 0)
    viewState.setMode(viewState.last_mode);
}

/**
 * @brief Handler for SCPI errors
 */
void SCPIErrorHandler(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // This function is called every time a SCPI error occurs

  /* For BufferOverflow errors, the rest of the message, still in the
  interface buffer or not yet received, will be processed later and probably
  trigger another kind of error. Here we flush the incomming message*/
  system_error.set_error(parser.last_error);
  if (parser.last_error == SCPI_Parser::ErrorCode::BufferOverflow) {
    delay(2);
    while (interface.available()) {
      delay(2);
      interface.read();
    }
  }
};

#endif