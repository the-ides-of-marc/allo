#include "test_utils.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

void *malloc_aligned(void **dest, size_t size, size_t align) {
  *dest = malloc(size + align);
  if (!*dest) {
    return NULL;
  }

  uintptr_t addr = (uintptr_t)*dest;
  uintptr_t aligned_addr = (addr + align - 1) & ~(align - 1);
  return (void *)aligned_addr;
}
