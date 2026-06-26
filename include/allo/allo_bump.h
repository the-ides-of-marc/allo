#ifndef ALLO_BUMP_H
#define ALLO_BUMP_H

#include "allo_allocator.h"
#include "allo_status.h"
#include "internal/allo_math.h"
#include <stddef.h>
#include <stdint.h>

// Fixed size bump allocator.
typedef struct allo_bump allo_bump;

// Asserts the state of a bump allocator.
static inline void allo_bump_assert(allo_bump *b);

// Initializes the bump allocator `b` to manage `buf` from
// [buf[0]..buf[size-1]]. The cursor is set to buf[size]. ALLO_ERR_NULL is
// returned if `b` or `buf` is NULL. ALLO_ERR_INVALID_SIZE is returned if `size`
// is 0.
static inline allo_status allo_bump_init(allo_bump *restrict b,
                                         void *restrict buf, size_t buf_size);

// Tries to allocate `size` bytes at `align` alignment.
// `size` must be > 0 and `align` must be a power of 2.
// ALLO_OOM is returned if there is insufficient space to allocate the bytes.
static inline allo_status allo_bump_alloc(void *restrict *restrict dest,
                                          allo_bump *restrict b, size_t size,
                                          size_t align);

// Sets the allocator's cursor to point to the given `cursor` address.
// ALLO_ERR_NULL is returned if `b` or `cursor` is NULL.
// ALLO_ERR_OUT_OF_BOUNDS is returned if `cursor` is outside of the allocator's
// memory region.
static inline allo_status allo_bump_set_cursor(allo_bump *restrict b,
                                               const void *restrict cursor);

// Resets the allocator by setting the cursor back to its beginning position,
// buf[size].
// All memory held by the allocator prior to the reset should be considered
// invalid and unsafe to use.
static inline void allo_bump_reset(allo_bump *b);

// Returns a allocator type from a bump allocator.
static inline allo_allocator allo_allocator_from_bump(allo_bump *b);

// VTable for bump allocator.
extern const allo_allocator_vtable allo_bump_vtable;

// Fixed size bump allocator.
struct allo_bump {
  uintptr_t start;
  uintptr_t end;
  uintptr_t cursor;
};

static inline void allo_bump_assert(allo_bump *b) {
  ALLO_ASSERT(b, "bump allocator must not be NULL");
  ALLO_ASSERT(b->start, "start of memory region must not be NULL");
  ALLO_ASSERT(b->end, "end of memory region must not be NULL");
  ALLO_ASSERT(b->start < b->end, "memory region property: start < end");
  ALLO_ASSERT(b->start <= b->cursor, "memory region property: start <= cursor");
  ALLO_ASSERT(b->cursor <= b->end, "memory region property: cursor <= end");
  (void)b;
}

static inline allo_status allo_bump_init(allo_bump *restrict b,
                                         void *restrict buf, size_t buf_size) {
  if (!b || !buf) {
    return ALLO_ERR_INVALID_NULL;
  }
  if (!buf_size) {
    return ALLO_ERR_INVALID_SIZE;
  }

  b->start = (uintptr_t)buf;
  b->end = b->start + buf_size;
  b->cursor = b->end;

  allo_bump_assert(b);
  return ALLO_OK;
}

static inline allo_status allo_bump_alloc(void *restrict *restrict dest,
                                          allo_bump *restrict b, size_t size,
                                          size_t align) {
  ALLO_ASSERT(dest, "dest must not be NULL");
  allo_bump_assert(b);
  ALLO_ASSERT(size, "size to allocate must be non-zero");
  ALLO_ASSERT(align, "alignment must be non-zero");
  ALLO_ASSERT(allo_math_is_pow2(align), "alignment must be a power of 2");

  uintptr_t next_cursor = b->cursor - size;
  if (next_cursor > b->cursor) {
    return ALLO_OOM;
  }
  next_cursor = allo_math_align_down(next_cursor, align);
  if (next_cursor < b->start) {
    return ALLO_OOM;
  }
  b->cursor = next_cursor;
  allo_bump_assert(b);

  *dest = (void *)next_cursor;
  return ALLO_OK;
}

static inline allo_status allo_bump_set_cursor(allo_bump *restrict b,
                                               const void *restrict cursor) {
  if (!b || !cursor) {
    return ALLO_ERR_INVALID_NULL;
  }

  allo_bump_assert(b);

  uintptr_t c = (uintptr_t)cursor;
  if (c < b->start || c > b->end) {
    return ALLO_ERR_OUT_OF_BOUNDS;
  }
  b->cursor = c;

  allo_bump_assert(b);
  return ALLO_OK;
}

static inline void allo_bump_reset(allo_bump *b) {
  allo_bump_assert(b);
  b->cursor = b->end;
  allo_bump_assert(b);
}

static inline allo_allocator allo_allocator_from_bump(allo_bump *b) {
  allo_bump_assert(b);
  return (allo_allocator){
      .allocator = b,
      .vtable = &allo_bump_vtable,
  };
}

#endif // !ALLO_BUMP_H
