#ifndef ALLO_ALLOC_REQ_H
#define ALLO_ALLOC_REQ_H

#include "allo/config.h"
#include "allo/status.h"
#include <stddef.h>

// An allocation request to an allocator consisting of a size and alignment.
typedef struct allo_alloc_req {
  size_t size;
  size_t align;
} allo_alloc_req;

static inline void allo_alloc_req_assert(const allo_alloc_req *a) {
  ALLO_ASSERT(a->size, "allocation request must be non-zero");
  ALLO_ASSERT(allo_math_is_pow2(a->align), "alignment must be a power of 2");
  (void)a;
}

allo_status allo_alloc_req_bump_query_size(size_t *size,
                                           const allo_alloc_req *alloc_reqs,
                                           size_t alloc_reqs_count);

#endif // !ALLO_ALLOC_REQ_H
