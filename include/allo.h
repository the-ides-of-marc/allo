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

#endif // !ALLO_H

#ifdef ALLO_IMPLEMENTATION
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
