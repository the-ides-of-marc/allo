#include "allo/allo.h"
#include "allo/allo_allocator.h"
#include "allo/allo_bump.h"
#include "allo/allo_status.h"
#include "allo/internal/allo_common.h"
#include "allo/internal/allo_math.h"
#include "allo_test/allo_test.h"
#include "allo_test_assert.h"
#include "allo_test_mem.h"
#include "unity.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unity_internals.h>

void setUp(void) {}

void tearDown(void) {}

enum {
  // Buffer size for all test cases.
  BUF_SIZE = 1 << 10
};

// Alignments for buffers and allocations that tests will use.
const size_t aligns[] = {1, 1 << 1, 1 << 2, 1 << 3, 1 << 4};

// Sizes for allocations that tests will use.
const size_t chunk_sizes[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

// Tests that allocator has the correct state on a successful init.
static void test_allo_bump_init_ok(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_bump b = {0};
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

// Tests when init takes in a null allocator.
static void test_allo_bump_init_null_allocator(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_status status = allo_bump_init(NULL, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status, "init must fail");
}

// Tests when init takes in a null buffer.
static void test_allo_bump_init_null_buf(void) {
  allo_bump b = {0};
  allo_status status = allo_bump_init(&b, NULL, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status, "init must fail");
}

// Tests when init takes in a zero sized buffer.
static void test_allo_bump_init_zero_buf_size(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_bump b = {0};
  allo_status status = allo_bump_init(&b, buf, 0);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_SIZE, status, "init must fail");
}

// Tests when alloc takes in a null destination.
static void test_allo_bump_alloc_null_dest(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      uint8_t buf[BUF_SIZE] = {0};
      allo_bump b = {0};
      allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
      allo_bump_assert(&b);
      ALLO_TEST_ASSERT_STATUS_MSG(
          ALLO_ERR_INVALID_NULL,
          allo_bump_alloc(NULL, &b, chunk_sizes[size_i], aligns[align_i]),
          "alloc must fail");
    }
  }
}

// Tests when alloc takes in a null allocator.
static void test_allo_bump_alloc_null_allocator(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      uint8_t buf[BUF_SIZE] = {0};
      allo_bump b = {0};
      allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
      allo_bump_assert(&b);

      void *dest = NULL;
      ALLO_TEST_ASSERT_STATUS_MSG(
          ALLO_ERR_INVALID_NULL,
          allo_bump_alloc(&dest, NULL, chunk_sizes[size_i], aligns[align_i]),
          "alloc must fail");
    }
  }
}

// Tests when alloc takes in a zero sized allocation.
static void test_allo_bump_alloc_zero_size(void) {
  for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
    uint8_t buf[BUF_SIZE] = {0};
    allo_bump b = {0};
    allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_bump_assert(&b);

    void *dest = NULL;
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_SIZE,
                                allo_bump_alloc(&dest, &b, 0, aligns[align_i]),
                                "alloc must fail");
  }
}

// Tests when alloc takes in an invalid alignment.
static void test_allo_bump_alloc_invalid_align(void) {
  size_t invalid_aligns[] = {0, 3, 5, 6, 7, 9, 10};
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(invalid_aligns);
         ++align_i) {
      uint8_t buf[BUF_SIZE] = {0};
      allo_bump b = {0};
      allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
      allo_bump_assert(&b);

      void *dest = NULL;
      ALLO_TEST_ASSERT_STATUS_MSG(
          ALLO_ERR_INVALID_ALIGNMENT,
          allo_bump_alloc(&dest, &b, chunk_sizes[size_i], invalid_aligns[align_i]),
          "alloc must fail");
    }
  }
}

