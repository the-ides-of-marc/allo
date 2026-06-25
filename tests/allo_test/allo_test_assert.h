#ifndef ALLO_TEST_ASSERT_H
#define ALLO_TEST_ASSERT_H

#include "allo/allo_status.h"
#include <stddef.h>
#include <stdint.h>

void allo_test_assert_status(allo_status expected, allo_status actual,
                             const char *message, size_t line);
#define ALLO_TEST_ASSERT_STATUS_MSG(expected, actual, message)                 \
  allo_test_assert_status((expected), (actual), (message), __LINE__)

void allo_test_assert_mem(uintptr_t expected, uintptr_t actual,
                          const char *message, size_t line);
#define ALLO_TEST_ASSERT_MEM_MSG(expected, actual, message)                    \
  allo_test_assert_mem((expected), (actual), (message), __LINE__)

void allo_test_assert_size_t(size_t expected, size_t actual,
                             const char *message, size_t line);
#define ALLO_TEST_ASSERT_SIZE_T_MSG(expected, actual, message)                 \
  allo_test_assert_size_t((expected), (actual), (message), __LINE__)

#endif // !ALLO_TEST_ASSERT_H
