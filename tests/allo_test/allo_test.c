#include "allo_test.h"
#include "allo/internal/allo_math.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unity.h>
#include <unity_internals.h>

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
  uintptr_t aligned_addr = allo_math_align_up(addr, align);
  return (void *)aligned_addr;
}
