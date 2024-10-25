/******************************************************************************
    @file:  global_error.h

    @brief: Global error handling and error buffer
******************************************************************************/

#ifndef GLOBAL_ERROR_H
#define GLOBAL_ERROR_H

#include <avr/pgmspace.h>
#include "error_table.h"

#define MAX_BUFFER_SIZE 5  // maximum number of errors in buffer
#define MAX_MSG_SIZE 16    // lcd screen is 16x2

class GlobalError {
 public:
  void (*ErrorHandler)();
  char message_buffer[MAX_MSG_SIZE + 1];  // buffer to help print messages

  /**
   * @brief Constructor for the GlobalError class.
   *
   * Initializes the error handler, buffer size, and write index.
   *
   * @param func The error handling function to be called when an error occurs.
   */
  GlobalError(void (*func)()) {
    ErrorHandler = func;
    buffer_size = 0;
    write_indx = 0;
  }

  /**
   * @brief Checks if there is an error in the queue.
   *
   * @return True if there is an error in the buffer, false otherwise.
   */
  bool is_error() { return !(buffer_size == 0); }

  /**
   * @brief Retrieves the last error (most recent) code from the buffer.
   *
   * @param read If true, retrieves but does not remove the last error from the
   * buffer.
   * @return The last error code from the buffer.
   */
  int get_error(bool read = false) {
    if (buffer_size == 0)
      return 0;

    int last_indx = (write_indx - 1 + MAX_BUFFER_SIZE) % MAX_BUFFER_SIZE;
    int last_error = error_buffer[last_indx];
    if (!read) {
      write_indx = last_indx;
      buffer_size--;
    }
    return last_error;
  }

  /**
   * @brief Sets an error in the buffer.
   *
   * @tparam ErrorCodeType The type of the error code.
   * @param errorCode The error code to be set.
   */
  template <typename ErrorCodeType>
  void set_error(ErrorCodeType errorCode) {
    int code = get_error_code(errorCode);

    error_buffer[write_indx] = code;
    write_indx = (write_indx + 1) % MAX_BUFFER_SIZE;
    if (buffer_size < MAX_BUFFER_SIZE) {
      buffer_size++;
    }
    this->ErrorHandler();
  }

 private:
  int error_buffer[MAX_BUFFER_SIZE];  // circular buffer for errors
  int buffer_size;                    // size of the error buffer
  int write_indx;                     // index to write new errors to
};

#endif
