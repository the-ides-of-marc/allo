#include "allo_test_io.h"
#include <stdarg.h>
#include <unity.h>

void allo_test_snprintf(char *restrict buffer, size_t maxlen, size_t *offset,
                        const char *restrict format, ...) {
  TEST_ASSERT_NOT_NULL_MESSAGE(buffer, "buffer is NULL");
  TEST_ASSERT_TRUE_MESSAGE(maxlen > 0, "maxlen must not be zero");

  va_list args;
  va_start(args, format);
  size_t write_index = offset ? *offset : 0;

  TEST_ASSERT_TRUE_MESSAGE(write_index < maxlen,
                           "buffer index before write is out of bounds");

  int written =
      vsnprintf(buffer + write_index, maxlen - write_index, format, args);
  va_end(args);

  TEST_ASSERT_TRUE_MESSAGE(written >= 0, "error in vsnprintf");
  TEST_ASSERT_TRUE_MESSAGE(written < maxlen, "message truncated");
  write_index += written;
  TEST_ASSERT_TRUE_MESSAGE(write_index < maxlen,
                           "buffer index after write is out of bounds");
  TEST_ASSERT_EQUAL_CHAR_MESSAGE('\0', buffer[write_index],
                                 "buffer is terminated with \0");
  if (offset) {
    *offset = write_index;
  }
}
