#include "allo/allo.h"
#include "allo/allo_bump.h"
#include "allo/allo_status.h"
#include "allo/internal/allo_common.h"
#include "allo/internal/allo_math.h"
#include "allo_test/allo_test.h"
#include "allo_test_assert.h"
#include "unity.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

void setUp(void) {}

void tearDown(void) {}

enum { BUF_SIZE = 1 << 10 };

static void test_allo_bump_init_ok(void) {
  uint8_t buf[BUF_SIZE];
  allo_bump b;
  allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
  allo_bump_assert(&b);

  TEST_ASSERT_EQUAL_PTR_MESSAGE(buf, b.start,
                                "start must = start of memory range");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(buf + BUF_SIZE, b.end,
                                "end must = end of memory range");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end, b.cursor,
                                "cursor must = end of memory range");
}

static void test_allo_bump_init_null_allocator(void) {
  uint8_t buf[BUF_SIZE];
  allo_status status = allo_bump_init(NULL, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status, "init must fail");
}

static void test_allo_bump_init_null_buf(void) {
  allo_bump b;
  allo_status status = allo_bump_init(&b, NULL, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status, "init must fail");
}

static void test_allo_bump_init_zero_buf_size(void) {
  uint8_t buf[BUF_SIZE];
  allo_bump b;
  allo_status status = allo_bump_init(&b, buf, 0);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_SIZE, status, "init must fail");
}

static void test_allo_bump_alloc_empty_allocator(void) {
  const size_t sizes[] = {1, 2, 3, 4, 5, 6, 7, 8};
  const size_t aligns[] = {1, 2, 4, 8, 16};

  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      uint8_t buf[BUF_SIZE] = {0};
      allo_bump b;
      allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
      allo_bump_assert(&b);

      size_t old_cursor = b.cursor;
      void *dest = NULL;
      status = allo_bump_alloc(&dest, &b, sizes[size_i], aligns[align_i]);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
      TEST_ASSERT_EQUAL_PTR_MESSAGE(dest, b.cursor, "dest must = cursor");
      TEST_ASSERT_TRUE_MESSAGE(old_cursor > b.cursor,
                               "cursor must have shifted down");

      bool is_aligned = allo_math_is_aligned((uintptr_t)dest, aligns[align_i]);
      TEST_ASSERT_TRUE_MESSAGE(is_aligned, "dest must be aligned");
      allo_bump_assert(&b);
    }
  }
}

static void test_allo_bump_alloc_non_empty_allocator(void) {
  const size_t sizes[] = {1, 2, 3, 4, 5, 6, 7, 8};
  const size_t aligns[] = {1, 2, 4, 8, 16};
  const size_t cursor_offsets[] = {1, 2, 4, 8, 16};

  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      for (const size_t *cursor_offset = cursor_offsets;
           cursor_offset < cursor_offsets + ALLO_ARR_LEN(cursor_offsets);
           ++cursor_offset) {
        uint8_t buf[BUF_SIZE];
        allo_bump b;
        allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_bump_assert(&b);

        b.cursor -= *cursor_offset;
        allo_bump_assert(&b);

        uintptr_t next_expected_cursor =
            allo_math_align_down(b.cursor - sizes[size_i], aligns[align_i]);
        TEST_ASSERT_TRUE_MESSAGE(
            b.start <= next_expected_cursor && next_expected_cursor < b.end,
            "next_expected_cursor must be a valid memory address in allocator");

        void *dest = NULL;
        status = allo_bump_alloc(&dest, &b, sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        TEST_ASSERT_EQUAL_PTR_MESSAGE(next_expected_cursor, b.cursor,
                                      "cursor must be at expected position");
        TEST_ASSERT_EQUAL_PTR_MESSAGE(dest, b.cursor, "dest must = cursor");
        TEST_ASSERT_TRUE_MESSAGE(
            allo_math_is_aligned((uintptr_t)dest, aligns[align_i]),
            "dest must be aligned");
        allo_bump_assert(&b);
      }
    }
  }
}

