#ifndef ALLO_H
#define ALLO_H

// DOCUMENTATION
//
// This library implements different types of memory allocators, along with a
// generic allocator interface so that users can choose what allocator strategy
// to use via dynamic dispatch.
//
// Allocator implementations:
// - Allocator interface         (allo/allocator.h)
// - Fixed sized bump allocator  (allo/bump.h)
// - Fixed sized stack allocator (allo/stack.h)
// - Fixed sized pool allocator  (allo/pool.h)

// IWYU pragma: begin_exports
#include "bump.h"
#include "config.h"
#include "pool.h"
#include "stack.h"
#include "status.h"
// IWYU pragma: end_exports

#endif // !ALLO_H
