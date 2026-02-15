#ifndef ALLO_TEST_UTILS_H
#define ALLO_TEST_UTILS_H

#include <stddef.h>

// Performs malloc on `*dest` and returns an address within
// [`*dest`..`*dest`+`size`) that is aligned to `align`
// If malloc returns NULL, it is written to `*dest` and the return is also NULL.
void *malloc_aligned(void **dest, size_t size, size_t align);

#endif // !ALLO_TEST_UTILS_H
