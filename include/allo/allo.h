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
//
// TODO:
//
// - Update vtable to have a free_all/reset function, that all allocators should
//   be able to implement.

#include "allocator.h"
#include "bump.h"
#include "pool.h"
#include "status.h"

#endif // !ALLO_H
