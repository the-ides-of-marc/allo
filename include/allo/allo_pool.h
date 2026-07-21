#ifndef ALLO_POOL_H
#define ALLO_POOL_H

#include "allo/allo_allocator.h"
#include "allo/allo_config.h"
#include "allo_status.h"
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
// ALLO_ERR_INVALID_NULL is returned if `p` or `buf` is NULL.
// ALLO_ERR_INVALID_SIZE is returned if `buf_size` or `chunk_size` is 0 or if
// the padded `chunk_size` exceeds `buf_size`.
// ALLO_ERR_INVALID_ALIGNMENT is returned if `align` is 0.
// ALLO_ERR_NOT_ALIGNED is returned if the `buf`  is not aligned with the
// rounded `align`.
static inline allo_status allo_pool_init(allo_pool *restrict p,
                                         void *restrict buf, size_t buf_size,
                                         size_t chunk_size, size_t align);

// Returns the maximum number of chunks this pool can allocate.
static inline size_t allo_pool_chunk_cap(const allo_pool *p);

// Returns the number of free chunks remaining in this pool.
static inline size_t allo_pool_free_chunks(const allo_pool *p);

// Allocates a new chunk of memory of `p->chunk_size` and writes it to `*dest`.
// The free list is then updated to point to the next free chunk of memory.
// ALLO_ERR_INVALID_NULL is returned if `dest` or `p` is NULL.
// ALLO_OOM is returned if there is no more available chunk to allocate.
static inline allo_status allo_pool_alloc(void *restrict *restrict dest,
                                          allo_pool *restrict p);

// Frees the memory allocated at `ptr`.
// The free list is then updated to point to `ptr`.
// ALLO_ERR_INVALID_NULL is returned if `p` or `ptr` is NULL.
// ALLO_ERR_INVALID_ADDR is returned if `ptr` is not a valid chunk address.
static inline allo_status allo_pool_free(allo_pool *restrict p,
                                         void *restrict ptr);

// Frees all memory allocated on allocator `p`.
// ALLO_ERR_INVALID_NULL is returned if `p` is NULL.
static inline allo_status allo_pool_free_all(allo_pool *p);

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

  ALLO_ASSERT((p->end - p->start) / p->chunk_size,
              "chunk count must be non-zero");

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

static inline void allo_pool_freelist_reset_(allo_pool *p) {
  allo_pool_assert(p);

  size_t chunk_count = (p->end - p->start) / p->chunk_size;
  void **curr_chunk = (void **)p->start;
  for (size_t i = 0; i < chunk_count - 1; ++i) {
    void *next = (uint8_t *)curr_chunk + p->chunk_size;
    *curr_chunk = next;
    curr_chunk = next;
  }
  *curr_chunk = NULL;

  p->free_list = (void *)p->start;
  allo_pool_assert(p);
}

static inline allo_status allo_pool_init(allo_pool *restrict p,
                                         void *restrict buf, size_t buf_size,
                                         size_t chunk_size, size_t align) {
  if (!p || !buf) {
    return ALLO_ERR_INVALID_NULL;
  }
  if (!buf_size || !chunk_size) {
    return ALLO_ERR_INVALID_SIZE;
  }
  if (!align || align < sizeof(void *) || !allo_math_is_pow2(align)) {
    return ALLO_ERR_INVALID_ALIGNMENT;
  }
  if (chunk_size < sizeof(void *) || chunk_size % align != 0 ||
      chunk_size > buf_size) {
    return ALLO_ERR_INVALID_SIZE;
  }
  if (!allo_math_is_aligned((uintptr_t)buf, align)) {
    return ALLO_ERR_NOT_ALIGNED;
  }

  ALLO_ASSERT(align, "alignment must not be 0");
  ALLO_ASSERT(allo_math_is_pow2(align), "alignment must be a power of 2");
  ALLO_ASSERT(align >= sizeof(void *), "alignment must >= sizeof(void*)");
  ALLO_ASSERT(chunk_size >= sizeof(void *), "chunk size must <= sizeof(void*)");
  ALLO_ASSERT(chunk_size % align == 0,
              "chunk size must be a multiple of alignment");
  ALLO_ASSERT(
      chunk_size >= align,
      "chunk size must >= align to prevent padding between aligned chunks");
  ALLO_ASSERT(chunk_size <= buf_size,
              "chunk size must <= buf size to fit at least 1 chunk");

  size_t chunk_count = buf_size / chunk_size;

  p->chunk_size = chunk_size;
  p->align = align;
  p->start = (uintptr_t)buf;
  p->end = p->start + chunk_count * p->chunk_size;
  ALLO_ASSERT((uintptr_t)p->end <= (uintptr_t)buf + buf_size,
              "end must be within input memory region");
  allo_pool_freelist_reset_(p);

  allo_pool_assert(p);
  return ALLO_OK;
}

static inline size_t allo_pool_chunk_cap(const allo_pool *p) {
  allo_pool_assert(p);
  size_t count = (p->end - p->start) / p->chunk_size;
  ALLO_ASSERT(count > 0, "pool allocator must fit at least 1 chunk");
  return count;
}

static inline size_t allo_pool_free_chunks(const allo_pool *p) {
  allo_pool_assert(p);
  size_t chunks = 0;
  for (void **ptr = p->free_list; ptr; ptr = *ptr) {
    ++chunks;
  }
  return chunks;
}

static inline allo_status allo_pool_alloc(void *restrict *restrict dest,
                                          allo_pool *restrict p) {
#ifdef ALLO_SAFE_ALLOC
  {
    if (!dest || !p) {
      return ALLO_ERR_INVALID_NULL;
    }
  }
#endif
  ALLO_ASSERT(dest, "dest must not be NULL");
  ALLO_ASSERT(p, "pool allocator must not be NULL");
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

static inline allo_status allo_pool_free(allo_pool *restrict p,
                                         void *restrict ptr) {
#ifdef ALLO_SAFE_FREE
  {
    if (!p || !ptr) {
      return ALLO_ERR_INVALID_NULL;
    }
    uintptr_t mem = (uintptr_t)ptr;
    if (mem < p->start || mem >= p->end) {
      return ALLO_ERR_INVALID_ADDR;
    }
    if ((mem - p->start) % p->chunk_size != 0) {
      return ALLO_ERR_INVALID_ADDR;
    }
  }
#endif

  ALLO_ASSERT(ptr, "ptr must not be NULL");
  ALLO_ASSERT(p->start <= (uintptr_t)ptr,
              "ptr must be >= start of memory region");
  ALLO_ASSERT((uintptr_t)ptr < p->end, "ptr must be < end of memory region");
  ALLO_ASSERT(((uintptr_t)ptr - p->start) % p->chunk_size == 0,
              "region in [start..ptr] must be a multiple of chunk size");
  ALLO_ASSERT(allo_math_is_aligned((uintptr_t)ptr, p->align),
              "ptr must be aligned");
  allo_pool_assert(p);

  void **next_ptr = ptr;
  *next_ptr = p->free_list;
  p->free_list = ptr;

  allo_pool_assert(p);
  return ALLO_OK;
}

static inline allo_status allo_pool_free_all(allo_pool *p) {
  if (!p) {
    return ALLO_ERR_INVALID_NULL;
  }
  allo_pool_assert(p);
  allo_pool_freelist_reset_(p);
  allo_pool_assert(p);
  return ALLO_OK;
}

static inline allo_allocator allo_allocator_from_pool(allo_pool *p) {
  allo_pool_assert(p);
  return (allo_allocator){
      .allocator = p,
      .vtable = &allo_pool_vtable,
  };
}

#endif // !ALLO_POOL_H
