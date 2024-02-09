#include <stdio.h>
#include <errno.h>

#include "constants.h"

void log_error_code(int code) {
  if (ERROR_LOGGING_ENABLED && code < 0) {
    printf("LOG - ERROR CODE: ");
    switch (code) {
    case EADDRINUSE:
      printf("EADDRINUSE");
    default:
      printf("UNKOWN ERROR CODE (%d)", code);
    }
    printf("\n");
  };
}
