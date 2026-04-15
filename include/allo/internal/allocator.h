#ifndef ALLO_ALLOCATOR_H
#define ALLO_ALLOCATOR_H

#include "defines.h"
#include "status.h"
#include <stddef.h>

struct allo__allocator_vtable {
  enum allo_status (*alloc)(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                            void *ALLO_RESTRICT ctx, size_t size, size_t align);
  void (*free)(void *ctx, void *ptr);
};

struct allo_allocator {
  void *allo__ptr;
  const struct allo__allocator_vtable *allo__vtable;
};

static ALLO_FORCE_INLINE enum allo_status
allo_alloc(void **dest, struct allo_allocator a, size_t size, size_t align) {
  return a.allo__vtable->alloc(dest, a.allo__ptr, size, align);
}

static ALLO_FORCE_INLINE void allo_free(struct allo_allocator a, void *ptr) {
  a.allo__vtable->free(a.allo__ptr, ptr);
}

#endif // !ALLO_ALLOCATOR_H
