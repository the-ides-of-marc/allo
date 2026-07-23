#include "allo/bump.h"

allo_status allo_bump_init(allo_bump *restrict b, void *restrict buf,
                           size_t buf_size) {
  if (!b || !buf) {
    return ALLO_ERR_NULL;
  }
  if (!buf_size) {
    return ALLO_ERR_SIZE;
  }

  b->start = (uintptr_t)buf;
  b->end = b->start + buf_size;
  b->cursor = b->end;

  allo_bump_assert(b);
  return ALLO_OK;
}

allo_status allo_bump_set_cursor(allo_bump *restrict b,
                                 const void *restrict cursor) {
  if (!b || !cursor) {
    return ALLO_ERR_NULL;
  }

  allo_bump_assert(b);

  uintptr_t c = (uintptr_t)cursor;
  if (c < b->start || c > b->end) {
    return ALLO_ERR_ADDR;
  }
  b->cursor = c;

  allo_bump_assert(b);
  return ALLO_OK;
}

allo_status allo_bump_free_all(allo_bump *b) {
  if (!b) {
    return ALLO_ERR_NULL;
  }
  allo_bump_assert(b);
  b->cursor = b->end;
  allo_bump_assert(b);
  return ALLO_OK;
}
