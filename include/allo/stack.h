#ifndef ALLO_STACK_H
#define ALLO_STACK_H

#include "allo/config.h"
#include "allo/internal/math.h"
#include "allo/status.h"
#include <stddef.h>
#include <stdint.h>

// A fixed size stack allocator.
typedef struct allo_stack {
  // Start of managed memory region.
  uintptr_t start;
  // End of managed memory region.
  uintptr_t end;
  // Cursor pointing to the header of latest allocated memory,
  // which is followed by the allocated memory itself.
  // Points to `end` if allocator is empty.
  uintptr_t cursor;
} allo_stack;

// Asserts the state of the stack allocator.
static inline void allo_stack_assert(allo_stack *s) {
  ALLO_ASSERT(s, "stack allocator must not be NULL");
  ALLO_ASSERT(s->start < s->end, "start must be < end");
  ALLO_ASSERT(s->start <= s->cursor, "start must be <= cursor");
  ALLO_ASSERT(s->cursor <= s->end, "cursor must be <= end");
  (void)s;
}

// Initializes the stack allocator `s` to manage `buf` from
// `buf[0]..buf[buf_size-1]`.
// ALLO_ERR_NULL is returned if `s` or `buf` is NULL.
// ALLO_ERR_SIZE is returned if `buf_size` is 0.
allo_status allo_stack_init(allo_stack *restrict s, void *restrict buf,
                            size_t buf_size);

// Since C99 does not have alignof in its standard,
// Use sizeof(uintptr_t) for alignment.
// sizeof(uintptr_t) will never be smaller than alignof(uintptr_t),
// so header will always have sufficient space.
// The potential wasted space is bounded to sizeof(uintptr_t) -
// alignof(uintptr_t), which is typically 0.
#define ALLO_STACK_HEADER_ALIGN sizeof(uintptr_t)

// Tries to allocate `size` bytes at `align` alignment.
// `size` must be > 0 and `align` must be a power of 2.
// ALLO_OOM is returned if there is insufficient space to allocate the bytes.
//
// Arguments are not checked and invalid values can result in undefined
// behaviour.
static inline allo_status allo_stack_alloc_unsafe(void *restrict *restrict dest,
                                                  allo_stack *restrict s,
                                                  size_t size, size_t align) {
  ALLO_ASSERT(dest, "dest must not be NULL");
  ALLO_ASSERT(size, "size must not be 0");
  ALLO_ASSERT(align, "alignment must not be 0");
  ALLO_ASSERT(allo_math_is_pow2(align), "alignment must be a power of 2");
  allo_stack_assert(s);

  uintptr_t next_cursor = s->cursor - size;
  if (next_cursor > s->cursor) {
    return ALLO_OOM;
  }
  next_cursor = allo_math_align_down(next_cursor, align);
  if (next_cursor < s->start) {
    return ALLO_OOM;
  }
  *dest = (void *)next_cursor;

  next_cursor = allo_math_align_down(next_cursor - sizeof(uintptr_t),
                                     ALLO_STACK_HEADER_ALIGN);

  *(uintptr_t *)next_cursor = s->cursor;
  s->cursor = next_cursor;

  allo_stack_assert(s);
  return ALLO_OK;
}

// Tries to allocate `size` bytes at `align` alignment.
// `size` must be > 0 and `align` must be a power of 2.
// ALLO_ERR_NULL is returned if `dest` or `s` is NULL.
// ALLO_ERR_SIZE is returned if `size` is 0.
// ALLO_ERR_ALIGN is returned if `align` is not a power of 2.
// ALLO_OOM is returned if there is insufficient space to allocate the bytes.
static inline allo_status allo_stack_alloc(void *restrict *restrict dest,
                                           allo_stack *restrict s, size_t size,
                                           size_t align) {
  if (!dest || !s) {
    return ALLO_ERR_NULL;
  }
  if (!size) {
    return ALLO_ERR_SIZE;
  }
  if (!align || !allo_math_is_pow2(align)) {
    return ALLO_ERR_ALIGN;
  }
  return allo_stack_alloc_unsafe(dest, s, size, align);
}

// Frees the latest allocation.
//
// Arguments are not checked and invalid values can result in undefined
// behaviour.
static inline allo_status allo_stack_free_unsafe(allo_stack *s) {
  allo_stack_assert(s);

  if (s->cursor == s->end) {
    return ALLO_OK;
  }

  uintptr_t next_cursor = *(uintptr_t *)s->cursor;

  ALLO_ASSERT(s->start <= next_cursor, "start must be <= next cursor");
  ALLO_ASSERT(next_cursor <= s->end, "next cursor must be <= end");

  s->cursor = next_cursor;

  allo_stack_assert(s);
  return ALLO_OK;
}

// Frees the latest allocation.
// ALLO_ERR_NULL is returned if `s` is NULL.
static inline allo_status allo_stack_free(allo_stack *s) {
  if (!s) {
    return ALLO_ERR_NULL;
  }
  return allo_stack_free_unsafe(s);
}

// Frees all memory allocated on allocator `s`.
// ALLO_ERR_NULL is returned if `s` is NULL.
allo_status allo_stack_free_all(allo_stack *s);

#endif // !ALLO_STACK_H
