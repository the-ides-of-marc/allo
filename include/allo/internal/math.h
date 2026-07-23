#ifndef ALLO_MATH_H
#define ALLO_MATH_H

#include "allo/config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline uint8_t allo_math_popcount_uint8(uint8_t n) {
  n -= (n >> 1) & 0x55;
  n = (n & 0x33) + ((n >> 2) & 0x33);
  n = (n + (n >> 4)) & 0x0f;
  return n;
}

static inline uint8_t allo_math_popcount_uint16(uint16_t n) {
  n -= (n >> 1) & 0x5555;
  n = (n & 0x3333) + ((n >> 2) & 0x3333);
  n = (n + (n >> 4)) & 0x0f0f;
  return (uint8_t)((n * 0x0101) >> 8);
}

static inline uint8_t allo_math_popcount_uint32(uint32_t n) {
  n -= (n >> 1) & 0x55555555;
  n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
  n = (n + (n >> 4)) & 0x0f0f0f0f;
  return (uint8_t)((n * 0x01010101) >> 24);
}

static inline uint8_t allo_math_popcount_uint64(uint64_t n) {
  n -= (n >> 1) & 0x5555555555555555;
  n = (n & 0x3333333333333333) + ((n >> 2) & 0x3333333333333333);
  n = (n + (n >> 4)) & 0x0f0f0f0f0f0f0f0f;
  return (uint8_t)((n * 0x0101010101010101) >> 56);
}

static inline uint8_t allo_math_popcount_size_t(size_t n) {
#if SIZE_MAX == UINT64_MAX
  return allo_math_popcount_uint64(n);
#elif SIZE_MAX == UINT32_MAX
  return allo_math_popcount_uint32(n);
#elif SIZE_MAX == UINT16_MAX
  return allo_math_popcount_uint16(n);
#elif SIZE_MAX == UINT8_MAX
  return allo_math_popcount_uint8(n);
#else
#error "unsupported size_t"
#endif
}

static inline uint8_t allo_math_ctz_size_t(size_t n) {
  if (n == 0) {
    return sizeof(size_t);
  }
  n ^= n - 1;
  return allo_math_popcount_size_t(n) - 1;
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
#if SIZE_MAX == UINT64_MAX
  n |= n >> 32;
#endif
  return ++n;
}

// Returns if the uintptr_t is aligned.
static inline bool allo_math_is_aligned(uintptr_t n, size_t align) {
  ALLO_ASSERT(allo_math_is_pow2(align), "alignment must be a power of 2");
  return n % align == 0;
}

static inline uintptr_t allo_math_align_up(uintptr_t n, size_t align) {
  ALLO_ASSERT(allo_math_is_pow2(align), "alignment must be a power of 2");
  return (n + align - 1) & ~(align - 1);
}

static inline uintptr_t allo_math_align_down(uintptr_t n, size_t align) {
  ALLO_ASSERT(allo_math_is_pow2(align), "alignment must be a power of 2");
  return n & ~(align - 1);
}

#endif // !ALLO_MATH_H
