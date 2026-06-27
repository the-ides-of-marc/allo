#include "allo_test_mem.h"
#include "allo/internal/allo_math.h"
#include <stddef.h>
#include <stdlib.h>
#include <unity_internals.h>

void *allo_test_mem_alloc(void **dest, size_t size, size_t align, size_t line) {
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
    UnityFail("allocation failed", line);
  }

  uintptr_t addr = (uintptr_t)*dest;
  uintptr_t aligned_addr = allo_math_align_up(addr, align);
  return (void *)aligned_addr;
}
