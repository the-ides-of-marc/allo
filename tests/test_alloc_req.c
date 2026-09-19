#include "allo/alloc_req.h"
#include "allo/bump.h"
#include "allo/internal/common.h"
#include "allo/stack.h"
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

// Tests when query size receives an empty alloc req input.
static void test_allo_alloc_req_query_size_no_allocs(void) {
  size_t query_size = 0;
  ALLO_TEST_ASSERT_STATUS(ALLO_OK,
                          allo_alloc_req_query_size_bump(&query_size, NULL, 0));
  TEST_ASSERT_EQUAL_size_t(0, query_size);
  ALLO_TEST_ASSERT_STATUS(
      ALLO_OK, allo_alloc_req_query_size_stack(&query_size, NULL, 0));
  TEST_ASSERT_EQUAL_size_t(0, query_size);
}

// Tests when query size receives a single alloc req input.
static void test_allo_alloc_req_query_size_single_alloc(void) {

  enum { MSG_LEN = 1 << 10 };
  char msg[MSG_LEN] = "";
  size_t query_size;

  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      size_t size = sizes[size_i];
      size_t align = aligns[align_i];
      allo_test_snprintf(msg, MSG_LEN, NULL, "size=%zu align=%zu", size, align);

      allo_alloc_req alloc_req = {.size = size, .align = align};

      ALLO_TEST_ASSERT_STATUS(
          ALLO_OK, allo_alloc_req_query_size_bump(&query_size, &alloc_req, 1));
      TEST_ASSERT_EQUAL_size_t_MESSAGE(size + align - 1, query_size, msg);

      ALLO_TEST_ASSERT_STATUS(
          ALLO_OK, allo_alloc_req_query_size_stack(&query_size, &alloc_req, 1));
      TEST_ASSERT_EQUAL_size_t_MESSAGE(size + align - 1 +
                                           ALLO_STACK_HEADER_WIDTH +
                                           ALLO_STACK_HEADER_ALIGN - 1,
                                       query_size, msg);
    }
  }
}

// Tests when query size receives a single alloc req input.
static void test_allo_alloc_req_query_size_multiple_alloc(void) {
  size_t query_size;
  void *buf;

  allo_alloc_req alloc_reqs[] = {
      {.size = 5, .align = 16}, {.size = 10, .align = 4},
      {.size = 8, .align = 2},  {.size = 7, .align = 1},
      {.size = 3, .align = 8},
  };
  ALLO_TEST_ASSERT_STATUS(
      ALLO_OK, allo_alloc_req_query_size_bump(&query_size, alloc_reqs,
                                              ALLO_ARR_LEN(alloc_reqs)));

  buf = malloc(query_size);
  TEST_ASSERT_NOT_NULL(buf);
  allo_bump bump;
  ALLO_TEST_ASSERT_STATUS(ALLO_OK, allo_bump_init(&bump, buf, query_size));
  for (size_t i = 0; i < ALLO_ARR_LEN(alloc_reqs); ++i) {
    void *dest;
    ALLO_TEST_ASSERT_STATUS(
        ALLO_OK,
        allo_bump_alloc(&dest, &bump, alloc_reqs[i].size, alloc_reqs[i].align));
  }
  free(buf);

  ALLO_TEST_ASSERT_STATUS(
      ALLO_OK, allo_alloc_req_query_size_stack(&query_size, alloc_reqs,
                                               ALLO_ARR_LEN(alloc_reqs)));
  buf = malloc(query_size);
  TEST_ASSERT_NOT_NULL(buf);
  allo_stack stack;
  ALLO_TEST_ASSERT_STATUS(ALLO_OK, allo_stack_init(&stack, buf, query_size));
  for (size_t i = 0; i < ALLO_ARR_LEN(alloc_reqs); ++i) {
    void *dest;
    ALLO_TEST_ASSERT_STATUS(ALLO_OK,
                            allo_stack_alloc(&dest, &stack, alloc_reqs[i].size,
                                             alloc_reqs[i].align));
  }
  free(buf);
}

