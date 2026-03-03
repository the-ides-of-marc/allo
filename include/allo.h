// SPDX-License-Identifier: MIT
//
// allo.h - header-only library for allocators.
//
// LICENSE
//     See the end of file for license information.
#ifndef ALLO_H
#define ALLO_H

// USAGE
//
// Add `#define ALLO_IMPLEMENTATION` in *ONE* C file.
//
// Example:
// ```
// #define ALLO_IMPLEMENTATION
// #include "allo.h"
// ```
//
// -----------------------------------------------------------------------------
//
// DOCUMENTATION
//
// OVERVIEW:
//
//   This library implements different types of memory allocators.
//   Each memory allocator implementation can be abstracted by a common type
//   `struct allo_allocator` which is implemented as a fat pointer holding the
//   allocator and its vtable.
//
//   Allocator implementations:
//   - `allo_fixed_bump`
//   - `allo_pool`
//
//   For optimal usage, use the specific memory allocator directly to avoid
//   vtable overhead. However, `struct allo_allocator` is useful for code that
//   relies on a allocator, but delegates the implementation downstream.
//
//   Examples:
//   - data structures that have an underlying allocator.
//     resizable array, hash table, etc.
//   - complex allocator strategies that rely on underlying alllocators.
//     arena, debug/logging, etc.
//
//   Each allocator implementations have unique properties and support
//   unique operations, which are supported via their related functions,
//   `allo_<impl-name>_<operation`.
//
//   Integration of these operations into the more general
//   `allo_allocator_<operation>` functions is NOT guaranteed and done so in a
//   best effort attempt. This decision is done so to balance
//   providing these capabilities through the general
//   `allo_allocator_<operation>` functions in a misleading manner.
//
// NOTE:
//   Since this is a header-only library, internal types and struct fields
//   cannot be hidden in the source files, thus all internal types start with
//   the prefix `allo__<identifier>` and should not be used.
//
//   All failable functions return an `enum allo_status` that represents the
//   possible errors encountered in this library.
//
//
// INTERFACE:
//
//   enum allo_status:
//
//     This is an enum representing the status of an operation/function call.
//     This enum contains all the different possible success/error statuses
//     across this library. All functions that can fail returns an `enum
//     allo_status`.
//
//
//   struct allo_allocator:
//
//     This is an interface for a memory allocator.
//     The primary use case is when there is a need to have represent an
//     allocator with no concrete implementation.
//
//     Examples:
//     - Data structures that take an allocator.
//     - Backing allocators that take in an underlying allocator.
//
//     Functions:
//
//       allo_alloc:
//
//         enum allo_status allo_alloc(
//           void **dest,
//           struct allo_allocator a,
//           size_t size,
//           size_t align);
//
//         Allocates `size` bytes at an alignment of `align`.
//         `dest` points to the allocated memory on success.
//
//       allo_free:
//
//         void allo_free(struct allo_allocator a, void* ptr);
//
//         frees the memory at `ptr`.
//
//       allo_allocator_from_fixed_bump:
//
//         struct allo_allocator allo_allocator_from_fixed_bump(
//           struct allo_fixed_bump *b);
//
//         Wraps an existing `struct allo_fixed_bump` allocator in the a `struct
//         allo_allocator`.
//
//    struct allo_fixed_bump:
//
//       This is a fixed bump allocator implementation.
//
//       This follows a bump-down implementation instead of a bump-up
//       implementation as it is claimed that there is 1 less conditional and
//       lower pressure on registers. While bump-down might cause slower
//       reallocation but since this is a fixed size bump allocator, this means
//       bump-down is preferrable to bump-up.
//       See https://fitzgen.com/2019/11/01/always-bump-downwards.html.
//
//       Functions:
//
//         allo_fixed_bump_init:
//
//           enum allo_status allo_fixed_bump_init(
//             struct allo_fixed_bump *restrict b,
//             void *restrict buf,
//             size_t size);
//
//           Initializes the allocator `b` to track the `buf` from
//           [buf[0]..buf[size]).
//           The cursor of the allocator will point to buf[size], 1 position
//           after the first allocatable location in `buf`.
//           ALLO_ERR_NULL is returned if `b` or `buf` is NULL.
//           ALLO_ERR_INVALID_SIZE is returned if `size` is 0.
//
//         allo_fixed_bump_alloc:
//
//           enum allo_status allo_fixed_bump_alloc(
//             void *restrict *restrict dest,
//             struct allo_fixed_bump *restrict b,
//             size_t size,
//             size_t align);
//
//           Tries to allocate `size` bytes at `align` alignment.
//           `size` MUST be > 0 and `align` MUST be a power of 2.
//           ALLO_OOM is returned if there is insufficient space to allocate the
//           bytes.
//
//         allo_fixed_bump_set_cursor:
//
//           enum allo_status allo_fixed_bump_set_cursor(
//             struct allo_fixed_bump *b,
//             void *cursor);
//
//           This sets the allocator's cursor to point to the given `cursor`
//           address.
//           ALLO_ERR_NULL is returned if `b` or `cursor` is NULL.
//           ALLO_ERR_OUT_OF_BOUNDS is returned if `cursor` is outside of the
//           allocator's memory region.
//
//         allo_fixed_bump_reset:
//
//           void allo_fixed_bump_reset(struct allo_fixed_bump *b);
//
//           This resets the whole allocated, clearing the allocator by
//           resetting its cursor to its beginning position (1 position to the
//           right of the first allocatable memory). All addresses held be
//           previously allocated memory should be considered invalid and unsafe
//           to use even though they are still in the allocator's usable memory
//           region.
//
//     struct allo_pool:
//
//       This is a pool allocator that manages fixed size chunks of memory via
//       an intrusive free list.
//
//       Functions:
//
//         allo_pool_init:
//
//           enum allo_status allo_pool_init(
//             struct allo_pool *restrict p,
//             void *restrict buf, size_t buf_size,
//             size_t chunk_size, size_t align);
//
//           Initializes the pool allocator with a given chunk size and
//           alignment. The pool will use the memory region strictly within
//           [buf[0]..buf[buf_size]] with the free list pointing to the first
//           chunk.
//           `align` is rounded to the nearest power of 2, and `chunk_size` will
//           be incremented to the nearest multiple of `align`.
//           Both `align` and `chunk_size` will be at least `sizeof(void *)` to
//           facilitate the intrusive free list.
//
//           ALLO_ERR_NULL is returned if `p` or `buf` is NULL.
//           ALLO_ERR_INVALID_SIZE is returned if `buf_size` or `chunk_size` is
//           0 or if the incremented `chunk_size` is larger than `buf_size`.
//           ALLO_ERR_INVALID_ALIGN is returned if `align` is 0.
//           ALLO_ERR_MEM_NOT_ALIGNED is returned if the `buf` is not aligned
//           with the rounded `align`.
//
//         allo_pool_alloc:
//
//           enum allo_status allo_pool_alloc(
//             void *restrict *restrict dest,
//             struct allo_pool *restrict p);
//
//           Allocates a new chunk of memory based on the allocator's chunk size
//           and writes it to `*dest`. The free list is updated to point to the
//           next free chunk of memory.
//
//
// TODO:
//
// - Update vtable to have a free_all/reset function, that all allocators should
//   be able to implement.
//
// - Add feature/test macro toggles to enable or disable checks on allocation.
//   Since allocation is subjected to being executed in a hot loop, these
//   conditional checks that introduce branching should have the flexibility of
//   being toggled via macros.
//
// - Add function that takes in allo_status and return a readable string.