// Tests the state of the allocator and allocated memory on a successful init on
// an empty allocator.
static void test_allo_bump_alloc_empty_allocator(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      uint8_t buf[BUF_SIZE] = {0};
      allo_bump b = {0};
      allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
      allo_bump_assert(&b);

      size_t old_cursor = b.cursor;
      void *dest = NULL;
      status = allo_bump_alloc(&dest, &b, chunk_sizes[size_i], aligns[align_i]);
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

// Tests the state of the allocator and allocated memory on a successful init on
// an non-empty allocator.
static void test_allo_bump_alloc_non_empty_allocator(void) {
  const size_t cursor_offsets[] = {1, 2, 4, 8, 16};

  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      for (const size_t *cursor_offset = cursor_offsets;
           cursor_offset < cursor_offsets + ALLO_ARR_LEN(cursor_offsets);
           ++cursor_offset) {
        uint8_t buf[BUF_SIZE] = {0};
        allo_bump b = {0};
        allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_bump_assert(&b);

        b.cursor -= *cursor_offset;
        allo_bump_assert(&b);

        uintptr_t next_expected_cursor =
            allo_math_align_down(b.cursor - chunk_sizes[size_i], aligns[align_i]);
        TEST_ASSERT_TRUE_MESSAGE(
            b.start <= next_expected_cursor && next_expected_cursor < b.end,
            "next_expected_cursor must be a valid memory address in allocator");

        void *dest = NULL;
        status = allo_bump_alloc(&dest, &b, chunk_sizes[size_i], aligns[align_i]);
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

// Tests when allocation is attempted on an allocator with insufficient free
// memory.
static void test_allo_bump_alloc_oom(void) {
  const size_t size = 8;
  const uintptr_t offsets[] = {0, 1, 2, 3, 4, 5, 6, 7};

  for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
    for (uintptr_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
      TEST_ASSERT_TRUE_MESSAGE(offsets[offset_i] < size,
                               "free space in allocator must be less than size "
                               "to allocate to cause OOM");
      uint8_t buf[BUF_SIZE] = {0};
      allo_bump b = {0};
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

// Tests when setting the cursor of the allocator is successful and valid.
static void test_allo_bump_set_cursor_ok(void) {
  size_t offsets[] = {0, BUF_SIZE / 2, BUF_SIZE};

  for (size_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
    uint8_t buf[BUF_SIZE] = {0};
    allo_bump b = {0};
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

// Tests when setting the cursor on a null allocator.
static void test_allo_bump_set_cursor_null_allocator(void) {
  size_t offsets[] = {0, BUF_SIZE / 2, BUF_SIZE};

  for (size_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
    uint8_t buf[BUF_SIZE] = {0};
    allo_bump b = {0};
    allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_bump_assert(&b);

    uintptr_t cursor = b.end - offsets[offset_i];

    status = allo_bump_set_cursor(NULL, (void *)cursor);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status,
                                "operation must fail");
  }
}

// Tests when setting the cursor with NULL.
static void test_allo_bump_set_cursor_null_cursor(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_bump b = {0};
  allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
  allo_bump_assert(&b);

  status = allo_bump_set_cursor(&b, NULL);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status,
                              "operation must fail");
}

// Tests when setting the cursor with an address that is outside the memory
// region managed by the allocator.
static void test_allo_bump_set_cursor_out_of_bounds(void) {
  size_t offsets[] = {BUF_SIZE * 2, BUF_SIZE + 1, -1u * (BUF_SIZE + 1),
                      -1u * (BUF_SIZE * 2)};

  for (size_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
    uint8_t buf[BUF_SIZE] = {0};
    allo_bump b = {0};
    allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_bump_assert(&b);

    uintptr_t cursor = b.end - offsets[offset_i];

    status = allo_bump_set_cursor(&b, (void *)cursor);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_OUT_OF_BOUNDS, status,
                                "operation must fail");
  }
}

// Tests free all takes in a null allocator.
static void test_allo_bump_free_all_null_allocator(void) {
  size_t offsets[] = {0, BUF_SIZE / 2, BUF_SIZE};

  for (size_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
    uint8_t buf[BUF_SIZE] = {0};
    allo_bump b = {0};
    allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_bump_assert(&b);

    b.cursor -= offsets[offset_i];
    allo_bump_assert(&b);

    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, allo_bump_free_all(NULL),
                                "free all must fail");
    allo_bump_assert(&b);
  }
}

// Tests when freeing all allocated memory in the allocator.
static void test_allo_bump_free_all_ok(void) {
  size_t offsets[] = {0, BUF_SIZE / 2, BUF_SIZE};

  for (size_t offset_i = 0; offset_i < ALLO_ARR_LEN(offsets); ++offset_i) {
    uint8_t buf[BUF_SIZE] = {0};
    allo_bump b = {0};
    allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_bump_assert(&b);

    b.cursor -= offsets[offset_i];
    allo_bump_assert(&b);

    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, allo_bump_free_all(&b),
                                "free all must succeed");

    allo_bump_assert(&b);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end, b.cursor,
                                  "cursor must be reset to the end");
  }
}

// Tests initializing and using the allocator on buffers aligned to different
// values.
static void test_allo_bump_buf_alignments(void) {
  for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
    void *buf = NULL;
    void *aligned_buf = ALLO_TEST_MEM_ALLOC(&buf, BUF_SIZE, aligns[align_i]);

    allo_bump b = {0};
    allo_status status = allo_bump_init(&b, aligned_buf, BUF_SIZE);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
    allo_bump_assert(&b);

    void *dest = NULL;
    for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
      for (size_t align_j = 0; align_j < ALLO_ARR_LEN(aligns); ++align_j) {
        status = allo_bump_alloc(&dest, &b, chunk_sizes[size_i], aligns[align_j]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        allo_bump_assert(&b);
      }
    }

    status = allo_bump_set_cursor(&b, (void *)b.start);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "set cursor must succeed");
    allo_bump_assert(&b);
    status = allo_bump_set_cursor(&b, (void *)b.end);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "set cursor must succeed");
    allo_bump_assert(&b);
    status = allo_bump_set_cursor(&b, (void *)((b.end + b.start) / 2));
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "set cursor must succeed");
    allo_bump_assert(&b);

    status = allo_bump_free_all(&b);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "free all must succeed");
    allo_bump_assert(&b);

    free(buf);
  }
}