// Tests when size pointer to store the result is NULL.
static void test_allo_alloc_req_query_size_null_size(void) {
  allo_alloc_req alloc_reqs[] = {
      {.size = 5, .align = 16}, {.size = 10, .align = 4},
      {.size = 8, .align = 2},  {.size = 7, .align = 1},
      {.size = 3, .align = 8},
  };
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_NULL, allo_alloc_req_query_size_bump(NULL, alloc_reqs,
                                                    ALLO_ARR_LEN(alloc_reqs)));
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_NULL, allo_alloc_req_query_size_stack(NULL, alloc_reqs,
                                                     ALLO_ARR_LEN(alloc_reqs)));
}

// Tests when alloc_reqs array is NULL when its count > zero.
static void
test_allo_alloc_req_query_size_null_alloc_reqs_non_zero_count(void) {
  allo_alloc_req alloc_reqs[] = {
      {.size = 5, .align = 16}, {.size = 10, .align = 4},
      {.size = 8, .align = 2},  {.size = 7, .align = 1},
      {.size = 3, .align = 8},
  };
  size_t query_size;
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_NULL, allo_alloc_req_query_size_bump(&query_size, NULL,
                                                    ALLO_ARR_LEN(alloc_reqs)));
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_NULL, allo_alloc_req_query_size_stack(&query_size, NULL,
                                                     ALLO_ARR_LEN(alloc_reqs)));
}

// Tests when alloc reqs contain a zero sized allocation.
static void test_allo_alloc_req_query_size_zero_sized_alloc_req(void) {
  allo_alloc_req alloc_reqs[] = {
      {.size = 5, .align = 16}, {.size = 10, .align = 4},
      {.size = 8, .align = 2},  {.size = 0, .align = 1}, // invalid size
      {.size = 3, .align = 8},
  };
  size_t query_size;
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_SIZE, allo_alloc_req_query_size_bump(&query_size, alloc_reqs,
                                                    ALLO_ARR_LEN(alloc_reqs)));
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_SIZE, allo_alloc_req_query_size_stack(&query_size, alloc_reqs,
                                                     ALLO_ARR_LEN(alloc_reqs)));
}

// Tests when alloc reqs contain an allocation with an invalid alignment.
static void test_allo_alloc_req_query_size_unaligned_alloc_req(void) {
  allo_alloc_req alloc_reqs[] = {
      {.size = 5, .align = 16}, {.size = 10, .align = 3}, // unaligned alloc
      {.size = 8, .align = 2},  {.size = 7, .align = 1},
      {.size = 3, .align = 8},
  };
  size_t query_size;
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_ALIGN, allo_alloc_req_query_size_bump(&query_size, alloc_reqs,
                                                     ALLO_ARR_LEN(alloc_reqs)));
  ALLO_TEST_ASSERT_STATUS(
      ALLO_ERR_ALIGN, allo_alloc_req_query_size_stack(
                          &query_size, alloc_reqs, ALLO_ARR_LEN(alloc_reqs)));
}

// Tests when alloc reqs size requirement exceeds possible limits.
static void test_allo_alloc_req_query_size_oom(void) {
  allo_alloc_req alloc_reqs[] = {
      {.size = UINTPTR_MAX, .align = 1},
      {.size = 1, .align = 1},
  };
  size_t query_size;
  ALLO_TEST_ASSERT_STATUS(
      ALLO_OOM, allo_alloc_req_query_size_bump(&query_size, alloc_reqs,
                                               ALLO_ARR_LEN(alloc_reqs)));
  ALLO_TEST_ASSERT_STATUS(
      ALLO_OOM, allo_alloc_req_query_size_stack(&query_size, alloc_reqs,
                                                ALLO_ARR_LEN(alloc_reqs)));
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_allo_alloc_req_query_size_no_allocs);
  RUN_TEST(test_allo_alloc_req_query_size_single_alloc);
  RUN_TEST(test_allo_alloc_req_query_size_multiple_alloc);
  RUN_TEST(test_allo_alloc_req_query_size_null_size);
  RUN_TEST(test_allo_alloc_req_query_size_null_alloc_reqs_non_zero_count);
  RUN_TEST(test_allo_alloc_req_query_size_zero_sized_alloc_req);
  RUN_TEST(test_allo_alloc_req_query_size_unaligned_alloc_req);
  RUN_TEST(test_allo_alloc_req_query_size_oom);

  return UNITY_END();
}
