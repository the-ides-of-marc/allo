#ifndef ALLO_TEST_MEM_H
#define ALLO_TEST_MEM_H

#include <stddef.h>

// Performs malloc on `*dest` and returns an address within
// [`*dest`..`*dest`+`size`) that is aligned to `align`
// If malloc returns NULL, it is written to `*dest` and the return is also NULL.
void *allo_test_mem_alloc(void **dest, size_t size, size_t align, size_t line);
#define ALLO_TEST_MEM_ALLOC(dest, size, align)                                 \
  allo_test_mem_malloc_aligned((dest), (size), (align), __LINE__);

#endif // !ALLO_TEST_MEM_H
