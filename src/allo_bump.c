#include "allo/allo_bump.h"

static allo_status bump_alloc_adapter(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                                      void *ALLO_RESTRICT ctx, size_t size,
                                      size_t align) {
  return allo_bump_alloc(dest, (allo_bump *)ctx, size, align);
}

static allo_status bump_free_adapter(void *ALLO_RESTRICT ctx,
                                     void *ALLO_RESTRICT ptr) {
  (void)ctx;
  (void)ptr;
  return ALLO_OK;
}

static allo_status bump_free_all_adapter(void *ctx) {
  allo_bump_reset((allo_bump *)ctx);
  return ALLO_OK;
}

const allo_allocator_vtable allo_bump_vtable = {
    .alloc = bump_alloc_adapter,
    .free = bump_free_adapter,
    .free_all = bump_free_all_adapter,
};
