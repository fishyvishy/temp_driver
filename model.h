/******************************************************************************
    @file:  ad9106_model.h

    @brief: Model abstracting the EVAL-AD9106 board
******************************************************************************/

#ifndef MODEL_H
#define MODEL_H

#include <AD9106.h>
#include "Arduino.h"
#include "config.h"
#include "global_error.h"

extern GlobalError system_error;

class Model {
 public:
  AD9106 dac;
  Model(int CS) : dac(CS) {};

  /**
   * @brief: Initialize the AD9106 and start SPI communication
   */
  void begin() {
    // Initialize pins on device with op-amps enabled
    dac.begin(true);

    // Start SPI communication at 14MHz (Arduino Clock Speed)
    dac.spi_init(14000000);
    reset();
  }

  /**
   * @brief: Update the AD9106 model with new register values
   */
  void update() {
    dac.update_pattern();

    // Check for errors after updating
    AD9106::ErrorCode err = dac._last_error;
    if (err != AD9106::NO_ERROR) {
      system_error.set_error(err);
    }
  }

  /**
   * @brief: Reset the AD9106 and configure sinew waves
   */
  void reset() {
    // Reset registers
    dac.reg_reset();
    delay(1);

    // Configure sine waves on each channel
    for (int i = 1; i < 5; i++) {
      dac.setDDSsine(CHNL(i));
    }

    // Default Frequency
    dac.setDDSfreq(50000);

    // Characterized phases/amplitides with this pattern period. Not necessary
    dac.spi_write(dac.PAT_PERIOD, 0x8fff);
  }

  // Pattern functions
  void start() { dac.start_pattern(); }
  void stop_pattern() { dac.stop_pattern(); }

  /**
   * @brief: Set voltage on channel
   * @param chnl: Channel number
   * @param voltage: Voltage to set
   *
   * @returns 0 if voltage was set, 1 otherwise
   */
  int setVoltage(int chnl, float voltage) {
    float lower_bound = ((float)dac_amp_thesholds[0]) / 10.0;
    float upper_bound = ((float)dac_amp_thesholds[3]) / 10.0;

    if (voltage < lower_bound || voltage > upper_bound) {
      system_error.set_error(GenericError::ParamOutOfRange);
      return NULL;
    }

    int16_t val = v_to_addr(voltage, chnl);
    if (val != NULL) {
      dac.set_CHNL_DGAIN(static_cast<CHNL>(chnl), val);
      return 1;
    }
    return 0;
  }

  float getVoltage(int chnl) { ; }

  // AD9106 register access functions
  uint16_t readReg(uint16_t add) { return dac.spi_read(add); }
  void writeReg(uint16_t add, int16_t val) {
    dac.stop_pattern();
    dac.spi_write(add, val);
  }

  // DDS Frequency functions
  void setFreq(float freq) { dac.setDDSfreq(freq); }
  float getFreq() { return dac.getDDSfreq(); }

  /**
   * @brief: Set phase on channel
   */
  void setPhase(int chnl, float phase) {
    if (phase < 0) {
      phase += 360;
    }

    // Define channel 1 to be baseline for phase offsets
    // if (chnl != 1) {
    //   float offset = interpolate_offset(chnl);
    //   // interface.println(offset);
    //   phase -= offset;
    // }

    uint16_t val = (uint16_t)round_float(phase * (pow(2, 16) - 1) / 360);
    dac.set_CHNL_prop(DDS_PHASE, static_cast<CHNL>(chnl), val);
  }

  /**
   * @brief: Get phase on channel
   * @param chnl: Channel number
   *
   * @returns Phase in degrees (-180 to 180)
   */
  float getPhase(int chnl) {
    uint16_t reg_val = dac.get_CHNL_prop(DDS_PHASE, static_cast<CHNL>(chnl));
    // Brackets important to avoid overflow errors
    float phase = 360.0f * (reg_val / (pow(2, 16) - 1));
    if (phase > 180) {
      phase -= 360;
    }
    return phase;
  }

 private:
  // Interpolate phase offset using offsets array
  // float interpolate_offset(int chan) {
  //   float freq = dac.getDDSfreq();
  //   float f_step = dac.fclk / pow(2, 24);

  //   // get pointer to offset array for channel in PROGMEM
  //   const int* offset_ptr =
  //       (const int*)pgm_read_word_near(&(dacphase_offsets[chan - 2]));

  //   if (offset_ptr == NULL) {
  //     return 0;
  //   }

