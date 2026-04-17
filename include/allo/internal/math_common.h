#ifndef ALLO_MATH_COMMON_H
#define ALLO_MATH_COMMON_H

#include "defines.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static ALLO_FORCE_INLINE bool allo_is_pow2(size_t n) {
  return n > 0 && (n & (n - 1)) == 0;
}

static ALLO_FORCE_INLINE size_t allo_round_pow2(size_t n) {
  assert(n > 0 && "n must be non-zero");
  --n;
#if SIZE_MAX >= UINT8_MAX
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
#endif
#if SIZE_MAX >= UINT16_MAX
  n |= n >> 8;
#endif
#if SIZE_MAX >= UINT32_MAX
  n |= n >> 16;
#endif
#if SIZE_MAX >= UINT64_MAX
  n |= n >> 32;
#endif
  return ++n;
}

static ALLO_FORCE_INLINE bool allo_is_aligned(void *addr, size_t align) {
    return (uintptr_t)addr % align == 0;
}

#endif // !ALLO_MATH_COMMON_H
