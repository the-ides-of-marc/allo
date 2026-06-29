#include "allo_test_assert.h"
#include "allo_test_io.h"
#include <unity_internals.h>

void allo_test_assert_status(allo_status expected, allo_status actual,
                             const char *message, size_t line) {
  enum { BUF_SIZE = 1 << 10 };
  if (expected != actual) {
    char buf[BUF_SIZE] = {0};
    size_t offset = 0;
    allo_test_snprintf(buf, BUF_SIZE, &offset, "%s: ", message);
    allo_test_snprintf(
        buf, BUF_SIZE, &offset, "expected=%d (%s) actual=%d (%s)", expected,
        allo_status_str(expected), actual, allo_status_str(actual));
    UnityFail(buf, line);
  }
}
