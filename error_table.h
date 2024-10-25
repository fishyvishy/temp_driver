/******************************************************************************
 * @file:  error_table.h
 *
 * @brief: Error tables and getters
 ******************************************************************************/

#ifndef ERROR_TABLE_H
#define ERROR_TABLE_H

#include <AD9106.h>
#include <Vrekrer_scpi_parser.h>
#include <avr/pgmspace.h>

const int SCPI_PRIORITY = 1;
const int GENERIC_PRIORITY = 2;
const int AD9106_PRIORITY = 3;

enum GenericError {
  NoError = 0,
  TooManyParams = 201,
  TooFewParams = 202,
  UnknownParam = 203,
  ParamOutOfRange = 204,
  BadSuffix = 205
};

/*********************************************************/
// Error Tables
/*********************************************************/

const char gen_error_0[] PROGMEM = "No Error";
const char gen_error_1[] PROGMEM = "Too many Params";
const char gen_error_2[] PROGMEM = "Too few Params";
const char gen_error_3[] PROGMEM = "Unknown Param";
const char gen_error_4[] PROGMEM = "Out of Range";
const char gen_error_5[] PROGMEM = "Bad Channel Num";

const char scpi_error_1[] PROGMEM = "Unknown Cmd";
const char scpi_error_2[] PROGMEM = "Timeout";
const char scpi_error_3[] PROGMEM = "Buffer Ovf";

const char ad9106_error_1[] PROGMEM = "Mem Read Fail";
const char ad9106_error_2[] PROGMEM = "Odd Addr Err";
const char ad9106_error_3[] PROGMEM = "Short Period";
const char ad9106_error_4[] PROGMEM = "Short DOUT";
const char ad9106_error_5[] PROGMEM = "Short Pat Dly";
const char ad9106_error_6[] PROGMEM = "Large DOUT";

const char* const gen_error_table[] PROGMEM = {gen_error_0, gen_error_1,
                                               gen_error_2, gen_error_3,
                                               gen_error_4, gen_error_5};

const char* const scpi_error_table[] PROGMEM = {scpi_error_1, scpi_error_2,
                                                scpi_error_3};

const char* const ad9106_error_table[] PROGMEM = {
    ad9106_error_1, ad9106_error_2, ad9106_error_3,
    ad9106_error_4, ad9106_error_5, ad9106_error_6};

/*********************************************************/
// Error Data getters
/*********************************************************/

int get_error_code(SCPI_Parser::ErrorCode errorCode) {
  int code = 0;
  switch (errorCode) {
    case SCPI_Parser::ErrorCode::UnknownCommand:
      code = 1;
      break;
    case SCPI_Parser::ErrorCode::Timeout:
      code = 2;
      break;
    case SCPI_Parser::ErrorCode::BufferOverflow:
      code = 3;
      break;
    default:
      return 0;
  }
  return 100 * SCPI_PRIORITY + code;
};

int get_error_code(GenericError errorCode) {
  int code = 0;
  switch (errorCode) {
    case GenericError::NoError:
      code = 0;
      break;
    case GenericError::TooManyParams:
      code = 1;
      break;
    case GenericError::TooFewParams:
      code = 2;
      break;
    case GenericError::UnknownParam:
      code = 3;
      break;
    case GenericError::ParamOutOfRange:
      code = 4;
      break;
    case GenericError::BadSuffix:
      code = 5;
      break;
    default:
      return 0;
  }
  return 100 * GENERIC_PRIORITY + code;
};

int get_error_code(AD9106::ErrorCode errorCode) {
  int code = 0;
  switch (errorCode) {
    case AD9106::ErrorCode::MEM_READ_ERR:
      code = 1;
      break;
    case AD9106::ErrorCode::ODD_ADDR_ERR:
      code = 2;
      break;
    case AD9106::ErrorCode::PERIOD_SHORT_ERR:
      code = 3;
      break;
    case AD9106::ErrorCode::DOUT_START_SHORT_ERR:
      code = 4;
      break;
    case AD9106::ErrorCode::PAT_DLY_SHORT_ERR:
      code = 5;
      break;
    case AD9106::ErrorCode::DOUT_START_LG_ERR:
      code = 6;
      break;
    default:
      return 0;
  }
  return 100 * AD9106_PRIORITY + code;
};

/**
 * @brief Gets the error message pointer from the error code
 */
const char* get_error_ptr(int errorCode) {
  const char* ptr;
  if (errorCode == 0) {
    ptr = (const char*)pgm_read_ptr(&gen_error_table[0]);
    return ptr;
  }
  int table = errorCode / 100;
  int index = errorCode % 100;

  switch (table) {
    case SCPI_PRIORITY:
      ptr = (const char*)pgm_read_ptr(&scpi_error_table[index - 1]);
      break;
    case GENERIC_PRIORITY:
      // First error is NO Error in table, so don't decrement index
      ptr = (const char*)pgm_read_ptr(&gen_error_table[index]);
      break;
    case AD9106_PRIORITY:
      ptr = (const char*)pgm_read_ptr(&ad9106_error_table[index - 1]);
      break;
  }
  return ptr;
};

#endif