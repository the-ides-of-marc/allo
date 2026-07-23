#include "allo/stack.h"
#include "allo/status.h"

allo_status allo_stack_init(allo_stack *restrict s, void *restrict buf,
                            size_t buf_size) {
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

allo_status allo_stack_free_all(allo_stack *s) {
  if (!s) {
    return ALLO_ERR_INVALID_NULL;
  }
  allo_stack_assert(s);
  s->cursor = s->end;
  allo_stack_assert(s);
  return ALLO_OK;
}
