#ifndef ALLO_ALLOC_REQ_H
#define ALLO_ALLOC_REQ_H

#include "allo/config.h"
#include "allo/internal/math.h"
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

// Queries the size of the buffer (in bytes) required to support the following
// allocations, `alloc_reqs`, in the given order in a bump allocator, and writes
// the result to `size`.
//
// ALLO_ERR_NULL is returned if `size` is NULL or `alloc_reqs` is NULL
// when `alloc_reqs_count` > 0.
// ALLO_ERR_SIZE is returned if there are any zero
// sized allocations in `alloc_reqs`.
// ALLO_ERR_ALIGN is returned if there are
// any allocations with an alignment that is not a power of 2 in `alloc_reqs`.
// ALLO_OOM is returned if there is insufficient size that can fit all
// allocations in `alloc_reqs`.
allo_status allo_alloc_req_query_size_bump(size_t *size,
                                           const allo_alloc_req *alloc_reqs,
                                           size_t alloc_reqs_count);

// Queries the size of the buffer (in bytes) required to support the following
// allocations, `alloc_reqs`, in the given order in a stack allocator, and
// writes the result to `size`.
//
// ALLO_ERR_NULL is returned if `size` is NULL or `alloc_reqs` is NULL
// when `alloc_reqs_count` > 0.
// ALLO_ERR_SIZE is returned if there are any zero
// sized allocations in `alloc_reqs`.
// ALLO_ERR_ALIGN is returned if there are any allocations with an alignment
// that is not a power of 2 in `alloc_reqs`.
// ALLO_OOM is returned if there is
// insufficient size that can fit all allocations in `alloc_reqs`.
allo_status allo_alloc_req_query_size_stack(size_t *size,
                                            const allo_alloc_req *alloc_reqs,
                                            size_t alloc_reqs_count);

// Queries the size of the buffer (in bytes) required to support the following
// allocation, `alloc_req`, repeated `count` times in a pool allocator, and
// writes the result to `size`.
allo_status allo_alloc_req_query_size_pool(size_t *size,
                                           allo_alloc_req alloc_req,
                                           size_t count);

#endif // !ALLO_ALLOC_REQ_H
