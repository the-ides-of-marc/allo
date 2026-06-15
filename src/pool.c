#include "allo/pool.h"

enum allo_status allo_pool_init(struct allo_pool *ALLO_RESTRICT p,
                                void *ALLO_RESTRICT buf, size_t buf_size,
                                size_t chunk_size, size_t align) {
  if (!p || !buf) {
    return ALLO_ERR_NULL;
  }
  if (!buf_size || !chunk_size) {
    return ALLO_ERR_INVALID_SIZE;
  }
  if (!align) {
    return ALLO_ERR_INVALID_ALIGN;
  }

  align = allo_math_round_pow2(align);
  align = align >= sizeof(void *) ? align : sizeof(void *);
  chunk_size = chunk_size >= sizeof(void *) ? chunk_size : sizeof(void *);
  chunk_size = (chunk_size + align - 1) & ~(align - 1);

  ALLO_ASSERT(allo_math_is_pow2(align), "alignment must be a power of 2");
  ALLO_ASSERT(align >= sizeof(void *),
              "alignment must be greater than sizeof(void*)");
  ALLO_ASSERT(chunk_size >= align,
              "chunk size must be at least the size of the alignment");
  ALLO_ASSERT(chunk_size % align == 0,
              "chunk size must be a multiple of alignment");

  if (chunk_size > buf_size) {
    return ALLO_ERR_INVALID_SIZE;
  }

  size_t chunk_count = buf_size / chunk_size;
  ALLO_ASSERT(chunk_count > 0, "chunk count must be non-zero");

  if (!allo_math_is_ptr_aligned(buf, align)) {
    return ALLO_ERR_MEM_NOT_ALIGNED;
  }

  p->chunk_size = chunk_size;
  p->align = align;
  p->free_list = buf;
  p->start = (uintptr_t)buf;
  p->end = p->start + chunk_count * p->chunk_size;
  ALLO_ASSERT((uintptr_t)p->end <= (uintptr_t)buf + buf_size,
              "end must be within input memory region");

  void **curr_chunk = buf;
  for (size_t i = 0; i < chunk_count - 1; ++i) {
    void *next = (uint8_t *)curr_chunk + p->chunk_size;
    *curr_chunk = next;
    curr_chunk = next;
  }
  *curr_chunk = NULL;

  allo_assert_pool(p);
  return ALLO_OK;
}

static enum allo_status
pool_alloc_adapter(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                   void *ALLO_RESTRICT ctx, size_t size, size_t align) {
  struct allo_pool *pool = (struct allo_pool * ALLO_RESTRICT) ctx;
  ALLO_ASSERT(pool->chunk_size == size,
              "size must match pool allocator's chunk size");
  ALLO_ASSERT(pool->align == align,
              "alignment must match pool allocator's alignment");
  (void)size;
  (void)align;
  return allo_pool_alloc(dest, pool);
}

static void pool_free_adapter(void *ALLO_RESTRICT ctx,
                              void *ALLO_RESTRICT ptr) {
  allo_pool_free((struct allo_pool * ALLO_RESTRICT) ctx, ptr);
}

static void pool_free_all_adapter(void *ctx) {
  allo_pool_free_all((struct allo_pool *)ctx);
}

const struct allo_allocator_vtable allo_pool_vtable = {
    .alloc = pool_alloc_adapter,
    .free = pool_free_adapter,
    .free_all = pool_free_all_adapter,
};

inline struct allo_allocator allo_allocator_from_pool(struct allo_pool *p) {
  return (struct allo_allocator){
      .allocator = p,
      .vtable = &allo_pool_vtable,
  };
}
