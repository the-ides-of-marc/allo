#include "allo/bump.h"

enum allo_status allo_bump_init(struct allo_bump *ALLO_RESTRICT b,
                                void *ALLO_RESTRICT buf, size_t size) {
  if (!b || !buf) {
    return ALLO_ERR_NULL;
  }
  if (!size) {
    return ALLO_ERR_INVALID_SIZE;
  }

  b->start = (uintptr_t)buf;
  b->end = b->start + size;
  b->cursor = b->end;

  allo_assert_bump(b);
  return ALLO_OK;
}

enum allo_status allo_bump_set_cursor(struct allo_bump *ALLO_RESTRICT b,
                                      const void *ALLO_RESTRICT cursor) {
  if (!b || !cursor) {
    return ALLO_ERR_NULL;
  }

  allo_assert_bump(b);

  uintptr_t c = (uintptr_t)cursor;
  if (c < b->start || c > b->end) {
    return ALLO_ERR_OUT_OF_BOUNDS;
  }
  b->cursor = c;

  allo_assert_bump(b);
  return ALLO_OK;
}

void allo_bump_reset(struct allo_bump *b) {
  allo_assert_bump(b);
  b->cursor = b->end;
  allo_assert_bump(b);
}

static enum allo_status
bump_alloc_adapter(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                   void *ALLO_RESTRICT ctx, size_t size, size_t align) {
  return allo_bump_alloc(dest, (struct allo_bump *)ctx, size, align);
}

static void bump_free_adapter(void *ALLO_RESTRICT ctx,
                              void *ALLO_RESTRICT ptr) {
  (void)ctx;
  (void)ptr;
}

static void bump_free_all_adapter(void *ctx) {
  allo_bump_reset((struct allo_bump *)ctx);
}

const struct allo_allocator_vtable allo_bump_vtable = {
    .alloc = bump_alloc_adapter,
    .free = bump_free_adapter,
    .free_all = bump_free_all_adapter,
};

inline struct allo_allocator allo_allocator_from_bump(struct allo_bump *b) {
  return (struct allo_allocator){
      .allocator = b,
      .vtable = &allo_bump_vtable,
  };
}
