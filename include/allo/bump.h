#ifndef ALLO_BUMP_H
#define ALLO_BUMP_H

#include "allocator.h"
#include "internal/defines.h"
#include "internal/math_common.h"
#include "status.h"
#include <stddef.h>
#include <stdint.h>

// Fixed size bump allocator.
struct allo_bump {
  uintptr_t start;
  uintptr_t end;
  uintptr_t cursor;
};

static inline void allo_assert_bump(struct allo_bump *b) {
  ALLO_ASSERT(b, "bump allocator must not be NULL");
  ALLO_ASSERT(b->start, "start of memory region must not be NULL");
  ALLO_ASSERT(b->end, "end of memory region must not be NULL");
  ALLO_ASSERT(b->start < b->end, "memory region property: start < end");
  ALLO_ASSERT(b->start <= b->cursor, "memory region property: start <= cursor");
  ALLO_ASSERT(b->cursor <= b->end, "memory region property: cursor <= end");
  (void)b;
}

// Initializes the bump allocator `b` to manage `buf` from
// [buf[0]..buf[size-1]]. The cursor is set to buf[size]. ALLO_ERR_NULL is
// returned if `b` or `buf` is NULL. ALLO_ERR_INVALID_SIZE is returned if `size`
// is 0.
enum allo_status allo_bump_init(struct allo_bump *ALLO_RESTRICT b,
                                void *ALLO_RESTRICT buf, size_t size);

// Tries to allocate `size` bytes at `align` alignment.
// `size` must be > 0 and `align` must be a power of 2.
// ALLO_OOM is returned if there is insufficient space to allocate the bytes.
static inline enum allo_status
allo_bump_alloc(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                struct allo_bump *ALLO_RESTRICT b, size_t size, size_t align) {
  ALLO_ASSERT(dest, "dest must not be NULL");
  allo_assert_bump(b);
  ALLO_ASSERT(size, "size to allocate must be non-zero");
  ALLO_ASSERT(align, "alignment must be non-zero");
  ALLO_ASSERT(allo_math_is_pow2(align), "alignment must be a power of 2");

  uintptr_t next_cursor = b->cursor - size;
  if (next_cursor > b->cursor) {
    return ALLO_OOM;
  }
  next_cursor = next_cursor & ~(align - 1);
  if (next_cursor < b->start) {
    return ALLO_OOM;
  }
  b->cursor = next_cursor;
  allo_assert_bump(b);

  *dest = (void *)next_cursor;
  return ALLO_OK;
}

// Sets the allocator's cursor to point to the given `cursor` address.
// ALLO_ERR_NULL is returned if `b` or `cursor` is NULL.
// ALLO_ERR_OUT_OF_BOUNDS is returned if `cursor` is outside of the allocator's
// memory region.
enum allo_status allo_bump_set_cursor(struct allo_bump *ALLO_RESTRICT b,
                                      const void *ALLO_RESTRICT cursor);

// Resets the allocator by setting the cursor back to its beginning position,
// buf[size].
// All memory held by the allocator prior to the reset should be considered
// invalid and unsafe to use.
void allo_bump_reset(struct allo_bump *b);

extern const struct allo_allocator_vtable allo_bump_vtable;

// Returns a generic allocator type from a bump allocator.
struct allo_allocator allo_allocator_from_bump(struct allo_bump *b);

#endif // !ALLO_BUMP_H
