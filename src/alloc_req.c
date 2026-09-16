#include "allo/alloc_req.h"
#include "allo/internal/math.h"
#include "allo/status.h"
#include <stddef.h>
#include <stdint.h>

allo_status allo_alloc_req_bump_query_size(size_t *size,
                                           const allo_alloc_req *alloc_reqs,
                                           size_t alloc_reqs_count) {
  if (!size || (!alloc_reqs && alloc_reqs_count)) {
    return ALLO_ERR_NULL;
  }
  uintptr_t ptr = UINTPTR_MAX;
  for (size_t i = 0; i < alloc_reqs_count; ++i) {
    if (alloc_reqs[i].size == 0) {
      return ALLO_ERR_SIZE;
    }
    if (allo_math_is_pow2(alloc_reqs[i].align)) {
      return ALLO_ERR_ALIGN;
    }
    ptr -= alloc_reqs[i].size;
    ptr = allo_math_align_down(ptr, alloc_reqs[i].align);
  }
  size_t total_size_required = UINTPTR_MAX - ptr;
  *size = total_size_required;
  return ALLO_OK;
}
