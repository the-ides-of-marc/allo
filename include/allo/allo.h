#ifndef ALLO_H
#define ALLO_H

// DOCUMENTATION
//
// This library implements different types of memory allocators, along with a
// generic allocator interface so that users can choose what allocator strategy
// to use via dynamic dispatch.
//
// Allocator implementations:
// - Allocator interface (see allo/allo_allocator.h)
// - Fixed sized bump allocator (see allo/allo_bump.h)
// - Fixed sized stack allocator (see allo/allo_stack.h)
// - Fixed sized pool allocator (see allo/allo_pool.h)
//
// TODO:
// - add status for unsupported allocator operations in vtables instead of
//   performing a no-op.
// - consider adding macro flags such as ALLO_ENABLE_SAFE_ALLOC to toggle
//   between using ALLO_ASSERT or returning allo_status

// IWYU pragma: begin_exports
#include "allo_allocator.h"
#include "allo_bump.h"
#include "allo_pool.h"
#include "allo_stack.h"
#include "allo_status.h"
// IWYU pragma: end_exports

#endif // !ALLO_H
