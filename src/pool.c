#include "allo/pool.h"
#include "allo/status.h"

static allo_status pool_alloc_adapter(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                                      void *ALLO_RESTRICT ctx, size_t size,
                                      size_t align) {
  allo_pool *pool = (allo_pool * ALLO_RESTRICT) ctx;
  ALLO_ASSERT(pool->chunk_size == size,
              "size must match pool allocator's chunk size");
  ALLO_ASSERT(pool->align == align,
              "alignment must match pool allocator's alignment");
  (void)size;
  (void)align;
  return allo_pool_alloc(dest, pool);
}

static allo_status pool_free_adapter(void *ALLO_RESTRICT ctx,
                                     void *ALLO_RESTRICT ptr) {
  allo_pool_free((allo_pool * ALLO_RESTRICT) ctx, ptr);
  return ALLO_OK;
}

static allo_status pool_free_all_adapter(void *ctx) {
  allo_pool_free_all((allo_pool *)ctx);
  return ALLO_OK;
}

const allo_allocator_vtable allo_pool_vtable = {
    .alloc = pool_alloc_adapter,
    .free = pool_free_adapter,
    .free_all = pool_free_all_adapter,
};
