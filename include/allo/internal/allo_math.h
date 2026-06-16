#ifndef ALLO_MATH_H
#define ALLO_MATH_H

#include "allo/allo_config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Returns the popcount of an uint8.
static inline uint8_t allo_math_popcount_uint8(uint8_t n) {
  n = n - ((n >> 1) & 0x55);
  n = (n & 0x33) + ((n >> 2) & 0x33);
  return (n + (n >> 4)) & 0x0F;
}

// Returns if `n` is a power of 2.
static inline bool allo_math_is_pow2(size_t n) {
  return n > 0 && (n & (n - 1)) == 0;
}

// Returns the closest `m` such that `m` >= `n` and `m` is a power of 2.
// `n` must not be zero.
static inline size_t allo_math_round_pow2(size_t n) {
  ALLO_ASSERT(n > 0, "n must be non-zero");
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

// Returns if the uintptr_t is aligned.
static inline bool allo_math_is_aligned_uintptr(uintptr_t addr, size_t align) {
  ALLO_ASSERT(align > 0, "alignment must not be 0");
  return addr % align == 0;
}

// Returns if the pointer is aligned.
static inline bool allo_math_is_aligned_ptr(void *ptr, size_t align) {
  ALLO_ASSERT(align > 0, "alignment must not be 0");
  return (uintptr_t)ptr % align == 0;
}

#endif // !ALLO_MATH_H
