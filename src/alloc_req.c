#include "allo/alloc_req.h"
#include "allo/config.h"
#include "allo/internal/math.h"
#include "allo/stack.h"
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
    *size = 0;
    return ALLO_OK;
  }

  if (alloc_reqs[0].size == 0) {
    return ALLO_ERR_SIZE;
  }
  if (!allo_math_is_pow2(alloc_reqs[0].align)) {
    return ALLO_ERR_ALIGN;
  }

  // First allocation will have max padding to guarantee sufficent space to deal
  // with any amount of unaligned offset.

  size_t align_padding = alloc_reqs[0].align - 1;
  if (UINTPTR_MAX - align_padding < alloc_reqs[0].size) {
    return ALLO_OOM;
  }
  size_t first_alloc_size = alloc_reqs[0].size + align_padding;

  // Move ptr_start to an aligned memory after the first allocation.
  // Subtract the entire first allocation size before aligning down so that the
  // final size computation does not overflow:
  // ptr_start - ptr_curr + first_alloc_size < UINTPTR_MAX,
  // where ptr_start <= UINTPTR_MAX - first_alloc_size.

  uintptr_t ptr_start = UINTPTR_MAX - first_alloc_size;
  ptr_start = allo_math_align_down(ptr_start, alloc_reqs[0].align);

  uintptr_t ptr_curr = ptr_start;

  for (size_t i = 1; i < alloc_reqs_count; ++i) {
    if (alloc_reqs[i].size == 0) {
      return ALLO_ERR_SIZE;
    }
    if (!allo_math_is_pow2(alloc_reqs[i].align)) {
      return ALLO_ERR_ALIGN;
    }
    if (alloc_reqs[i].size > ptr_curr) {
      return ALLO_OOM;
    }
    ptr_curr -= alloc_reqs[i].size;
    ptr_curr = allo_math_align_down(ptr_curr, alloc_reqs[i].align);
  }
  *size = ptr_start - ptr_curr + first_alloc_size;

  return ALLO_OK;
}

allo_status allo_alloc_req_query_size_stack(size_t *size,
                                            const allo_alloc_req *alloc_reqs,
                                            size_t alloc_reqs_count) {
  if (!size || (!alloc_reqs && alloc_reqs_count)) {
    return ALLO_ERR_NULL;
  }
  if (!alloc_reqs_count) {
    *size = 0;
    return ALLO_OK;
  }

  if (alloc_reqs[0].size == 0) {
    return ALLO_ERR_SIZE;
  }
  if (!allo_math_is_pow2(alloc_reqs[0].align)) {
    return ALLO_ERR_ALIGN;
  }
  ALLO_ASSERT(allo_math_is_pow2(ALLO_STACK_HEADER_ALIGN),
              "header alignment must be a power of 2");

  // First allocation will have max padding to guarantee sufficent space to deal
  // with any amount of unaligned offset. Max padding is applied to both the
  // size allocated and the header.

  size_t align_padding = alloc_reqs[0].align - 1;
  if (UINTPTR_MAX - align_padding < alloc_reqs[0].size) {
    return ALLO_OOM;
  }
  size_t first_alloc_size = alloc_reqs[0].size + align_padding;

  // Move ptr_start to an aligned memory after the first allocation.
  // Subtract the entire first allocation size before aligning down so that the
  // final size computation does not overflow:
  // ptr_start - ptr_curr + first_alloc_size < UINTPTR_MAX,
  // where ptr_start <= UINTPTR_MAX - first_alloc_size.

  uintptr_t ptr_start = UINTPTR_MAX - first_alloc_size;
  ptr_start = allo_math_align_down(ptr_start, alloc_reqs[0].align);

  size_t header_padding = ALLO_STACK_HEADER_ALIGN - 1;
  if (UINTPTR_MAX - header_padding < ALLO_STACK_HEADER_WIDTH) {
    return ALLO_OOM;
  }
  size_t header_alloc_size = ALLO_STACK_HEADER_WIDTH + header_padding;
  if (header_alloc_size > ptr_start) {
    return ALLO_OOM;
  }
  ptr_start -= ALLO_STACK_HEADER_WIDTH;
  ptr_start = allo_math_align_down(ptr_start, ALLO_STACK_HEADER_ALIGN);

  uintptr_t ptr_curr = ptr_start;

  for (size_t i = 1; i < alloc_reqs_count; ++i) {
    if (alloc_reqs[i].size == 0) {
      return ALLO_ERR_SIZE;
    }
    if (!allo_math_is_pow2(alloc_reqs[i].align)) {
      return ALLO_ERR_ALIGN;
    }

    if (alloc_reqs[i].size > ptr_curr) {
      return ALLO_OOM;
    }
    ptr_curr -= alloc_reqs[i].size;
    ptr_curr = allo_math_align_down(ptr_curr, alloc_reqs[i].align);

    if (ALLO_STACK_HEADER_WIDTH > ptr_curr) {
      return ALLO_OOM;
    }
    ptr_curr -= ALLO_STACK_HEADER_WIDTH;
    ptr_curr = allo_math_align_down(ptr_curr, ALLO_STACK_HEADER_ALIGN);
  }
  *size = ptr_start - ptr_curr + first_alloc_size + header_alloc_size;

  return ALLO_OK;
}

allo_status allo_alloc_req_query_size_pool(size_t *size,
                                           allo_alloc_req alloc_req,
                                           size_t count) {
  if (!size) {
    return ALLO_ERR_NULL;
  }

  if (count == 0) {
    *size = 0;
    return ALLO_OK;
  }

  if (alloc_req.size == 0) {
    return ALLO_ERR_SIZE;
  }
  if (!allo_math_is_pow2(alloc_req.align)) {
    return ALLO_ERR_ALIGN;
  }
  size_t max_allocs = UINTPTR_MAX / alloc_req.size;
  if (max_allocs < count) {
    return ALLO_OOM;
  }

  return ALLO_OK;
}