static void test_allo_bump_alloc_oom(void) {
  const size_t size = 8;
  const size_t aligns[] = {1, 2, 4, 8, 16};
  const uintptr_t offsets[] = {0, 1, 2, 3, 4, 5, 6, 7};

  for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
    for (uintptr_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
      TEST_ASSERT_TRUE_MESSAGE(offsets[offset_i] < size,
                               "free space in allocator must be less than size "
                               "to allocate to cause OOM");
      uint8_t buf[BUF_SIZE];
      allo_bump b;
      allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
      allo_bump_assert(&b);

      b.cursor = b.start + offsets[offset_i];
      allo_bump_assert(&b);

      uintptr_t cursor_position = b.cursor;
      void *dest = NULL;
      status = allo_bump_alloc(&dest, &b, size, aligns[align_i]);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OOM, status, "alloc must fail (oom)");
      TEST_ASSERT_NULL_MESSAGE(dest, "dest must remain NULL");
      TEST_ASSERT_EQUAL_PTR_MESSAGE(cursor_position, b.cursor,
                                    "cursor must remain unchanged");
    }
  }
}

static void test_allo_bump_set_cursor_ok(void) {
  size_t offsets[] = {0, BUF_SIZE / 2, BUF_SIZE};

  for (size_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
    uint8_t buf[BUF_SIZE];
    allo_bump b;
    allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_bump_assert(&b);

    uintptr_t cursor = b.end - offsets[offset_i];

    status = allo_bump_set_cursor(&b, (void *)cursor);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "cursor update must succeed");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(cursor, b.cursor, "cursor must be updated");

    allo_bump_assert(&b);
  }
}

static void test_allo_bump_set_cursor_null_allocator(void) {
  size_t offsets[] = {0, BUF_SIZE / 2, BUF_SIZE};

  for (size_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
    uint8_t buf[BUF_SIZE];
    allo_bump b;
    allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_bump_assert(&b);

    uintptr_t cursor = b.end - offsets[offset_i];

    status = allo_bump_set_cursor(NULL, (void *)cursor);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status,
                                "operation must fail");
  }
}

static void test_allo_bump_set_cursor_null_cursor(void) {
  uint8_t buf[BUF_SIZE];
  allo_bump b;
  allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
  allo_bump_assert(&b);

  status = allo_bump_set_cursor(&b, NULL);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status,
                              "operation must fail");
}

static void test_allo_bump_set_cursor_out_of_bounds(void) {
  size_t offsets[] = {BUF_SIZE * 2, BUF_SIZE + 1, -1u * (BUF_SIZE + 1),
                      -1u * (BUF_SIZE * 2)};

  for (size_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
    uint8_t buf[BUF_SIZE];
    allo_bump b;
    allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_bump_assert(&b);

    uintptr_t cursor = b.end - offsets[offset_i];

    status = allo_bump_set_cursor(&b, (void *)cursor);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_OUT_OF_BOUNDS, status,
                                "operation must fail");
  }
}

void test_allo_bump_reset(void) {
  size_t offsets[] = {0, BUF_SIZE / 2, BUF_SIZE};

  for (size_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
    uint8_t buf[BUF_SIZE];
    allo_bump b;
    allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_bump_assert(&b);

    b.cursor -= offsets[offset_i];
    allo_bump_assert(&b);

    allo_bump_reset(&b);

    allo_bump_assert(&b);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end, b.cursor,
                                  "cursor must be reset to the end");
  }
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_allo_bump_init_ok);
  RUN_TEST(test_allo_bump_init_null_allocator);
  RUN_TEST(test_allo_bump_init_null_buf);
  RUN_TEST(test_allo_bump_init_zero_buf_size);

  RUN_TEST(test_allo_bump_alloc_empty_allocator);
  RUN_TEST(test_allo_bump_alloc_non_empty_allocator);
  RUN_TEST(test_allo_bump_alloc_oom);

  RUN_TEST(test_allo_bump_set_cursor_ok);
  RUN_TEST(test_allo_bump_set_cursor_null_allocator);
  RUN_TEST(test_allo_bump_set_cursor_null_cursor);
  RUN_TEST(test_allo_bump_set_cursor_out_of_bounds);

  RUN_TEST(test_allo_bump_reset);

  return UNITY_END();
}
