/********************************************************************************
 * @file: viewState.h
 *
 * @brief: ViewState class and mode setting functions
 ******************************************************************************/

#ifndef VIEWSTATE_H
#define VIEWSTATE_H

#include "Arduino.h"

class ViewState {
 public:
  bool update;
  enum class Mode : uint8_t {
    NORMAL = 0,
    FOCUS1 = 1,
    FOCUS2 = 2,
    FOCUS3 = 3,
    FOCUS4 = 4,
    ERROR = 5,
    REMOTE = 6
  };
  Mode mode;
  Mode last_mode = Mode::NORMAL;
  int volts[4];
  int phases[4];
  float freq;

  ViewState() { reset(); }

  void reset() {
    update = false;
    mode = Mode::NORMAL;
    for (int i = 0; i < 4; i++) {
      volts[i] = 0;
      phases[i] = 0;
    }
    freq = 50000;
  }

  void setMode(Mode newMode) {
    update = true;
    if (mode != Mode::ERROR)
      last_mode = mode;
    mode = newMode;
  }

  /**
   * @brief Populates viewState data for a given channel voltage
   *
   * @param channel channel number (1-4)
   * @param value pointer to voltage value
   */
  void setVolts(int channel, float* value) {
    int reduced_value = (int)round((*value) * v_multipler);
    volts[channel - 1] = reduced_value;
  }

  /**
   * @brief Gets the voltage value for a given channel
   *
   * @param channel channel number (1-4)
   * @return voltage value
   */
  float getVolts(int channel) {
    return volts[channel - 1] / ((float)v_multipler);
  }

  /**
   * @brief Populates viewState phase data for a given channel
   *
   * @param channel channel number (1-4)
   * @param phase pointer to phase value
   */
  void setPhase(int channel, float* phase) {
    int reduced_phase = (int)round((*phase) * p_multipler);
    phases[channel - 1] = reduced_phase;
  }

  /**
   * @brief Gets the phase value for a given channel
   *
   * @param channel channel number (1-4)
   * @return phase value in between (0, 360)
   */
  float getPhase(int channel) {
    float phase = phases[channel - 1] / ((float)p_multipler);
    if (phase < 0)
      phase += 360;
    return phase;
  }

 private:
  byte v_multipler = 10;
  byte p_multipler = 100;
};

#endif