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

allo_allocator allo_allocator_from_stack(allo_stack *s) {
  allo_stack_assert(s);
  return (allo_allocator){
      .allocator = s,
      .vtable = &allo_stack_vtable,
  };
}

static allo_status
allo_stack_alloc_unsafe_adapter(void *restrict *restrict dest,
                                void *restrict ctx, size_t size, size_t align) {
  return allo_stack_alloc_unsafe(dest, (allo_stack *)ctx, size, align);
}

static allo_status allo_stack_alloc_adapter(void *restrict *restrict dest,
                                            void *restrict ctx, size_t size,
                                            size_t align) {
  return allo_stack_alloc(dest, (allo_stack *)ctx, size, align);
}

static allo_status allo_stack_free_unsafe_adapter(void *restrict ctx,
                                                  void *restrict ptr) {
  (void)ptr;
  allo_stack_free_unsafe((allo_stack *)ctx);
  return ALLO_OK;
}

static allo_status allo_stack_free_adapter(void *restrict ctx,
                                           void *restrict ptr) {
  (void)ptr;
  allo_stack_free((allo_stack *)ctx);
  return ALLO_OK;
}

static allo_status allo_stack_free_all_adapter(void *ctx) {
  allo_stack_free_all((allo_stack *)ctx);
  return ALLO_OK;
}

const allo_allocator_vtable allo_stack_vtable = {
    .alloc = allo_stack_alloc_adapter,
    .alloc_unsafe = allo_stack_alloc_unsafe_adapter,
    .free = allo_stack_free_adapter,
    .free_unsafe = allo_stack_free_unsafe_adapter,
    .free_all = allo_stack_free_all_adapter,
};
