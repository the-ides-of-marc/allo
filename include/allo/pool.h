#ifndef ALLO_POOL_H
#define ALLO_POOL_H

#include "allo/allocator.h"
#include "internal/defines.h"
#include "internal/math_common.h"
#include "status.h"
#include <assert.h>
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
enum allo_status allo_pool_alloc(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                                 struct allo_pool *ALLO_RESTRICT p);

// Frees the memory allocated at `ptr`.
// The free list is then updated to point to `ptr`.
void allo_pool_free(struct allo_pool *ALLO_RESTRICT p, void *ALLO_RESTRICT ptr);

#ifdef ALLO_POOL_IMPLEMENTATION

static inline void allo_assert_pool(const struct allo_pool *p) {
  assert(p && "pool allocator must not be NULL");

  assert(p->align && "pool allocator alignment must be non-zero");
  assert(p->align >= sizeof(void *) &&
         "pool allocator alignment must be able to hold a pointer");
  assert(allo_is_pow2(p->align) && "pool allocator must be a power of 2");

  assert(p->chunk_size && "chunk size must be non-zero");
  assert(p->chunk_size <= p->end - p->start &&
         "chunk size must not be greater than the memory region");
  assert(p->chunk_size >= sizeof(void *) &&
         "chunk size must be able to hold a pointer");
  assert(p->chunk_size % p->align == 0 && "chunk size must be aligned");

  assert(p->start && "start must not be NULL");
  assert(p->end && "end must not be NULL");
  assert(p->start < p->end && "start must be lesser than end");

  assert(p->start % p->align == 0 && "start must be aligned");
  assert(p->end % p->align == 0 && "end must be aligned");

  if (p->free_list) {
    assert(p->start <= (uintptr_t)p->free_list &&
           "free list must be >= start of memory region");
    assert((uintptr_t)p->free_list < p->end &&
           "free list must be < end of memory region");
    assert(p->start <= (uintptr_t)p->free_list &&
           (uintptr_t)p->free_list < p->end &&
           "free list must be within the allocator's memory region");
    assert(((uintptr_t)p->free_list % p->align == 0) &&
           "free list must be aligned");
  }

  (void)p;
}

enum allo_status allo_pool_init(struct allo_pool *ALLO_RESTRICT p,
                                void *ALLO_RESTRICT buf, size_t buf_size,
                                size_t chunk_size, size_t align) {
  if (!p || !buf) {
    return ALLO_ERR_NULL;
  }
  if (!buf_size || !chunk_size) {
    return ALLO_ERR_INVALID_SIZE;
  }
  if (!align) {
    return ALLO_ERR_INVALID_ALIGN;
  }

  align = allo_round_pow2(align);
  align = align >= sizeof(void *) ? align : sizeof(void *);
  chunk_size = chunk_size >= sizeof(void *) ? chunk_size : sizeof(void *);
  chunk_size = (chunk_size + align - 1) & ~(align - 1);

  assert(allo_is_pow2(align) && "alignment must be a power of 2");
  assert(align >= sizeof(void *) &&
         "alignment must be greater than sizeof(void*)");
  assert(chunk_size >= align &&
         "chunk size must be at least the size of the alignment");
  assert(chunk_size % align == 0 &&
         "chunk size must be a multiple of alignment");

  if (chunk_size > buf_size) {
    return ALLO_ERR_INVALID_SIZE;
  }

  size_t chunk_count = buf_size / chunk_size;
  assert(chunk_count > 0 && "chunk count must be non-zero");

  if ((uintptr_t)buf % align != 0) {
    return ALLO_ERR_MEM_NOT_ALIGNED;
  }

  p->chunk_size = chunk_size;
  p->align = align;
  p->free_list = buf;
  p->start = (uintptr_t)buf;
  p->end = p->start + chunk_count * p->chunk_size;
  assert((uintptr_t)p->end <= (uintptr_t)buf + buf_size &&
         "end must be within input memory region");

  void **curr_chunk = buf;
  for (size_t i = 0; i < chunk_count - 1; ++i) {
    void *next = (uint8_t *)curr_chunk + p->chunk_size;
    *curr_chunk = next;
    curr_chunk = next;
  }
  *curr_chunk = NULL;

  allo_assert_pool(p);
  return ALLO_OK;
}

enum allo_status allo_pool_alloc(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
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

void allo_pool_free(struct allo_pool *ALLO_RESTRICT p,
                    void *ALLO_RESTRICT ptr) {
  allo_assert_pool(p);
  assert(p->start <= (uintptr_t)ptr && "ptr must be >= start of memory region");
  assert((uintptr_t)ptr < p->end && "ptr must be < end of memory region");
  assert(((uintptr_t)ptr - (uintptr_t)p->start) % p->chunk_size == 0 &&
         "ptr must be aligned to chunks");
  assert((uintptr_t)ptr % p->align == 0 && "ptr must be aligned");

  void **next_ptr = ptr;
  *next_ptr = p->free_list;
  p->free_list = ptr;

  allo_assert_pool(p);
}

static enum allo_status
pool_alloc_adapter(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                   void *ALLO_RESTRICT ctx, size_t size, size_t align) {
  struct allo_pool *pool = (struct allo_pool * ALLO_RESTRICT) ctx;
  assert(pool->chunk_size == size &&
         "size must match pool allocator's chunk size");
  assert(pool->align == align &&
         "alignment must match pool allocator's alignment");
  (void)size;
  (void)align;
  return allo_pool_alloc(dest, pool);
}

static void pool_free_adapter(void *ALLO_RESTRICT ctx,
                              void *ALLO_RESTRICT ptr) {
  allo_pool_free((struct allo_pool * ALLO_RESTRICT) ctx, ptr);
}

const struct allo_allocator_vtable allo_pool_vtable = {
    .alloc = pool_alloc_adapter,
    .free = pool_free_adapter,
};

inline struct allo_allocator allo_allocator_from_pool(struct allo_pool *p) {
  return (struct allo_allocator){
      .allocator = p,
      .vtable = &allo_pool_vtable,
  };
}

#endif

#endif // !ALLO_POOL_H
