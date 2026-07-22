#ifndef ALLO_CONFIG_H
#define ALLO_CONFIG_H

#ifdef ALLO_ENABLE_ASSERT
#include <assert.h>
#define ALLO_ASSERT(invariant, message) assert((invariant) && (message))
#else
#define ALLO_ASSERT(invariant, message) ((void)0)
#endif

#endif // !ALLO_CONFIG_H
