#ifndef ALLO_STACK_H
#define ALLO_STACK_H

#include "allo/allo_config.h"
#include "allo/allo_status.h"
#include <stddef.h>
#include <stdint.h>

// A fixed size stack allocator.
typedef struct allo_stack allo_stack;

// Asserts the state of the stack allocator.
static inline void allo_stack_assert(allo_stack *s);

static inline allo_status allo_stack_init(allo_stack *s, void *buf,
                                          size_t bufsize);

struct allo_stack {
  uintptr_t start;
  uintptr_t end;
  uintptr_t cursor;
  size_t last_alloc_size;
};

static inline void allo_stack_assert(allo_stack *s) {
  ALLO_ASSERT(s, "stack allocator must not be NULL");
  ALLO_ASSERT(s->start < s->end, "start must be < end");
  ALLO_ASSERT(s->start <= s->cursor, "start must be < cursor");
  ALLO_ASSERT(s->cursor <= s->end, "cursor must be < end");
  ALLO_ASSERT(s->last_alloc_size < s->end - s->start,
              "last_alloc_size cannot exceed the size of the memory region");
  (void)s;
}

static inline allo_status allo_stack_init(allo_stack *s, void *buf,
                                          size_t bufsize) {
  if (!s || !buf) {
    return ALLO_ERR_INVALID_NULL;
  }
  if (!bufsize) {
    return ALLO_ERR_INVALID_SIZE;
  }

  s->start = (uintptr_t)buf;
  s->end = s->start + bufsize;
  s->cursor = s->start;
  s->last_alloc_size = 0;

  allo_stack_assert(s);
  return ALLO_OK;
}

#endif // !ALLO_STACK_H