#include <stddef.h>
#include <stdint.h>

enum allo_status {
  ALLO_OK = 0,
  ALLO_OOM,
  ALLO_ERR_NULL,
  ALLO_ERR_INVALID_SIZE,
  ALLO_ERR_INVALID_ALIGN,
  ALLO_ERR_MEM_NOT_ALIGNED,
  ALLO_ERR_OUT_OF_BOUNDS,
};

struct allo__allocator_vtable {
  enum allo_status (*alloc)(void **dest, void *ctx, size_t size, size_t align);
  void (*free)(void *ctx, void *ptr);
};

struct allo_allocator {
  void *allo__ptr;
  const struct allo__allocator_vtable *allo__vtable;
};

static inline enum allo_status allo_alloc(void **dest, struct allo_allocator a,
                                          size_t size, size_t align) {
  return a.allo__vtable->alloc(dest, a.allo__ptr, size, align);
}

static inline void allo_free(struct allo_allocator a, void *ptr) {
  a.allo__vtable->free(a.allo__ptr, ptr);
}

struct allo_fixed_bump {
  uintptr_t allo__start;
  uintptr_t allo__end;
  uintptr_t allo__cursor;
};

enum allo_status allo_fixed_bump_init(struct allo_fixed_bump *restrict b,
                                      void *restrict buf, size_t size);

enum allo_status allo_fixed_bump_alloc(void *restrict *restrict dest,
                                       struct allo_fixed_bump *restrict b,
                                       size_t size, size_t align);

enum allo_status allo_fixed_bump_set_cursor(struct allo_fixed_bump *b,
                                            const void *ptr);

void allo_fixed_bump_reset(struct allo_fixed_bump *b);

struct allo_allocator allo_allocator_from_fixed_bump(struct allo_fixed_bump *b);

