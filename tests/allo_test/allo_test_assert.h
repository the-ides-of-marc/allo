#ifndef ALLO_TEST_ASSERT_H
#define ALLO_TEST_ASSERT_H

#include "allo/allo_status.h"
#include <stddef.h>

void allo_test_assert_status(allo_status expected, allo_status actual,
                             const char *message, size_t line);
#define TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(expected, actual, message)       \
  allo_test_assert_status((expected), (actual), (message), __LINE__);

#endif // !ALLO_TEST_ASSERT_H
