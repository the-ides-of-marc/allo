#ifndef ALLO_CONFIG_H
#define ALLO_CONFIG_H

// Safe alloc and free are enabled by default.

#define ALLO_SAFE_ALLOC
#define ALLO_SAFE_FREE

// If unsafe alloc or free is enabled,
// asserts must be enabled.

#ifdef ALLO_ENABLE_UNSAFE_ALLOC
#undef ALLO_SAFE_ALLOC
#endif

#ifdef ALLO_ENABLE_UNSAFE_FREE
#undef ALLO_SAFE_FREE
#endif

#if defined(ALLO_ENABLE_UNSAFE_ALLOC) || defined(ALLO_ENABLE_UNSAFE_FREE)
#define ALLO_ENABLE_ASSERT
#endif

#ifdef ALLO_ENABLE_ASSERT
#include <assert.h>
#define ALLO_ASSERT(invariant, message) assert((invariant) && (message))
#else
#define ALLO_ASSERT(invariant, message) ((void)0)
#endif

#endif // !ALLO_CONFIG_H
