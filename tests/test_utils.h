#ifndef ALLO_TEST_UTILS_H
#define ALLO_TEST_UTILS_H

#include "allo/status.h"
#include <stddef.h>
#include <unity.h>

// Performs malloc on `*dest` and returns an address within
// [`*dest`..`*dest`+`size`) that is aligned to `align`
// If malloc returns NULL, it is written to `*dest` and the return is also NULL.
void *malloc_aligned(void **dest, size_t size, size_t align);

void test_utils_assert_status(enum allo_status expected,
                              enum allo_status actual, const char *message,
                              size_t line);
#define TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(expected, actual, message)           \
  test_utils_assert_status((expected), (actual), (message), __LINE__);

#endif // !ALLO_TEST_UTILS_H
