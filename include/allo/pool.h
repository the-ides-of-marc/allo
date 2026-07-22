#ifndef ALLO_POOL_H
#define ALLO_POOL_H

#include "allo/allocator.h"
#include "allo/config.h"
#include "allo/status.h"
#include <stddef.h>
#include <stdint.h>

// Fixed size pool allocator.
typedef struct allo_pool {
  // Implicit free list managing the addresses of available chunks.
  void *free_list;
  // Start of managed memory region.
  uintptr_t start;
  // End of managed memory region.
  uintptr_t end;
  // Chunk size.
  size_t chunk_size;
} allo_pool;

// Asserts the state of a pool allocator.
static inline void allo_pool_assert(const allo_pool *p) {
  ALLO_ASSERT(p, "pool allocator must not be NULL");

  ALLO_ASSERT(p->chunk_size, "chunk size must be non-zero");
  ALLO_ASSERT(p->chunk_size <= p->end - p->start,
              "chunk size must not be greater than the memory region");
  ALLO_ASSERT(p->chunk_size >= sizeof(void *),
              "chunk size must be able to hold a pointer");

  ALLO_ASSERT((p->end - p->start) / p->chunk_size,
              "chunk count must be non-zero");

  ALLO_ASSERT(p->start, "start must not be NULL");
  ALLO_ASSERT(p->end, "end must not be NULL");
  ALLO_ASSERT(p->start < p->end, "start must be lesser than end");

  if (p->free_list) {
    ALLO_ASSERT(p->start <= (uintptr_t)p->free_list,
                "free list must be >= start of memory region");
    ALLO_ASSERT((uintptr_t)p->free_list < p->end,
                "free list must be < end of memory region");
    ALLO_ASSERT(p->start <= (uintptr_t)p->free_list &&
                    (uintptr_t)p->free_list < p->end,
                "free list must be within the allocator's memory region");
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
// ALLO_ERR_INVALID_NULL is returned if `p` or `buf` is NULL.
// ALLO_ERR_INVALID_SIZE is returned if `buf_size` or `chunk_size` is 0 or if
// the padded `chunk_size` exceeds `buf_size`.
// ALLO_ERR_INVALID_ALIGN is returned if `align` is 0 or if the `buf` is not
// aligned with the rounded `align`.
allo_status allo_pool_init(allo_pool *restrict p, void *restrict buf,
                           size_t buf_size, size_t chunk_size, size_t align);

// Returns the maximum number of chunks this pool can allocate.
size_t allo_pool_chunk_cap(const allo_pool *p);

// Returns the number of free chunks remaining in this pool.
size_t allo_pool_free_chunks(const allo_pool *p);

// Resets the free list in the pool allocator.
void allo_pool_freelist_reset(allo_pool *p);

// Allocates a new chunk of memory of `p->chunk_size` and writes it to `*dest`.
// The free list is then updated to point to the next free chunk of memory.
// ALLO_OOM is returned if there is no more available chunk to allocate.
//
// Arguments are not checked and invalid values can result in undefined
// behaviour.
static inline allo_status allo_pool_alloc_unsafe(void *restrict *restrict dest,
                                                 allo_pool *restrict p) {
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

// Allocates a new chunk of memory of `p->chunk_size` and writes it to `*dest`.
// The free list is then updated to point to the next free chunk of memory.
// ALLO_ERR_INVALID_NULL is returned if `dest` or `p` is NULL.
// ALLO_OOM is returned if there is no more available chunk to allocate.
static inline allo_status allo_pool_alloc(void *restrict *restrict dest,
                                          allo_pool *restrict p) {
  if (!dest || !p) {
    return ALLO_ERR_INVALID_NULL;
  }
  return allo_pool_alloc_unsafe(dest, p);
}

// Frees the memory allocated at `ptr`.
// The free list is then updated to point to `ptr`.
// ALLO_ERR_INVALID_NULL is returned if `p` or `ptr` is NULL.
// ALLO_ERR_OUT_OF_BOUNDS is returned if `ptr` is outside of the allocator's
// memory range.
// ALLO_ERR_INVALID_ADDR is returned if `ptr` is not a valid chunk address.
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
  allo_pool_assert(p);

  void **next_ptr = ptr;
  *next_ptr = p->free_list;
  p->free_list = ptr;

  allo_pool_assert(p);
  return ALLO_OK;
}

// Frees all memory allocated on allocator `p`.
// ALLO_ERR_INVALID_NULL is returned if `p` is NULL.
allo_status allo_pool_free_all(allo_pool *p);

// Returns a allocator type from a pool allocator.
allo_allocator allo_allocator_from_pool(allo_pool *p);

// VTable for pool allocator.
extern const allo_allocator_vtable allo_pool_vtable;

#endif // !ALLO_POOL_H
