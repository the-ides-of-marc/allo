#include "allo/allo_allocator.h"
#include "allo/allo_stack.h"
#include "allo/allo_status.h"
#include "allo/internal/allo_common.h"
#include "allo/internal/allo_math.h"
#include "allo_test/allo_test.h"
#include "allo_test_assert.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unity.h>
#include <unity_internals.h>

void setUp(void) {}

void tearDown(void) {}

enum {
  // Buffer size for all test cases.
  BUF_SIZE = 1 << 10
};

// Alignments for buffers and allocations that tests will use.
const size_t aligns[] = {sizeof(void *), sizeof(void *) << 1,
                         sizeof(void *) << 2};

// Sizes for pool chunks that tests will use.
const size_t sizes[] = {sizeof(void *), sizeof(void *) * 2, sizeof(void *) * 3,
                        sizeof(void *) * 4};

// Tests that allocator thas the correct state on a successful init.
static void test_allo_stack_init_ok(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_stack s = {0};
  allo_status status = allo_stack_init(&s, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
  allo_stack_assert(&s);

  TEST_ASSERT_EQUAL_PTR_MESSAGE(buf, s.start,
                                "start must = start of memory range");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(buf + BUF_SIZE, s.end,
                                "end must = end of memory range");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(s.end, s.cursor,
                                "cursor must = end of memory range");
}

// Tests when init takes in a null allocator.
static void test_allo_stack_init_null_allocator(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_status status = allo_stack_init(NULL, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status, "init must fail");
}

// Tests when init takes in a zero sized buffer.
static void test_allo_stack_init_zero_buf_size(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_stack s = {0};
  allo_status status = allo_stack_init(&s, buf, 0);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_SIZE, status, "init must fail");
}

// Tests the state of the allocator and allocated memory on a successful init on
// an empty allocator.
static void test_allo_stack_alloc_empty_allocator(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      uint8_t buf[BUF_SIZE] = {0};
      allo_stack s = {0};
      allo_status status = allo_stack_init(&s, buf, BUF_SIZE);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
      allo_stack_assert(&s);

      size_t old_cursor = s.cursor;
      void *dest = NULL;
      status = allo_stack_alloc(&dest, &s, sizes[size_i], aligns[align_i]);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");

      TEST_ASSERT_EQUAL_PTR_MESSAGE(
          dest, s.cursor + sizeof(uintptr_t),
          "dest must = cursor + header size storing the previous alloc");
      TEST_ASSERT_EQUAL_PTR_MESSAGE(
          s.end, *(uintptr_t *)s.cursor,
          "header must = end of memory region on alloc on empty allocator");

      TEST_ASSERT_TRUE_MESSAGE(old_cursor > s.cursor,
                               "cursor must have shifted and down");

      bool is_aligned = allo_math_is_aligned((uintptr_t)dest, aligns[align_i]);
      TEST_ASSERT_TRUE_MESSAGE(is_aligned, "dest must be aligned");
      allo_stack_assert(&s);
    }
  }
}

// Tests the state of the allocator and allocated memory on a successful init on
// an non-empty allocator.
static void test_allo_stack_alloc_non_empty_allocator(void) {
  const size_t cursor_offsets[] = {1, 2, 4, 8, 16};

  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      for (const size_t *cursor_offset = cursor_offsets;
           cursor_offset < cursor_offsets + ALLO_ARR_LEN(cursor_offsets);
           ++cursor_offset) {

        uint8_t buf[BUF_SIZE] = {0};
        allo_stack s = {0};
        allo_status status = allo_stack_init(&s, buf, BUF_SIZE);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_stack_assert(&s);

        s.cursor -= *cursor_offset;
        *(uintptr_t *)s.cursor = s.start;
        allo_stack_assert(&s);

        uintptr_t next_expected_cursor =
            allo_math_align_down(s.cursor - sizes[size_i], aligns[align_i]);
        TEST_ASSERT_TRUE_MESSAGE(
            s.start <= next_expected_cursor && next_expected_cursor < s.end,
            "next_expected_cursor must be a valid memory address in allocator");

        size_t old_cursor = s.cursor;
        void *dest = NULL;
        status = allo_stack_alloc(&dest, &s, sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        TEST_ASSERT_EQUAL_PTR_MESSAGE(
            dest, s.cursor + sizeof(uintptr_t),
            "dest must = cursor + header size storing the previous alloc");
        TEST_ASSERT_EQUAL_PTR_MESSAGE(
            old_cursor, *(uintptr_t *)s.cursor,
            "header must = end of memory region on previous cursor");
        TEST_ASSERT_TRUE_MESSAGE(old_cursor > s.cursor,
                                 "cursor must have shifted and down");
        bool is_aligned =
            allo_math_is_aligned((uintptr_t)dest, aligns[align_i]);
        TEST_ASSERT_TRUE_MESSAGE(is_aligned, "dest must be aligned");
        allo_stack_assert(&s);
      }
    }
  }
}

