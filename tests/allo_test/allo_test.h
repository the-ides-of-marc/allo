#ifndef ALLO_TEST_UTILS_H
#define ALLO_TEST_UTILS_H

#include <stddef.h>
#include <unity.h>

// IWYU pragma: begin_exports
#include "allo_test_assert.h"
#include "allo_test_io.h"
// IWYU pragma: end_exports

// Performs malloc on `*dest` and returns an address within
// [`*dest`..`*dest`+`size`) that is aligned to `align`
// If malloc returns NULL, it is written to `*dest` and the return is also NULL.
void *test_utils_malloc_aligned(void **dest, size_t size, size_t align,
                                size_t line);
#define TEST_UTILS_MALLOC_ALIGNED(dest, size, align)                           \
  test_utils_malloc_aligned((dest), (size), (align), __LINE__);

#endif // !ALLO_TEST_UTILS_H
