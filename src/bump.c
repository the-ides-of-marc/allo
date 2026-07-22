#include "allo/bump.h"

allo_status allo_bump_init(allo_bump *restrict b, void *restrict buf,
                           size_t buf_size) {
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

allo_status allo_bump_set_cursor(allo_bump *restrict b,
                                 const void *restrict cursor) {
  if (!b || !cursor) {
    return ALLO_ERR_INVALID_NULL;
  }

  allo_bump_assert(b);

  uintptr_t c = (uintptr_t)cursor;
  if (c < b->start || c > b->end) {
    return ALLO_ERR_INVALID_ADDR;
  }
  b->cursor = c;

  allo_bump_assert(b);
  return ALLO_OK;
}

allo_status allo_bump_free_all(allo_bump *b) {
  if (!b) {
    return ALLO_ERR_INVALID_NULL;
  }
  allo_bump_assert(b);
  b->cursor = b->end;
  allo_bump_assert(b);
  return ALLO_OK;
}

allo_allocator allo_allocator_from_bump(allo_bump *b) {
  allo_bump_assert(b);
  return (allo_allocator){
      .allocator = b,
      .vtable = &allo_bump_vtable,
  };
}

static allo_status bump_alloc_unsafe_adapter(void *restrict *restrict dest,
                                             void *restrict ctx, size_t size,
                                             size_t align) {
  return allo_bump_alloc_unsafe(dest, (allo_bump *)ctx, size, align);
}

static allo_status bump_alloc_adapter(void *restrict *restrict dest,
                                      void *restrict ctx, size_t size,
                                      size_t align) {
  return allo_bump_alloc(dest, (allo_bump *)ctx, size, align);
}

static allo_status bump_free_unsafe_adapter(void *restrict ctx,
                                            void *restrict ptr) {
  (void)ctx;
  (void)ptr;
  return ALLO_ERR_INVALID_OP;
}

static allo_status bump_free_adapter(void *restrict ctx, void *restrict ptr) {
  (void)ctx;
  (void)ptr;
  return ALLO_ERR_INVALID_OP;
}

static allo_status bump_free_all_adapter(void *ctx) {
  allo_bump_free_all((allo_bump *)ctx);
  return ALLO_OK;
}

const allo_allocator_vtable allo_bump_vtable = {
    .alloc = bump_alloc_adapter,
    .alloc_unsafe = bump_alloc_unsafe_adapter,
    .free = bump_free_adapter,
    .free_unsafe = bump_free_unsafe_adapter,
    .free_all = bump_free_all_adapter,
};
