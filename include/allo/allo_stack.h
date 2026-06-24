#ifndef ALLO_STACK_H
#define ALLO_STACK_H

#include "allo/allo_config.h"
#include "allo/allo_status.h"
#include "allo/internal/allo_defines.h"
#include "allo/internal/allo_math.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

// A fixed size stack allocator.
typedef struct allo_stack allo_stack;

// Asserts the state of the stack allocator.
static inline void allo_stack_assert(allo_stack *s);

// Initializes the stack allocator `s` to manage `buf` from
// `buf[0]..buf[bufsize-1]`.
static inline allo_status allo_stack_init(allo_stack *ALLO_RESTRICT s,
                                          void *ALLO_RESTRICT buf,
                                          size_t bufsize);

// Tries to allocate `size` bytes at `align` alignment.
// `size` must be > 0 and `align` must be a power of 2.
// ALLO_OOM is returned if there is insufficient space to allocate the bytes.
static inline allo_status
allo_stack_alloc(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                 allo_stack *ALLO_RESTRICT s, size_t size, size_t align);

struct allo_stack {
  uintptr_t start;
  uintptr_t end;
  uintptr_t cursor;
};

static inline void allo_stack_assert(allo_stack *s) {
  ALLO_ASSERT(s, "stack allocator must not be NULL");
  ALLO_ASSERT(s->start < s->end, "start must be < end");
  ALLO_ASSERT(s->start <= s->cursor, "start must be < cursor");
  ALLO_ASSERT(s->cursor <= s->end, "cursor must be < end");
  (void)s;
}

static inline allo_status allo_stack_init(allo_stack *ALLO_RESTRICT s,
                                          void *ALLO_RESTRICT buf,
                                          size_t bufsize) {
  if (!s || !buf) {
    return ALLO_ERR_INVALID_NULL;
  }
  if (!bufsize) {
    return ALLO_ERR_INVALID_SIZE;
  }

  s->start = (uintptr_t)buf;
  s->end = s->start + bufsize;
  s->cursor = s->end;

  allo_stack_assert(s);
  return ALLO_OK;
}

static inline allo_status
allo_stack_alloc(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                 allo_stack *ALLO_RESTRICT s, size_t size, size_t align) {
  ALLO_ASSERT(dest, "dest must not be NULL");
  ALLO_ASSERT(size, "size mut not be 0");
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
  next_cursor =
      allo_math_align_down(next_cursor - sizeof(uintptr_t), alignof(uintptr_t));
  uintptr_t blksize = s->cursor - next_cursor;
  *(uintptr_t *)next_cursor = blksize;
  s->cursor = next_cursor;

  allo_stack_assert(s);
  return ALLO_OK;
}

#endif // !ALLO_STACK_H
