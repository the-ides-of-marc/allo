#include "allo_test_assert.h"
#include "allo_test_io.h"
#include <unity_internals.h>

void allo_test_assert_status(allo_status expected, allo_status actual,
                             const char *message, size_t line) {
  enum { BUFSIZE = 1 << 10 };
  if (expected != actual) {
    char buf[BUFSIZE] = {0};
    size_t offset = 0;
    allo_test_snprintf(buf, BUFSIZE, &offset, "%s: ", message);
    allo_test_snprintf(buf, BUFSIZE, &offset, "expected=%d (%s) actual=%d (%s)",
                       expected, allo_status_str(expected), actual,
                       allo_status_str(actual));
    UnityFail(buf, line);
  }
}

void allo_test_assert_mem(uintptr_t expected, uintptr_t actual,
                          const char *message, size_t line) {
  enum { BUFSIZE = 1 << 6 };
  if (expected != actual) {
    char buf[BUFSIZE] = {0};
    size_t offset = 0;
    allo_test_snprintf(buf, BUFSIZE, &offset, "%s: ", message);
    allo_test_snprintf(buf, BUFSIZE, &offset, "expected=0x%lx actual=0x%lx",
                       expected, actual);
    UnityFail(buf, line);
  }
}

void allo_test_assert_size_t(size_t expected, size_t actual,
                             const char *message, size_t line) {
  enum { BUFSIZE = 1 << 6 };
  if (expected != actual) {
    char buf[BUFSIZE] = {0};
    size_t offset = 0;
    allo_test_snprintf(buf, BUFSIZE, &offset, "%s: ", message);
    allo_test_snprintf(buf, BUFSIZE, &offset, "expected=%zu actual=%zu",
                       expected, actual);
    UnityFail(buf, line);
  }
}