struct allo_pool {
  void *allo__free_list;
  uintptr_t allo__start;
  uintptr_t allo__end;
  size_t allo__chunk_size;
  size_t allo__align;
};

enum allo_status allo_pool_init(struct allo_pool *restrict p,
                                void *restrict buf, size_t buf_size,
                                size_t chunk_size, size_t align);

enum allo_status allo_pool_alloc(void *restrict *restrict dest,
                                 struct allo_pool *restrict p);

void allo_pool_free(struct allo_pool *restrict p, void *restrict ptr);

#endif // !ALLO_H

#ifdef ALLO_IMPLEMENTATION

#include <assert.h>
#include <limits.h>
#include <stdbool.h>

static inline bool allo__is_pow2(size_t n) {
  return n > 0 && (n & (n - 1)) == 0;
}

static inline size_t allo__round_pow2(size_t n) {
  --n;
#if SIZE_MAX >= UINT8_MAX
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
#endif
#if SIZE_MAX >= UINT16_MAX
  n |= n >> 8;
#endif
#if SIZE_MAX >= UINT32_MAX
  n |= n >> 16;
#endif
#if SIZE_MAX >= UINT64_MAX
  n |= n >> 32;
#endif
  return ++n;
}

static enum allo_status allo_fixed_bump_alloc_adapter(void **dest, void *ctx,
                                                      size_t size,
                                                      size_t align) {
  return allo_fixed_bump_alloc(dest, (struct allo_fixed_bump *)ctx, size,
                               align);
}

static void allo__fixed_bump_free_adapter(void *ctx, void *ptr) {
  (void)ctx;
  (void)ptr;
}

static const struct allo__allocator_vtable allo__fixed_bump_vtable = {
    .alloc = allo_fixed_bump_alloc_adapter,
    .free = allo__fixed_bump_free_adapter,
};

struct allo_allocator
allo_allocator_from_fixed_bump(struct allo_fixed_bump *b) {
  return (struct allo_allocator){
      .allo__ptr = b,
      .allo__vtable = &allo__fixed_bump_vtable,
  };
}

static inline void allo__assert_fixed_bump(struct allo_fixed_bump *b) {
  assert(b && "bump allocator must not be NULL");
  assert(b->allo__start && "start of memory region must not be NULL");
  assert(b->allo__end && "end of memory region must not be NULL");
  assert(b->allo__start < b->allo__end &&
         "memory region property: start < end");
  assert(b->allo__start <= b->allo__cursor &&
         "memory region property: start <= cursor");
  assert(b->allo__cursor <= b->allo__end &&
         "memory region property: cursor <= end");
  (void)b;
}

enum allo_status allo_fixed_bump_init(struct allo_fixed_bump *restrict b,
                                      void *restrict buf, size_t size) {
  if (!b || !buf) {
    return ALLO_ERR_NULL;
  }
  if (!size) {
    return ALLO_ERR_INVALID_SIZE;
  }

  b->allo__start = (uintptr_t)buf;
  b->allo__end = b->allo__start + size;
  b->allo__cursor = b->allo__end;

  allo__assert_fixed_bump(b);
  return ALLO_OK;
}

enum allo_status allo_fixed_bump_alloc(void *restrict *restrict dest,
                                       struct allo_fixed_bump *restrict b,
                                       size_t size, size_t align) {
  assert(dest && "dest must not be NULL");
  allo__assert_fixed_bump(b);
  assert(size && "size to allocate must be non-zero");
  assert(align && "alignment must be non-zero");
  assert(allo__is_pow2(align) && "alignment must be a power of 2");

  uintptr_t next_cursor = b->allo__cursor - size;
  if (next_cursor > b->allo__cursor) {
    return ALLO_OOM;
  }
  next_cursor = next_cursor & ~(align - 1);
  if (next_cursor < b->allo__start) {
    return ALLO_OOM;
  }
  b->allo__cursor = next_cursor;
  allo__assert_fixed_bump(b);

  *dest = (void *)next_cursor;
  return ALLO_OK;
}

enum allo_status allo_fixed_bump_set_cursor(struct allo_fixed_bump *b,
                                            const void *cursor) {
  if (!b || !cursor) {
    return ALLO_ERR_NULL;
  }

  allo__assert_fixed_bump(b);

  uintptr_t c = (uintptr_t)cursor;
  if (c < b->allo__start || c > b->allo__end) {
    return ALLO_ERR_OUT_OF_BOUNDS;
  }
  b->allo__cursor = c;

  allo__assert_fixed_bump(b);
  return ALLO_OK;
}

void allo_fixed_bump_reset(struct allo_fixed_bump *b) {
  allo__assert_fixed_bump(b);
  b->allo__cursor = b->allo__end;
  allo__assert_fixed_bump(b);
}

