#define RETURN_IF_ERROR(code)                                                  \
  ({                                                                           \
    if (code < 0) {                                                            \
      log_error_code(code);                                                    \
      return code;                                                             \
    }                                                                          \
  })

#define RETURN_CODE_IF_ERROR(code, propagated_code)                            \
  ({                                                                           \
    if (code < 0) {                                                            \
      log_error_code(code);                                                    \
      return propagated_code;                                                  \
    }                                                                          \
  })

void log_error_code(int code);
