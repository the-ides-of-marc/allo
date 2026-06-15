#ifndef ALLO_POOL_H
#define ALLO_POOL_H

#include "allo/allocator.h"
#include "internal/defines.h"
#include "internal/math_common.h"
#include "status.h"
#include <stddef.h>
#include <stdint.h>

// Fixed size pool allocator.
struct allo_pool {
  void *free_list;
  uintptr_t start;
  uintptr_t end;
  size_t chunk_size;
  size_t align;
};

static inline void allo_assert_pool(const struct allo_pool *p) {
  ALLO_ASSERT(p, "pool allocator must not be NULL");

  ALLO_ASSERT(p->align, "pool allocator alignment must be non-zero");
  ALLO_ASSERT(p->align >= sizeof(void *),
              "pool allocator alignment must be able to hold a pointer");
  ALLO_ASSERT(allo_math_is_pow2(p->align),
              "pool allocator must be a power of 2");

  ALLO_ASSERT(p->chunk_size, "chunk size must be non-zero");
  ALLO_ASSERT(p->chunk_size <= p->end - p->start,
              "chunk size must not be greater than the memory region");
  ALLO_ASSERT(p->chunk_size >= sizeof(void *),
              "chunk size must be able to hold a pointer");
  ALLO_ASSERT(p->chunk_size % p->align == 0,
              "chunk size must be a multiple of alignment");

  ALLO_ASSERT(p->start, "start must not be NULL");
  ALLO_ASSERT(p->end, "end must not be NULL");
  ALLO_ASSERT(p->start < p->end, "start must be lesser than end");

  ALLO_ASSERT(allo_math_is_addr_aligned(p->start, p->align),
              "start must be aligned");
  ALLO_ASSERT(allo_math_is_addr_aligned(p->end, p->align),
              "end must be aligned");

  if (p->free_list) {
    ALLO_ASSERT(p->start <= (uintptr_t)p->free_list,
                "free list must be >= start of memory region");
    ALLO_ASSERT((uintptr_t)p->free_list < p->end,
                "free list must be < end of memory region");
    ALLO_ASSERT(p->start <= (uintptr_t)p->free_list &&
                    (uintptr_t)p->free_list < p->end,
                "free list must be within the allocator's memory region");
    ALLO_ASSERT(allo_math_is_ptr_aligned(p->free_list, p->align),
                "free list must be aligned");
  }

  (void)p;
}

// Initializes a pool allocator that manages the memory region, `buf`, within
// [buf[0]..buf[buf_size-1]] and a given chunk size, `chunk_size`, and
// alignment, `align`.
// A Free list is initialized and points to the first chunk to be allocated.
// `align` will be padded to the nearest power of 2 >= `align`.
// `chunk_size` will be padded to the nearest multiple of the padded
// `align`.
// Both the padded `align` and padded `chunk_size` will be >= `sizeof(void *)`,
// as required for the free list to operate.
//
// ALLO_ERR_NULL is returned if `p` or `buf` is NULL.
// ALLO_ERR_INVALID_SIZE is returend if `buf_size` or `chunk_size` is 0 or if
// the padded `chunk_size` exceeds `buf_size`.
// ALLO_ERR_INVALID_ALIGN is returned if `align` is 0.
// ALLO_ERR_MEM_NOT_ALIGNED is returned if the `buf`  is not aligned with the
// rounded `align`.
enum allo_status allo_pool_init(struct allo_pool *ALLO_RESTRICT p,
                                void *ALLO_RESTRICT buf, size_t buf_size,
                                size_t chunk_size, size_t align);

// Allocates a new chunk of memory of `p->chunk_size` and writes it to `*dest`.
// The free list is then updated to point to the next free chunk of memory.
static inline enum allo_status
allo_pool_alloc(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                struct allo_pool *ALLO_RESTRICT p) {
  allo_assert_pool(p);

  *dest = NULL;
  void **addr = p->free_list;
  if (!addr) {
    return ALLO_OOM;
  }

  *dest = addr;
  p->free_list = *addr;

  allo_assert_pool(p);
  return ALLO_OK;
}

// Frees the memory allocated at `ptr`.
// The free list is then updated to point to `ptr`.
static inline void allo_pool_free(struct allo_pool *ALLO_RESTRICT p,
                                  void *ALLO_RESTRICT ptr) {
  allo_assert_pool(p);
  ALLO_ASSERT(p->start <= (uintptr_t)ptr,
              "ptr must be >= start of memory region");
  ALLO_ASSERT((uintptr_t)ptr < p->end, "ptr must be < end of memory region");
  ALLO_ASSERT(((uintptr_t)ptr - (uintptr_t)p->start) % p->chunk_size == 0,
              "region in [start..ptr] must be a multiple of chunk size");
  ALLO_ASSERT(allo_math_is_ptr_aligned(ptr, p->align), "ptr must be aligned");

  void **next_ptr = ptr;
  *next_ptr = p->free_list;
  p->free_list = ptr;

  allo_assert_pool(p);
}

static inline void allo_pool_free_all(struct allo_pool *p) {
  allo_assert_pool(p);
  p->free_list = &p->start;
  allo_assert_pool(p);
}

extern const struct allo_allocator_vtable allo_pool_vtable;

#endif // !ALLO_POOL_H
