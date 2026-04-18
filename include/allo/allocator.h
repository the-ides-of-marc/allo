#ifndef ALLO_ALLOCATOR_H
#define ALLO_ALLOCATOR_H

#include "internal/defines.h"
#include "status.h"
#include <stddef.h>

// The vtable used by allo_allocator struct.
struct allo_allocator_vtable {
  enum allo_status (*alloc)(void *ALLO_RESTRICT *ALLO_RESTRICT dest,
                            void *ALLO_RESTRICT ctx, size_t size, size_t align);
  void (*free)(void *ctx, void *ptr);
};

// Generic allocator type used when dynamic dispatch is needed.
struct allo_allocator {
  void *allocator;
  const struct allo_allocator_vtable *vtable;
};

static inline enum allo_status allo_alloc(void **dest, struct allo_allocator a,
                                          size_t size, size_t align) {
  return a.vtable->alloc(dest, a.allocator, size, align);
}

static inline void allo_free(struct allo_allocator a, void *ptr) {
  a.vtable->free(a.allocator, ptr);
}

#endif // !ALLO_ALLOCATOR_H
