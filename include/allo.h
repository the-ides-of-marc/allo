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
// NOTE:
//   Since this is a header-only library, internal types cannot be hidden in the
//   source files, thus all internal types start with the prefix
//   `allo__<identifier>` and should not be used.
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
//
//         allo_fixed_bump_free:
//
//           void allo_fixed_bump_free(struct allo_fixed_bump *b, void *ptr);
//
//           This is a noop as the bump allocator cannot free specific memory
//           previously allocated and can only be reset, clearing the allocator.
//           See `allo_fixed_bump_reset`.
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

#include <stddef.h>
#include <stdint.h>

enum allo_status {
  ALLO_OK = 0,
  ALLO_OOM,
  ALLO_ERROR,
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
  uintptr_t start;
  uintptr_t end;
  uintptr_t cursor;
};

enum allo_status allo_fixed_bump_init(struct allo_fixed_bump *restrict b,
                                      void *restrict buf, size_t size);

enum allo_status allo_fixed_bump_alloc(void *restrict *restrict dest,
                                       struct allo_fixed_bump *restrict b,
                                       size_t size, size_t align);

void allo_fixed_bump_free(struct allo_fixed_bump *restrict b,
                          void *restrict ptr);

void allo_fixed_bump_reset(struct allo_fixed_bump *b);

struct allo_allocator allo_allocator_from_fixed_bump(struct allo_fixed_bump *b);

#endif // !ALLO_H

#ifdef ALLO_IMPLEMENTATION

#include <assert.h>
#include <stdbool.h>

static inline bool allo__is_pow2(size_t n) {
  return n > 0 && (n & (n - 1)) == 0;
}

static enum allo_status allo_fixed_bump_alloc_adapter(void **dest, void *ctx,
                                                      size_t size,
                                                      size_t align) {
  return allo_fixed_bump_alloc(dest, (struct allo_fixed_bump *)ctx, size,
                               align);
}

static void allo__fixed_bump_free_adapter(void *ctx, void *ptr) {
  allo_fixed_bump_free((struct allo_fixed_bump *)ctx, ptr);
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
  assert(b->start && "start of memory region must not be NULL");
  assert(b->end && "end of memory region must not be NULL");
  assert(b->start < b->end && "memory region property: start < end");
  assert(b->start <= b->cursor && "memory region property: start <= cursor");
  assert(b->cursor <= b->end && "memory region property: cursor <= end");
  (void)b;
}

enum allo_status allo_fixed_bump_init(struct allo_fixed_bump *restrict b,
                                      void *restrict buf, size_t size) {
  assert(b && "bump allocator must not be NULL");
  assert(buf && "buffer for allocator must not be NULL");
  assert(size && "buffer size must be non-zero");

  b->start = (uintptr_t)buf;
  b->end = b->start + size;
  b->cursor = b->end;

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

  uintptr_t next_cursor = b->cursor - size;
  if (next_cursor > b->cursor) {
    return ALLO_OOM;
  }
  next_cursor = next_cursor & ~(align - 1);
  if (next_cursor < b->start) {
    return ALLO_OOM;
  }
  b->cursor = next_cursor;
  allo__assert_fixed_bump(b);

  *dest = (void *)next_cursor;
  return ALLO_OK;
}

void allo_fixed_bump_free(struct allo_fixed_bump *restrict b,
                          void *restrict ptr) {
  (void)b;
  (void)ptr;
}

void allo_fixed_bump_reset(struct allo_fixed_bump *b) {
  b->cursor = b->end;
  allo__assert_fixed_bump(b);
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
