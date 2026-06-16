#include "test_utils.h"
#include "allo/allo_status.h"
#include "allo/internal/allo_math.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unity.h>
#include <unity_internals.h>

static void test_utils_snprintf(char *restrict buffer, size_t maxlen,
                                size_t *offset, const char *restrict format,
                                ...) __attribute__((format(printf, 4, 5)));

void *test_utils_malloc_aligned(void **dest, size_t size, size_t align,
                                size_t line) {
  if (!dest) {
    UnityFail("pointer to destination should not be NULL", line);
  }
  if (!size) {
    UnityFail("size must not be 0", line);
  }
  if (!allo_math_is_pow2(align)) {
    UnityFail("alignment must be a power of 2", line);
  }

  *dest = malloc(size + align);
  if (!*dest) {
    return NULL;
  }

  uintptr_t addr = (uintptr_t)*dest;
  uintptr_t aligned_addr = (addr + align - 1) & ~(align - 1);
  return (void *)aligned_addr;
}

void test_utils_assert_status(allo_status expected, allo_status actual,
                              const char *message, size_t line) {
  enum { BUFSIZE = 1 << 6 };
  if (expected != actual) {
    char buffer[BUFSIZE] = {0};
    size_t offset = 0;
    test_utils_snprintf(buffer, BUFSIZE, &offset, "%s: ", message);
    test_utils_snprintf(
        buffer, BUFSIZE, &offset, "expected=%d (%s) actual=%d (%s)", expected,
        allo_status_str(expected), actual, allo_status_str(actual));
    UnityFail(buffer, line);
  }
}

static void test_utils_snprintf(char *restrict buffer, size_t maxlen,
                                size_t *offset, const char *restrict format,
                                ...) {
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
