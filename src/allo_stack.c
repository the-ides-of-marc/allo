#include "allo/allo_stack.h"
#include "allo/allo_status.h"

static allo_status allo_stack_alloc_adapter(void *restrict *restrict dest,
                                            void *restrict ctx, size_t size,
                                            size_t align) {
  return allo_stack_alloc(dest, (allo_stack *)ctx, size, align);
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

const allo_allocator_vtable allo_allocator_stack_vtable = {
    .alloc = allo_stack_alloc_adapter,
    .free = allo_stack_free_adapter,
    .free_all = allo_stack_free_all_adapter,
};
