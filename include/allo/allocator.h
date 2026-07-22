#ifndef ALLO_ALLOCATOR_H
#define ALLO_ALLOCATOR_H

#include "allo/config.h"
#include "status.h"
#include <stddef.h>

// Allocator interface that provides common operations on allocator
// implementations.
// This is implemented with a fat pointer containing the allocator and its
// VTable.
typedef struct allo_allocator allo_allocator;

// VTable struct containing the operations supported by the allo_allocator
// interface.
// This struct contains function pointers for each operation.
typedef struct allo_allocator_vtable allo_allocator_vtable;

// Allocates `size` bytes at `align` alignment in allocator `a`, and writes the
// result to `dest`.
// Returns an allo_status indicating the status of the operation.
static inline allo_status allo_alloc(void **dest, allo_allocator a, size_t size,
                                     size_t align);

// Frees the memory at `ptr` in allocator `a`.
// Returns an allo_status indicating the status of the operation.
static inline allo_status allo_free(allo_allocator a, void *ptr);

// Frees all memory allocated in allocator `a`.
// Returns an allo_status indicating the status of the operation.
static inline allo_status allo_free_all(allo_allocator a);

struct allo_allocator_vtable {
  allo_status (*alloc)(void *restrict *restrict dest, void *restrict ctx,
                       size_t size, size_t align);
  allo_status (*alloc_unsafe)(void *restrict *restrict dest, void *restrict ctx,
                              size_t size, size_t align);
  allo_status (*free)(void *restrict ctx, void *restrict ptr);
  allo_status (*free_unsafe)(void *restrict ctx, void *restrict ptr);
  allo_status (*free_all)(void *ctx);
};

// Generic allocator type used when dynamic dispatch is needed.
struct allo_allocator {
  void *allocator;
  const allo_allocator_vtable *vtable;
};

static inline void allo_allocator_assert(allo_allocator a) {
  ALLO_ASSERT(a.allocator, "allocator must not be NULL");
  ALLO_ASSERT(a.vtable, "vtable must not be NULL");
  ALLO_ASSERT(a.vtable->alloc, "alloc must not be NULL");
  ALLO_ASSERT(a.vtable->alloc_unsafe, "alloc unsafe must not be NULL");
  ALLO_ASSERT(a.vtable->free, "free must not be NULL");
  ALLO_ASSERT(a.vtable->free_unsafe, "free unsafe must not be NULL");
  ALLO_ASSERT(a.vtable->free_all, "free all must not be NULL");
  (void)a;
}

static inline allo_status allo_alloc(void **dest, allo_allocator a, size_t size,
                                     size_t align) {
  allo_allocator_assert(a);
  return a.vtable->alloc(dest, a.allocator, size, align);
}

static inline allo_status allo_alloc_unsafe(void **dest, allo_allocator a,
                                            size_t size, size_t align) {
  allo_allocator_assert(a);
  return a.vtable->alloc_unsafe(dest, a.allocator, size, align);
}

static inline allo_status allo_free(allo_allocator a, void *ptr) {
  allo_allocator_assert(a);
  return a.vtable->free(a.allocator, ptr);
}

static inline allo_status allo_free_unsafe(allo_allocator a, void *ptr) {
  allo_allocator_assert(a);
  return a.vtable->free_unsafe(a.allocator, ptr);
}

static inline allo_status allo_free_all(allo_allocator a) {
  allo_allocator_assert(a);
  return a.vtable->free_all(a.allocator);
}

#endif // !ALLO_ALLOCATOR_H