// Tests when allocation is attempted on an allocator with insufficient free
// memory.
static void test_allo_stack_alloc_oom(void) {
  const size_t size = 8;
  const uintptr_t offsets[] = {0, 1, 2, 3, 4, 5, 6, 7};
  for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
    for (uintptr_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
      TEST_ASSERT_TRUE_MESSAGE(offsets[offset_i] < size,
                               "free space in allocator must be less than size "
                               "to allocate to cause OOM");
      uint8_t buf[BUF_SIZE] = {0};
      allo_stack s = {0};
      allo_status status = allo_stack_init(&s, buf, BUF_SIZE);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
      allo_stack_assert(&s);

      s.cursor = s.start + offsets[offset_i];
      *(uintptr_t *)s.cursor = s.end;
      allo_stack_assert(&s);

      uintptr_t cursor_position = s.cursor;
      void *dest = NULL;
      status = allo_stack_alloc(&dest, &s, size, aligns[align_i]);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OOM, status, "alloc must fail (oom)");
      TEST_ASSERT_NULL_MESSAGE(dest, "dest must remain NULL");
      TEST_ASSERT_EQUAL_PTR_MESSAGE(cursor_position, s.cursor,
                                    "cursor must remain unchanged");
    }
  }
}

// Tests when freeing memory from the allocator.
static void test_allo_stack_free(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_stack s = {0};
  allo_status status = allo_stack_init(&s, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
  allo_stack_assert(&s);

  uintptr_t cursor_positions[ALLO_ARR_LEN(sizes) * ALLO_ARR_LEN(aligns)] = {0};
  size_t cursor_i = 0;

  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      cursor_positions[cursor_i++] = s.cursor;

      void *dest = NULL;
      status = allo_stack_alloc(&dest, &s, sizes[size_i], aligns[align_i]);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
      allo_stack_assert(&s);
    }
  }
  TEST_ASSERT_EQUAL_MESSAGE(ALLO_ARR_LEN(cursor_positions), cursor_i,
                            "cursor positions must all be tracked");

  for (size_t i = 0; i < ALLO_ARR_LEN(cursor_positions); ++i) {
    allo_stack_free(&s);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(
        cursor_positions[ALLO_ARR_LEN(cursor_positions) - i - 1], s.cursor,
        "free must set cursor back to position");
    allo_stack_assert(&s);
  }
}

// Tests when freeing all the memory of the allocator at once.
static void test_allo_stack_free_all(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_stack s = {0};
  allo_status status = allo_stack_init(&s, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
  allo_stack_assert(&s);

  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      void *dest = NULL;
      status = allo_stack_alloc(&dest, &s, sizes[size_i], aligns[align_i]);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
      allo_stack_assert(&s);
    }
  }

  allo_stack_free_all(&s);
  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      s.end, s.cursor,
      "free all must reset cursor back to end of memory region");
  allo_stack_assert(&s);
}

// Tests initializing and using the allocator on buffers aligned to different
// values.
static void test_allo_stack_buf_alignments(void) {
  for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
    void *buf = NULL;
    void *aligned_buf = ALLO_TEST_MEM_ALLOC(&buf, BUF_SIZE, aligns[align_i]);

    allo_stack s = {0};
    allo_status status = allo_stack_init(&s, aligned_buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_stack_assert(&s);

    void *dest = NULL;
    for (size_t size_i = 0; size_i < ALLO_ARR_LEN(sizes); ++size_i) {
      for (size_t align_j = 0; align_j < ALLO_ARR_LEN(aligns); ++align_j) {
        status = allo_stack_alloc(&dest, &s, sizes[size_i], aligns[align_j]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        allo_stack_assert(&s);
      }
    }

    allo_stack_free(&s);
    allo_stack_assert(&s);

    allo_stack_free_all(&s);
    allo_stack_assert(&s);

    free(buf);
  }
}

// Tests creating an allocator interface from a concrete stack allocator.
static void test_allo_allocator_from_stack(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_stack s = {0};
  allo_status status = allo_stack_init(&s, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
  allo_stack_assert(&s);

  allo_allocator a = allo_allocator_from_stack(&s);
  TEST_ASSERT_EQUAL_PTR_MESSAGE(&s, a.allocator,
                                "underlying allocator must match");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(&allo_stack_vtable, a.vtable,
                                "vtable must match");
  allo_stack_assert(a.allocator);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_allo_stack_init_ok);
  RUN_TEST(test_allo_stack_init_null_allocator);
  RUN_TEST(test_allo_stack_init_zero_buf_size);

  RUN_TEST(test_allo_stack_alloc_empty_allocator);
  RUN_TEST(test_allo_stack_alloc_non_empty_allocator);
  RUN_TEST(test_allo_stack_alloc_oom);

  RUN_TEST(test_allo_stack_free);

  RUN_TEST(test_allo_stack_free_all);

  RUN_TEST(test_allo_stack_buf_alignments);

  RUN_TEST(test_allo_allocator_from_stack);

  return UNITY_END();
}
