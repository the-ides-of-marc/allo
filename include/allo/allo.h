#ifndef ALLO_H
#define ALLO_H

// DOCUMENTATION
//
// This library implements different types of memory allocators, along with a
// generic allocator interface so that users can choose what allocator strategy
// to use via dynamic dispatch.
//
// Allocator implementations:
// - Allocator interface (see allo/allocator.h)
// - Fixed sized bump allocator (see allo/bump.h)
// - Fixed size pool allocator (see allo/pool.h)

// IWYU pragma: begin_exports
#include "allo_allocator.h"
#include "allo_bump.h"
#include "allo_pool.h"
#include "allo_status.h"
// IWYU pragma: end_exports

#endif // !ALLO_H