// Tests creating an allocator interface from a concrete bump allocator.
static void test_allo_allocator_from_bump(void) {
  uint8_t buf[BUF_SIZE] = {0};
  allo_bump b = {0};
  allo_status status = allo_bump_init(&b, buf, BUF_SIZE);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
  allo_bump_assert(&b);

  allo_allocator a = allo_allocator_from_bump(&b);
  TEST_ASSERT_EQUAL_PTR_MESSAGE(&b, a.allocator,
                                "underlying allocator must match");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(&allo_bump_vtable, a.vtable,
                                "vtable must match");
  allo_bump_assert(a.allocator);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_allo_bump_init_ok);
  RUN_TEST(test_allo_bump_init_null_allocator);
  RUN_TEST(test_allo_bump_init_null_buf);
  RUN_TEST(test_allo_bump_init_zero_buf_size);

  RUN_TEST(test_allo_bump_alloc_null_dest);
  RUN_TEST(test_allo_bump_alloc_null_allocator);
  RUN_TEST(test_allo_bump_alloc_zero_size);
  RUN_TEST(test_allo_bump_alloc_invalid_align);
  RUN_TEST(test_allo_bump_alloc_empty_allocator);
  RUN_TEST(test_allo_bump_alloc_non_empty_allocator);
  RUN_TEST(test_allo_bump_alloc_oom);

  RUN_TEST(test_allo_bump_set_cursor_ok);
  RUN_TEST(test_allo_bump_set_cursor_null_allocator);
  RUN_TEST(test_allo_bump_set_cursor_null_cursor);
  RUN_TEST(test_allo_bump_set_cursor_out_of_bounds);

  RUN_TEST(test_allo_bump_free_all_null_allocator);
  RUN_TEST(test_allo_bump_free_all_ok);

  RUN_TEST(test_allo_bump_buf_alignments);

  RUN_TEST(test_allo_allocator_from_bump);

  return UNITY_END();
}
