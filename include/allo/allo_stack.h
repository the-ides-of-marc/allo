#ifndef ALLO_STACK_H
#define ALLO_STACK_H

#include "allo/allo_allocator.h"
#include "allo/allo_config.h"
#include "allo/allo_status.h"
#include "allo/internal/allo_math.h"
#include <stddef.h>
#include <stdint.h>

// A fixed size stack allocator.
typedef struct allo_stack allo_stack;

// Asserts the state of the stack allocator.
static inline void allo_stack_assert(allo_stack *s);

// Initializes the stack allocator `s` to manage `buf` from
// `buf[0]..buf[buf_size-1]`.
// ALLO_ERR_INVALID_NULL is returned if `s` or `buf` is NULL.
// ALLO_ERR_INVALID_SIZE is returned if `buf_size` is 0.
static inline allo_status allo_stack_init(allo_stack *restrict s,
                                          void *restrict buf, size_t buf_size);

// Tries to allocate `size` bytes at `align` alignment.
// `size` must be > 0 and `align` must be a power of 2.
// ALLO_ERR_INVALID_NULL is returned if `dest` or `s` is NULL.
// ALLO_ERR_INVALID_SIZE is returned if `size` is 0.
// ALLO_ERR_INVALID_ALIGNMENT is returned if `align` is not a power of 2.
// ALLO_OOM is returned if there is insufficient space to allocate the bytes.
static inline allo_status allo_stack_alloc(void *restrict *restrict dest,
                                           allo_stack *restrict s, size_t size,
                                           size_t align);

// Frees the latest allocation.
// ALLO_ERR_INVALID_NULL is returned if `s` is NULL.
static inline allo_status allo_stack_free(allo_stack *s);

// Frees all memory allocated on allocator `s`.
// ALLO_ERR_INVALID_NULL is returned if `s` is NULL.
static inline allo_status allo_stack_free_all(allo_stack *s);

// Returns a allocator type from a stack allocator.
static inline allo_allocator allo_allocator_from_stack(allo_stack *s);

// VTable for stack allocator.
extern const allo_allocator_vtable allo_stack_vtable;

struct allo_stack {
  uintptr_t start;
  uintptr_t end;
  uintptr_t cursor;
};

static inline void allo_stack_assert(allo_stack *s) {
  ALLO_ASSERT(s, "stack allocator must not be NULL");
  ALLO_ASSERT(s->start < s->end, "start must be < end");
  ALLO_ASSERT(s->start <= s->cursor, "start must be <= cursor");
  ALLO_ASSERT(s->cursor <= s->end, "cursor must be <= end");
  (void)s;
}

static inline allo_status allo_stack_init(allo_stack *restrict s,
                                          void *restrict buf, size_t buf_size) {
  if (!s || !buf) {
    return ALLO_ERR_INVALID_NULL;
  }
  if (!buf_size) {
    return ALLO_ERR_INVALID_SIZE;
  }

  s->start = (uintptr_t)buf;
  s->end = s->start + buf_size;
  s->cursor = s->end;

  allo_stack_assert(s);
  return ALLO_OK;
}

static inline allo_status allo_stack_alloc(void *restrict *restrict dest,
                                           allo_stack *restrict s, size_t size,
                                           size_t align) {
#ifdef ALLO_SAFE_ALLOC
  {
    if (!dest || !s) {
      return ALLO_ERR_INVALID_NULL;
    }
    if (!size) {
      return ALLO_ERR_INVALID_SIZE;
    }
    if (!align || !allo_math_is_pow2(align)) {
      return ALLO_ERR_INVALID_ALIGN;
    }
  }
#endif
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
                                     ALLO_MATH_ALIGNOF(uintptr_t));
  *(uintptr_t *)next_cursor = s->cursor;
  s->cursor = next_cursor;

  allo_stack_assert(s);
  return ALLO_OK;
}

static inline allo_status allo_stack_free(allo_stack *s) {
#ifdef ALLO_SAFE_FREE
  if (!s) {
    return ALLO_ERR_INVALID_NULL;
  }
#endif
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

static inline allo_status allo_stack_free_all(allo_stack *s) {
  if (!s) {
    return ALLO_ERR_INVALID_NULL;
  }
  allo_stack_assert(s);
  s->cursor = s->end;
  allo_stack_assert(s);
  return ALLO_OK;
}

static inline allo_allocator allo_allocator_from_stack(allo_stack *s) {
  allo_stack_assert(s);
  return (allo_allocator){
      .allocator = s,
      .vtable = &allo_stack_vtable,
  };
}

#endif // !ALLO_STACK_H
