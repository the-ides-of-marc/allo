#include "allo/alloc_req.h"
#include "allo/internal/math.h"
#include "allo/status.h"
#include <stddef.h>
#include <stdint.h>

allo_status allo_alloc_req_query_size_bump(size_t *size,
                                           const allo_alloc_req *alloc_reqs,
                                           size_t alloc_reqs_count) {
  if (!size || (!alloc_reqs && alloc_reqs_count)) {
    return ALLO_ERR_NULL;
  }
  if (!alloc_reqs_count) {
    return 0;
  }

  if (alloc_reqs[0].size == 0) {
    return ALLO_ERR_SIZE;
  }
  if (!allo_math_is_pow2(alloc_reqs[0].align)) {
    return ALLO_ERR_ALIGN;
  }

  // First allocation will have max padding to guarantee sufficent space to deal
  // with any amount of unaligned offset.

  size_t first_alloc_size = alloc_reqs[0].size + alloc_reqs[0].align - 1;

  // Move ptr_start to an aligned memory after the first allocation.
  // Subtract the entire first allocation size before aligning down so that the
  // final size computation does not overflow:
  // ptr_start - ptr_curr + first_alloc_size < UINTPTR_MAX,
  // where ptr_start <= UINTPTR_MAX - first_alloc_size.

  uintptr_t ptr_start = UINTPTR_MAX - first_alloc_size;
  while (ptr_start % alloc_reqs[0].align != 0) {
    --ptr_start;
  }
  uintptr_t ptr_curr = ptr_start;
  for (size_t i = 1; i < alloc_reqs_count; ++i) {
    if (alloc_reqs[i].size == 0) {
      return ALLO_ERR_SIZE;
    }
    if (!allo_math_is_pow2(alloc_reqs[i].align)) {
      return ALLO_ERR_ALIGN;
    }
    ptr_curr -= alloc_reqs[i].size;
    ptr_curr = allo_math_align_down(ptr_curr, alloc_reqs[i].align);
  }
  *size = ptr_start - ptr_curr + first_alloc_size;

  return ALLO_OK;
}