static inline void allo__assert_pool(const struct allo_pool *p) {
  assert(p && "pool allocator must not be NULL");

  assert(p->allo__align && "pool allocator alignment must be non-zero");
  assert(p->allo__align >= sizeof(void *) &&
         "pool allocator alignment must be able to hold a pointer");
  assert(allo__is_pow2(p->allo__align) &&
         "pool allocator must be a power of 2");

  assert(p->allo__chunk_size && "chunk size must be non-zero");
  assert(p->allo__chunk_size <= p->allo__end - p->allo__start &&
         "chunk size must not be greater than the memory region");
  assert(p->allo__chunk_size >= sizeof(void *) &&
         "chunk size must be able to hold a pointer");
  assert(p->allo__chunk_size % p->allo__align == 0 &&
         "chunk size must be aligned");

  assert(p->allo__start && "start must not be NULL");
  assert(p->allo__end && "end must not be NULL");
  assert(p->allo__start < p->allo__end && "start must be lesser than end");

  assert(p->allo__start % p->allo__align == 0 && "start must be aligned");
  assert(p->allo__end % p->allo__align == 0 && "end must be aligned");

  if (p->allo__free_list) {
    assert(p->allo__start <= (uintptr_t)p->allo__free_list &&
           "free list must be >= start of memory region");
    assert((uintptr_t)p->allo__free_list < p->allo__end &&
           "free list must be < end of memory region");
    assert(p->allo__start <= (uintptr_t)p->allo__free_list &&
           (uintptr_t)p->allo__free_list < p->allo__end &&
           "free list must be within the allocator's memory region");
    assert(((uintptr_t)p->allo__free_list % p->allo__align == 0) &&
           "free list must be aligned");
  }

  (void)p;
}

enum allo_status allo_pool_init(struct allo_pool *restrict p,
                                void *restrict buf, size_t buf_size,
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

  align = allo__round_pow2(align);
  align = align >= sizeof(void *) ? align : sizeof(void *);
  chunk_size = chunk_size >= sizeof(void *) ? chunk_size : sizeof(void *);
  chunk_size = (chunk_size + align - 1) & ~(align - 1);

  assert(allo__is_pow2(align) && "alignment must be a power of 2");
  assert(align >= sizeof(void *) &&
         "alignment must be greater than sizeof(void*)");
  assert(chunk_size >= align &&
         "chunk size must be at least the size of the alignment");
  assert(chunk_size % align == 0 &&
         "chunk size must be a multiple of alignment");

  if (chunk_size > buf_size) {
    return ALLO_ERR_INVALID_SIZE;
  }

  size_t chunk_count = buf_size / chunk_size;
  assert(chunk_count > 0 && "chunk count must be non-zero");

  if ((uintptr_t)buf % align != 0) {
    return ALLO_ERR_MEM_NOT_ALIGNED;
  }

  p->allo__chunk_size = chunk_size;
  p->allo__align = align;
  p->allo__free_list = buf;
  p->allo__start = (uintptr_t)buf;
  p->allo__end = p->allo__start + chunk_count * p->allo__chunk_size;
  assert((uintptr_t)p->allo__end <= (uintptr_t)buf + buf_size &&
         "end must be within input memory region");

  void **curr_chunk = buf;
  for (size_t i = 0; i < chunk_count - 1; ++i) {
    void *next = (uint8_t *)curr_chunk + p->allo__chunk_size;
    *curr_chunk = next;
    curr_chunk = next;
  }
  *curr_chunk = NULL;

  allo__assert_pool(p);
  return ALLO_OK;
}

enum allo_status allo_pool_alloc(void *restrict *restrict dest,
                                 struct allo_pool *restrict p) {
  allo__assert_pool(p);

  *dest = NULL;
  void **addr = p->allo__free_list;
  if (!addr) {
    return ALLO_OOM;
  }

  *dest = addr;
  p->allo__free_list = *addr;

  allo__assert_pool(p);
  return ALLO_OK;
}

void allo_pool_free(struct allo_pool *restrict p, void *restrict ptr) {
  allo__assert_pool(p);
  assert(p->allo__start <= (uintptr_t)ptr &&
         "ptr must be >= start of memory region");
  assert((uintptr_t)ptr < p->allo__end && "ptr must be < end of memory region");
  assert(((uintptr_t)ptr - (uintptr_t)p->allo__start) % p->allo__chunk_size ==
             0 &&
         "ptr must be aligned to chunks");
  assert((uintptr_t)ptr % p->allo__align == 0 && "ptr must be aligned");

  void **next_ptr = ptr;
  *next_ptr = p->allo__free_list;
  p->allo__free_list = ptr;

  allo__assert_pool(p);
}

#endif

/*
Copyright (c) 2026 Marc Fong

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
