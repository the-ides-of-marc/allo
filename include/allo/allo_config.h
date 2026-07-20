#ifndef ALLO_CONFIG_H
#define ALLO_CONFIG_H

// Safe alloc and free are enabled by default.
//
// If unsafe alloc or free is enabled, all validation performed on
// function arguments passed by the caller are omitted in alloc/free
// implementations. Invalid arguments can result in undefined behaviour.

#define ALLO_SAFE_ALLOC
#define ALLO_SAFE_FREE

#ifdef ALLO_ENABLE_UNSAFE_ALLOC
#undef ALLO_SAFE_ALLOC
#endif

#ifdef ALLO_ENABLE_UNSAFE_FREE
#undef ALLO_SAFE_FREE
#endif

#ifdef ALLO_ENABLE_ASSERT
#include <assert.h>
#define ALLO_ASSERT(invariant, message) assert((invariant) && (message))
#else
#define ALLO_ASSERT(invariant, message) ((void)0)
#endif

#endif // !ALLO_CONFIG_H
