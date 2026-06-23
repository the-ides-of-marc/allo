#ifndef ALLO_POOL_H
#define ALLO_POOL_H

#include "allo/allo_allocator.h"
#include "allo_status.h"
#include "internal/allo_defines.h"
#include "internal/allo_math.h"
#include <stddef.h>
#include <stdint.h>

// Fixed size pool allocator.
typedef struct allo_pool allo_pool;

// Asserts the state of a pool allocator.
static inline void allo_pool_assert(const allo_pool *p);

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
static inline allo_status allo_pool_init(allo_pool *ALLO_RESTRICT p,
                                         void *ALLO_RESTRICT buf,
                                         size_t buf_size, size_t chunk_size,
                                         size_t align);

// Allocates a new chunk of memory of `p->chunk_size` and writes it to `*dest`.
// The free list is then updated to point to the next free chunk of memory.
static inline allo_status
allo_pool_alloc(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                allo_pool *ALLO_RESTRICT p);

// Frees the memory allocated at `ptr`.
// The free list is then updated to point to `ptr`.
static inline void allo_pool_free(allo_pool *ALLO_RESTRICT p,
                                  void *ALLO_RESTRICT ptr);

// Frees all memory allocated on allocator `p`.
static inline void allo_pool_free_all(allo_pool *p);

// Returns a allocator type from a pool allocator.
static inline allo_allocator allo_allocator_from_pool(allo_pool *p);

// VTable for pool allocator.
extern const allo_allocator_vtable allo_pool_vtable;

struct allo_pool {
  void *free_list;
  uintptr_t start;
  uintptr_t end;
  size_t chunk_size;
  size_t align;
};

static inline void allo_pool_assert(const allo_pool *p) {
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

  ALLO_ASSERT(allo_math_is_aligned(p->start, p->align),
              "start must be aligned");
  ALLO_ASSERT(allo_math_is_aligned(p->end, p->align), "end must be aligned");

  if (p->free_list) {
    ALLO_ASSERT(p->start <= (uintptr_t)p->free_list,
                "free list must be >= start of memory region");
    ALLO_ASSERT((uintptr_t)p->free_list < p->end,
                "free list must be < end of memory region");
    ALLO_ASSERT(p->start <= (uintptr_t)p->free_list &&
                    (uintptr_t)p->free_list < p->end,
                "free list must be within the allocator's memory region");
    ALLO_ASSERT(allo_math_is_aligned((uintptr_t)p->free_list, p->align),
                "free list must be aligned");
  }
  (void)p;
}

static inline allo_status allo_pool_init(allo_pool *ALLO_RESTRICT p,
                                         void *ALLO_RESTRICT buf,
                                         size_t buf_size, size_t chunk_size,
                                         size_t align) {
  if (!p || !buf) {
    return ALLO_ERR_INVALID_NULL;
  }
  if (!buf_size || !chunk_size) {
    return ALLO_ERR_INVALID_SIZE;
  }
  if (!align) {
    return ALLO_ERR_INVALID_ALIGNMENT;
  }

  align = allo_math_round_pow2(align);
  align = align >= sizeof(void *) ? align : sizeof(void *);
  chunk_size = chunk_size >= sizeof(void *) ? chunk_size : sizeof(void *);
  chunk_size = allo_math_align_up(chunk_size, align);

  ALLO_ASSERT(allo_math_is_pow2(align), "alignment must be a power of 2");
  ALLO_ASSERT(align >= sizeof(void *),
              "alignment must be greater than sizeof(void*)");
  ALLO_ASSERT(chunk_size >= align,
              "chunk size must be at least the size of the alignment");
  ALLO_ASSERT(chunk_size % align == 0,
              "chunk size must be a multiple of alignment");

  if (chunk_size > buf_size) {
    return ALLO_ERR_INVALID_SIZE;
  }

  size_t chunk_count = buf_size / chunk_size;
  ALLO_ASSERT(chunk_count > 0, "chunk count must be non-zero");

  if (!allo_math_is_aligned((uintptr_t)buf, align)) {
    return ALLO_ERR_NOT_ALIGNED;
  }

  p->chunk_size = chunk_size;
  p->align = align;
  p->free_list = buf;
  p->start = (uintptr_t)buf;
  p->end = p->start + chunk_count * p->chunk_size;
  ALLO_ASSERT((uintptr_t)p->end <= (uintptr_t)buf + buf_size,
              "end must be within input memory region");

  void **curr_chunk = buf;
  for (size_t i = 0; i < chunk_count - 1; ++i) {
    void *next = (uint8_t *)curr_chunk + p->chunk_size;
    *curr_chunk = next;
    curr_chunk = next;
  }
  *curr_chunk = NULL;

  allo_pool_assert(p);
  return ALLO_OK;
}

static inline allo_status
allo_pool_alloc(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                allo_pool *ALLO_RESTRICT p) {
  allo_pool_assert(p);

  *dest = NULL;
  void **addr = p->free_list;
  if (!addr) {
    return ALLO_OOM;
  }

  *dest = addr;
  p->free_list = *addr;

  allo_pool_assert(p);
  return ALLO_OK;
}

// Frees the memory allocated at `ptr`.
// The free list is then updated to point to `ptr`.
static inline void allo_pool_free(allo_pool *ALLO_RESTRICT p,
                                  void *ALLO_RESTRICT ptr) {
  allo_pool_assert(p);
  ALLO_ASSERT(p->start <= (uintptr_t)ptr,
              "ptr must be >= start of memory region");
  ALLO_ASSERT((uintptr_t)ptr < p->end, "ptr must be < end of memory region");
  ALLO_ASSERT(((uintptr_t)ptr - (uintptr_t)p->start) % p->chunk_size == 0,
              "region in [start..ptr] must be a multiple of chunk size");
  ALLO_ASSERT(allo_math_is_aligned((uintptr_t)ptr, p->align),
              "ptr must be aligned");

  void **next_ptr = ptr;
  *next_ptr = p->free_list;
  p->free_list = ptr;

  allo_pool_assert(p);
}

static inline void allo_pool_free_all(allo_pool *p) {
  allo_pool_assert(p);
  p->free_list = &p->start;
  allo_pool_assert(p);
}

static inline allo_allocator allo_allocator_from_pool(allo_pool *p) {
  allo_pool_assert(p);
  return (allo_allocator){
      .allocator = p,
      .vtable = &allo_pool_vtable,
  };
}

#endif // !ALLO_POOL_H
