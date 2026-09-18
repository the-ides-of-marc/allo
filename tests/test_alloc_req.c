#include "allo/alloc_req.h"
#include "allo/internal/common.h"
#include "allo/status.h"
#include "allo_test/assert.h"
#include "allo_test/io.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unity.h>
#include <unity_internals.h>

// Alignments for buffers and allocations that tests will use.
const size_t aligns[] = {1, 1 << 1, 1 << 2, 1 << 3, 1 << 4};

// Sizes for allocations that tests will use.
const size_t sizes[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

void setUp(void) {}

void tearDown(void) {}

// Tests that the size is correct.
static void test_allo_alloc_req_query_size_bump_ok(void) {
  size_t query_size = 0;
  ALLO_TEST_ASSERT_STATUS(ALLO_OK,
                          allo_alloc_req_query_size_bump(&query_size, NULL, 0));

  enum { msg_len = 1 << 10 };
  char msg[msg_len] = "";

  // Test single allocations.
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      size_t size = sizes[size_i];
      size_t align = aligns[align_i];

      allo_test_snprintf(msg, msg_len, NULL, "size=%zu align=%zu", size, align);

      allo_alloc_req alloc_req = {.size = size, .align = align};
      ALLO_TEST_ASSERT_STATUS(
          ALLO_OK, allo_alloc_req_query_size_bump(&query_size, &alloc_req, 1));
      TEST_ASSERT_EQUAL_size_t_MESSAGE(size + align - 1, query_size, msg);
    }
  }

  // Test multiple allocations.
  allo_alloc_req alloc_reqs[] = {
      // format: - (size to allocate) - (adjustment for alignment)

      // - 5 - 15 (first alloc gets full padding)
      {.size = 5, .align = 16},
      // - 10 - 2
      {.size = 10, .align = 4},
      // - 8 - 0
      {.size = 8, .align = 2},
      // - 7 - 0
      {.size = 7, .align = 1},
      // - 3 - 2
      {.size = 3, .align = 8},

      // total expected size = 20 + 12 + 8 + 7 + 5 = 52
  };
  ALLO_TEST_ASSERT_STATUS(
      ALLO_OK, allo_alloc_req_query_size_bump(&query_size, alloc_reqs,
                                              ALLO_ARR_LEN(alloc_reqs)));
  TEST_ASSERT_EQUAL_size_t_MESSAGE(52, query_size, msg);
}

// Tests when size pointer to store the result is NULL.
static void test_allo_alloc_req_query_size_bump_null_size(void) {
  allo_alloc_req alloc_reqs[] = {
      {.size = 5, .align = 16}, {.size = 10, .align = 4},
      {.size = 8, .align = 2},  {.size = 7, .align = 1},
      {.size = 3, .align = 8},
  };
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_NULL, allo_alloc_req_query_size_bump(NULL, alloc_reqs,
                                                    ALLO_ARR_LEN(alloc_reqs)));
}

// Tests when alloc_reqs array is NULL when its count > zero.
static void
test_allo_alloc_req_query_size_bump_null_alloc_reqs_non_zero_count(void) {
  allo_alloc_req alloc_reqs[] = {
      {.size = 5, .align = 16}, {.size = 10, .align = 4},
      {.size = 8, .align = 2},  {.size = 7, .align = 1},
      {.size = 3, .align = 8},
  };
  size_t query_size;
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_NULL, allo_alloc_req_query_size_bump(&query_size, NULL,
                                                    ALLO_ARR_LEN(alloc_reqs)));
}

// Tests when alloc reqs contain a zero sized allocation.
static void test_allo_alloc_req_query_size_bump_zero_sized_alloc_req(void) {
  allo_alloc_req alloc_reqs[] = {
      {.size = 5, .align = 16}, {.size = 10, .align = 4},
      {.size = 8, .align = 2},  {.size = 0, .align = 1}, // invalid size
      {.size = 3, .align = 8},
  };
  size_t query_size;
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_SIZE, allo_alloc_req_query_size_bump(&query_size, alloc_reqs,
                                                    ALLO_ARR_LEN(alloc_reqs)));
}

// Tests when alloc reqs contain an allocation with an invalid alignment.
static void test_allo_alloc_req_query_size_bump_unaligned_alloc_req(void) {
  allo_alloc_req alloc_reqs[] = {
      {.size = 5, .align = 16}, {.size = 10, .align = 3}, // unaligned alloc
      {.size = 8, .align = 2},  {.size = 7, .align = 1},
      {.size = 3, .align = 8},
  };
  size_t query_size;
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_ALIGN, allo_alloc_req_query_size_bump(&query_size, alloc_reqs,
                                                     ALLO_ARR_LEN(alloc_reqs)));
}

static void test_allo_alloc_req_query_size_bump_oom(void) {
  allo_alloc_req alloc_reqs[] = {
      {.size = UINTPTR_MAX, .align = 1},
      {.size = 1, .align = 1},
  };
  size_t query_size;
  ALLO_TEST_ASSERT_STATUS(
      ALLO_OOM, allo_alloc_req_query_size_bump(&query_size, alloc_reqs,
                                               ALLO_ARR_LEN(alloc_reqs)));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_allo_alloc_req_query_size_bump_ok);
  RUN_TEST(test_allo_alloc_req_query_size_bump_null_size);
  RUN_TEST(test_allo_alloc_req_query_size_bump_null_alloc_reqs_non_zero_count);
  RUN_TEST(test_allo_alloc_req_query_size_bump_zero_sized_alloc_req);
  RUN_TEST(test_allo_alloc_req_query_size_bump_unaligned_alloc_req);
  RUN_TEST(test_allo_alloc_req_query_size_bump_oom);
  return UNITY_END();
}
