#include "allo/pool.h"
#include "allo/internal/math.h"

allo_status allo_pool_init(allo_pool *restrict p, void *restrict buf,
                           size_t buf_size, size_t chunk_size, size_t align) {
  if (!p || !buf) {
    return ALLO_ERR_INVALID_NULL;
  }
  if (!buf_size || !chunk_size) {
    return ALLO_ERR_INVALID_SIZE;
  }
  if (!align || align < sizeof(void *) || !allo_math_is_pow2(align)) {
    return ALLO_ERR_INVALID_ALIGN;
  }
  if (chunk_size < sizeof(void *) || chunk_size % align != 0 ||
      chunk_size > buf_size) {
    return ALLO_ERR_INVALID_SIZE;
  }
  if (!allo_math_is_aligned((uintptr_t)buf, align)) {
    return ALLO_ERR_INVALID_ALIGN;
  }

  ALLO_ASSERT(align, "alignment must not be 0");
  ALLO_ASSERT(allo_math_is_pow2(align), "alignment must be a power of 2");
  ALLO_ASSERT(align >= sizeof(void *), "alignment must >= sizeof(void*)");
  ALLO_ASSERT(chunk_size >= sizeof(void *), "chunk size must <= sizeof(void*)");
  ALLO_ASSERT(chunk_size % align == 0,
              "chunk size must be a multiple of alignment");
  ALLO_ASSERT(
      chunk_size >= align,
      "chunk size must >= align to prevent padding between aligned chunks");
  ALLO_ASSERT(chunk_size <= buf_size,
              "chunk size must <= buf size to fit at least 1 chunk");

  size_t chunk_count = buf_size / chunk_size;

  p->chunk_size = chunk_size;
  p->start = (uintptr_t)buf;
  p->end = p->start + chunk_count * p->chunk_size;
  ALLO_ASSERT((uintptr_t)p->end <= (uintptr_t)buf + buf_size,
              "end must be within input memory region");
  allo_pool_freelist_reset(p);

  allo_pool_assert(p);
  return ALLO_OK;
}

size_t allo_pool_chunk_cap(const allo_pool *p) {
  allo_pool_assert(p);
  size_t count = (p->end - p->start) / p->chunk_size;
  ALLO_ASSERT(count > 0, "pool allocator must fit at least 1 chunk");
  return count;
}

size_t allo_pool_free_chunks(const allo_pool *p) {
  allo_pool_assert(p);
  size_t chunks = 0;
  for (void **ptr = p->free_list; ptr; ptr = *ptr) {
    ++chunks;
  }
  return chunks;
}

void allo_pool_freelist_reset(allo_pool *p) {
  allo_pool_assert(p);

  size_t chunk_count = (p->end - p->start) / p->chunk_size;
  void **curr_chunk = (void **)p->start;
  for (size_t i = 0; i < chunk_count - 1; ++i) {
    void *next = (uint8_t *)curr_chunk + p->chunk_size;
    *curr_chunk = next;
    curr_chunk = next;
  }
  *curr_chunk = NULL;

  p->free_list = (void *)p->start;
  allo_pool_assert(p);
}

allo_status allo_pool_free_all(allo_pool *p) {
  if (!p) {
    return ALLO_ERR_INVALID_NULL;
  }
  allo_pool_assert(p);
  allo_pool_freelist_reset(p);
  allo_pool_assert(p);
  return ALLO_OK;
}

allo_allocator allo_allocator_from_pool(allo_pool *p) {
  allo_pool_assert(p);
  return (allo_allocator){
      .allocator = p,
      .vtable = &allo_pool_vtable,
  };
}

static allo_status pool_alloc_unsafe_adapter(void *restrict *restrict dest,
                                             void *restrict ctx, size_t size,
                                             size_t align) {
  allo_pool *pool = (allo_pool *restrict)ctx;
  (void)size;
  (void)align;
  return allo_pool_alloc_unsafe(dest, pool);
}

static allo_status pool_alloc_adapter(void *restrict *restrict dest,
                                      void *restrict ctx, size_t size,
                                      size_t align) {
  allo_pool *pool = (allo_pool *restrict)ctx;
  (void)size;
  (void)align;
  return allo_pool_alloc(dest, pool);
}

static allo_status pool_free_unsafe_adapter(void *restrict ctx,
                                            void *restrict ptr) {
  allo_pool_free_unsafe((allo_pool *restrict)ctx, ptr);
  return ALLO_OK;
}

static allo_status pool_free_adapter(void *restrict ctx, void *restrict ptr) {
  allo_pool_free((allo_pool *restrict)ctx, ptr);
  return ALLO_OK;
}

static allo_status pool_free_all_adapter(void *ctx) {
  allo_pool_free_all((allo_pool *)ctx);
  return ALLO_OK;
}

const allo_allocator_vtable allo_pool_vtable = {
    .alloc = pool_alloc_adapter,
    .alloc_unsafe = pool_alloc_unsafe_adapter,
    .free = pool_free_adapter,
    .free_unsafe = pool_free_unsafe_adapter,
    .free_all = pool_free_all_adapter,
};
