#include "allo_test_mem.h"
#include "allo/internal/math.h"
#include "allo_test_io.h"
#include <stddef.h>
#include <stdlib.h>
#include <unity_internals.h>

void *allo_test_mem_alloc(void **dest, size_t size, size_t align, size_t line) {
  enum { BUF_SIZE = 1 << 10 };
  char buf[BUF_SIZE] = {0};
  size_t offset = 0;
  allo_test_snprintf(buf, BUF_SIZE, &offset, "allo_test_mem_alloc: ");

  if (!dest) {
    allo_test_snprintf(buf, BUF_SIZE, &offset,
                       "pointer to destination should not be NULL");
    UnityFail(buf, line);
  }
  if (!size) {
    allo_test_snprintf(buf, BUF_SIZE, &offset, "size must not be 0");
    UnityFail(buf, line);
  }
  if (!allo_math_is_pow2(align)) {
    allo_test_snprintf(buf, BUF_SIZE, &offset,
                       "alignment must be a power of 2");
    UnityFail(buf, line);
  }

  *dest = malloc(size + align);
  if (!*dest) {
    allo_test_snprintf(buf, BUF_SIZE, &offset, "malloc failed");
    UnityFail(buf, line);
  }

  uintptr_t addr = (uintptr_t)*dest;
  uintptr_t aligned_addr = allo_math_align_up(addr, align);
  return (void *)aligned_addr;
}