  //   int index = _get_index(&freq);
  //   int freq_val = _get_freq_val(index, f_step);
  //   // TODO: add offsets for frequency vals up to 15 to avoid ugly code here
  //   if (freq_val <= 15 && !(freq_val == 11 || freq_val == 13)) {
  //     int offset = (int)pgm_read_word_near(offset_ptr + freq_val - 1);
  //     return (float)offset / 10000.0;
  //   }
  //   if (f_step * _get_freq_val(index, f_step) > freq) {
  //     index -= 1;
  //   }

  //   float freq_1 = f_step * _get_freq_val(index + 1, f_step);
  //   float freq_2 = f_step * _get_freq_val(index, f_step);

  //   index -=
  //       8;  // subtract 8 for removal of duplicates in offset array (see SOP)
  //   int off_1 = pgm_read_word_near(offset_ptr + index + 1);
  //   int off_2 = pgm_read_word_near(offset_ptr + index);

  //   float off_1f = (float)off_1 / 10000.0;
  //   float off_2f = (float)off_2 / 10000.0;

  //   float slope = (float)(off_1f - off_2f) / (freq_1 - freq_2);

  //   return slope * (freq - freq_1) + off_1f;
  // }

  // int _get_index(float* freq) {
  //   float index = (log10(*freq) - 1) * 69 / 4;
  //   return round(index);
  // }

  // int _get_freq_val(int index, float f_step) {
  //   // index * 4/69. Hardcoded to avoid floating point errors
  //   float inc = (float)index * (4.0 / 69);
  //   float val = pow(10, 1 + inc);
  //   return round_float(val / f_step);
  // }

  int round_float(float num) {
    return (num >= 0) ? (int)(num + 0.5f) : (int)(num - 0.5f);
  }

  /**
   * @brief: Convert voltage to value for address
   *
   * @param voltage: Voltage to convert
   * @param chan: Channel number
   *
   * @returns value for address
   */
  int16_t v_to_addr(float voltage, int chan) {
    // const float *coeffs =  (const float*) pgm_read_ptr(&(dac_amp_coeffs[chan
    // - 1]));
    int range_index = 0;
    for (int i = 0; i < 3; i++) {
      if (dac_amp_thesholds[i] / 10 <= voltage &&
          voltage <= dac_amp_thesholds[i + 1] / 10) {
        range_index = 6 * i;
        break;
      }
    }

    float freq = dac.getDDSfreq();
    float float_reader_buff;

    // absorb factor of 10^(-5) from fit function
    read_pgm_float(chan, range_index + 4, &float_reader_buff);
    float numerator = (100 * voltage) - (10 * float_reader_buff);
    float freq_poly = 0;
    int freq_order = get_order(freq);
    for (int i = 0; i < 4; i++) {
      // get difference in magnitudes of polynomial term
      int order_diff = (exps[i] - 5) - freq_order * (i + 1);
      if (-10 <= order_diff && order_diff <= 10) {
        float freq_sigval = freq / pow(10, freq_order);
        read_pgm_float(chan, range_index + i, &float_reader_buff);
        freq_poly +=
            float_reader_buff * pow(freq_sigval, i + 1) * pow(10, -order_diff);
      }
    }

    read_pgm_float(chan, range_index + 5, &float_reader_buff);
    float addr = numerator / (freq_poly + float_reader_buff);
    return addr;
  }

  void read_pgm_float(int chan, int index, float* dest) {
    *dest = pgm_read_float_near(&dac_amp_coeffs[chan - 1][index]);
  }

  // float _get_amp_addr(float voltage, const float coeffs[6]) {
  //   float freq = dac.getDDSfreq();

  //   // absorb factor of 10^(-5) from fit function
  //   float numerator = (100 * voltage) - (10 * coeffs[4]);
  //   float freq_poly = 0;
  //   int freq_order = get_order(freq);
  //   for (int i = 0; i < 4; i++) {
  //     // get difference in magnitudes of polynomial term
  //     int order_diff = (exps[i] - 5) - freq_order * (i + 1);
  //     if (-10 <= order_diff && order_diff <= 10) {
  //       float freq_sigval = freq / pow(10, freq_order);
  //       freq_poly += coeffs[i] * pow(freq_sigval, i + 1) * pow(10,
  //       -order_diff);
  //     }
  //   }

  //   float addr = numerator / (freq_poly + coeffs[5]);
  //   return addr;
  // }
};

#endif