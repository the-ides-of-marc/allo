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
// enum allo_status:
//
//   This is an enum representing the status of an operation/function call.
//   This enum contains all the different possible success/error statuses across
//   this library.
//   All functions that can fail returns an `enum allo_status`.
//
//
// struct allo_allocator:
//
//   This is an interface for a memory allocator.
//   The primary use case is when there is a need to have represent an
//   allocator with no concrete implementation.
//
//   Examples:
//   - Data structures that take an allocator.
//   - Backing allocators that take in an underlying allocator.
//
//   Functions:
//
//     allo_alloc:
//
//       enum allo_status allo_alloc(
//         void **dest,
//         struct allo_allocator a,
//         size_t size,
//         size_t align);
//
//       Allocates `size` bytes at an alignment of `align`.
//       `dest` points to the allocated memory on success.
//
//     allo_free:
//
//       void allo_free(struct allo_allocator a, void* ptr);
//
//       frees the memory at `ptr`.

#include <stddef.h>
#include <stdint.h>

enum allo_status {
  ALLO_OK = 0,
  ALLO_OOM,
  ALLO_ERROR,
};

struct allo_allocator {
  void *ctx;
  enum allo_status (*alloc)(void **dest, void *ctx, size_t size, size_t align);
  void (*free)(void *ctx, void *ptr);
};

static inline enum allo_status allo_alloc(void **dest, struct allo_allocator a,
                                          size_t size, size_t align) {
  return a.alloc(dest, a.ctx, size, align);
}

static inline void allo_free(struct allo_allocator a, void *ptr) {
  a.free(a.ctx, ptr);
}

struct allo_fixed_bump {
  uintptr_t start;
  uintptr_t end;
  uintptr_t cursor;
};

enum allo_status allo_fixed_bump_init(struct allo_fixed_bump *b,
                                      void *restrict buf, size_t size);

enum allo_status allo_fixed_bump_alloc(void *restrict *restrict dest,
                                       struct allo_fixed_bump *restrict b,
                                       size_t size, size_t align);

void allo_fixed_bump_free(struct allo_fixed_bump *restrict b,
                          void *restrict ptr);

void allo_fixed_bump_reset(struct allo_fixed_bump *b);

#endif // !ALLO_H

#ifdef ALLO_IMPLEMENTATION

#include <assert.h>
#include <stdbool.h>

static inline bool allo__is_pow2(size_t n) {
  return n > 0 && (n & (n - 1)) == 0;
}

static inline void allo__assert_fixed_bump(struct allo_fixed_bump *b) {
  assert(b && "bump allocator must not be NULL");
  assert(b->start && "start of memory range must not be NULL");
  assert(b->end && "end of memory range must not be NULL");
  assert(b->start < b->end && "memory range property: start < end");
  assert(b->start <= b->cursor && "memory range property: start <= cursor");
  assert(b->cursor <= b->end && "memory range property: cursor <= end");
}

enum allo_status allo_fixed_bump_init(struct allo_fixed_bump *b,
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

void allo_fixed_bump_reset(struct allo_fixed_bump *b) { b->cursor = b->end; }

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
