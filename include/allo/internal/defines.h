#ifndef ALLO_DEFINES_H
#define ALLO_DEFINES_H

// Platform detection.

#if defined(_WIN32)
#define ALLO_PLATFORM_WINDOWS 1
#elif defined(__unix__) || defined(__APPLE__)
#define ALLO_PLATFORM_UNIX 1
#else
#error "platform not supported"
#endif

// Inlining

#if defined(_MSC_VER)
#define ALLO_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define ALLO_FORCE_INLINE inline __attribute__((always_inline))
#else
#define ALLO_FORCE_INLINE inline
#endif

// Restrict qualifier

#if defined(_MSC_VER)
#define ALLO_RESTRICT __restrict
#else
#define ALLO_RESTRICT restrict
#endif

// Asserts

#if defined(ALLO_ENABLE_ASSERT)
#include <assert.h>
#define ALLO_ASSERT(invariant, message) assert((invariant) && (message))
#else
#define ALLO_ASSERT(invariant, message) ((void)0)
#endif

#endif // !ALLO_DEFINES_H
