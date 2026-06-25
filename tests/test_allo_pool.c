#include "allo/allo.h"
#include "allo/internal/allo_common.h"
#include "allo/internal/allo_math.h"
#include "allo_test/allo_test.h"
#include "unity.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

void setUp(void) {}

void tearDown(void) {}

// Tests that the chunk_size is always at least sizeof(void*) and aligned.
void test_init_chunk_size_and_align(void) {
  for (size_t chunk_size = 1; chunk_size <= sizeof(void *) * 4; ++chunk_size) {
    const size_t alignments[] = {1, 2, 4, 8, 16};
    for (size_t i = 0; i < sizeof(alignments) / sizeof(alignments[0]); ++i) {
      size_t align = alignments[i];

      size_t expected_align = ALLO_MAX(align, sizeof(void *));

      size_t expected_chunk_size = ALLO_MAX(chunk_size, sizeof(void *));
      expected_chunk_size =
          allo_math_align_up(expected_chunk_size, expected_align);

      void *buf;
      size_t bufsize = sizeof(void *) * 64;
      void *buf_aligned =
          TEST_UTILS_MALLOC_ALIGNED(&buf, bufsize, expected_align);

      allo_pool p = {0};
      allo_status status =
          allo_pool_init(&p, buf_aligned, bufsize, chunk_size, align);

      ALLO_TEST_ASSERT_STATUS_MSG(
          ALLO_OK, status, "allocator initialization should succeed");
      ALLO_TEST_ASSERT_STATUS_MSG(
          ALLO_OK, status, "allocator initialization should succeed");

      TEST_ASSERT_EQUAL_size_t_MESSAGE(expected_chunk_size, p.chunk_size,
                                       "chunk size should match");
      TEST_ASSERT_EQUAL_size_t_MESSAGE(expected_align, p.align,
                                       "alignment should match");
      TEST_ASSERT_TRUE_MESSAGE(p.chunk_size % p.align == 0,
                               "chunk size must be aligned");

      TEST_ASSERT_EQUAL_PTR_MESSAGE(p.start, p.free_list,
                                    "free list should point to the start");

      allo_pool_assert(&p);

      free(buf);
    }
  }
}

// Tests that the memory region is properly aligned and fits chunks exactly.
void test_init_memory_region(void) {
  struct {
    size_t buf_size;
    size_t chunk_size;
    size_t align;
    size_t expected_end_offset;
  } tests[] = {
      // buffer size already a multiple of chunk size
      {1024, 128, 2, 1024},
      {1024, 256, 4, 1024},
      {1024, 512, 16, 1024},

      // buffer size not a multiple of chunk size
      {1025, 256, 32, 1024},
      {1111, 256, 64, 1024},
      {1279, 256, 128, 1024},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    void *buf;
    void *buf_aligned =
        TEST_UTILS_MALLOC_ALIGNED(&buf, tests[i].buf_size, tests[i].align);
    allo_pool p = {0};
    allo_status status = allo_pool_init(&p, buf_aligned, tests[i].buf_size,
                                        tests[i].chunk_size, tests[i].align);
    ALLO_TEST_ASSERT_STATUS_MSG(
        ALLO_OK, status, "allocator initialization should be succeed");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(buf_aligned, p.start,
                                  "allocator start should match");
    TEST_ASSERT_EQUAL_PTR_MESSAGE((uintptr_t)buf_aligned +
                                      tests[i].expected_end_offset,
                                  p.end, "allocator end should match");

    TEST_ASSERT_EQUAL_PTR_MESSAGE(p.start, p.free_list,
                                  "free list should point to the start");

    free(buf);
  }
}

void test_init_null_allocator(void) {
  uint8_t buf[0x100];
  allo_status status = allo_pool_init(NULL, buf, 0x100, 0x10, 0x1);
  ALLO_TEST_ASSERT_STATUS_MSG(
      ALLO_ERR_INVALID_NULL, status,
      "error should match for receiving a NULL allocator");
}

void test_init_null_buffer(void) {
  allo_pool p = {0};
  allo_status status = allo_pool_init(&p, NULL, 0x100, 0x10, 0x1);
  ALLO_TEST_ASSERT_STATUS_MSG(
      ALLO_ERR_INVALID_NULL, status,
      "error should match for receiving a NULL buffer");
}

void test_init_zero_buf_size(void) {
  uint8_t buf[0x100];
  allo_pool p = {0};
  allo_status status = allo_pool_init(&p, buf, 0, 0x10, 0x1);
  ALLO_TEST_ASSERT_STATUS_MSG(
      ALLO_ERR_INVALID_SIZE, status,
      "error should match for receiving a zero buf size");
}

void test_init_zero_chunk_size(void) {
  uint8_t buf[0x100];
  allo_pool p = {0};
  allo_status status = allo_pool_init(&p, buf, 0x100, 0x0, sizeof(void *));
  ALLO_TEST_ASSERT_STATUS_MSG(
      ALLO_ERR_INVALID_SIZE, status,
      "error should match for receiving a zero chunk size");
}

void test_init_zero_alignment(void) {
  uint8_t buf[0x100];
  allo_pool p = {0};
  allo_status status = allo_pool_init(&p, buf, 0x100, 0x10, 0x0);
  ALLO_TEST_ASSERT_STATUS_MSG(
      ALLO_ERR_INVALID_ALIGNMENT, status,
      "error should match for receiving zero alignment");
}

void test_init_memory_not_aligned(void) {
  size_t buf_size = 0x100;
  uint8_t buf[0x100];

  size_t align = sizeof(void *);

  uintptr_t aligned_buf = allo_math_align_up((uintptr_t)buf, align);
  uint8_t *unaligned_buf = (uint8_t *)(aligned_buf + 1);

  buf_size -= (size_t)((uintptr_t)unaligned_buf - (uintptr_t)buf);

  allo_pool p = {0};
  allo_status status = allo_pool_init(&p, unaligned_buf, buf_size, 0x10, align);
  ALLO_TEST_ASSERT_STATUS_MSG(
      ALLO_ERR_NOT_ALIGNED, status,
      "error should match for receiving a buffer that is not aligned");
}

void test_alloc_first_alloc(void) {
  struct {
    size_t buf_size;
    size_t chunk_size;
    size_t align;
  } tests[] = {
      {1024, 128, 2},
      {1024, 256, 4},
      {1024, 512, 8},
      {1024, 1024, 16},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    void *buf;
    void *buf_aligned =
        TEST_UTILS_MALLOC_ALIGNED(&buf, tests[i].buf_size, tests[i].align);

    allo_pool p = {0};
    allo_status status = allo_pool_init(&p, buf_aligned, tests[i].buf_size,
                                        tests[i].chunk_size, tests[i].align);
    ALLO_TEST_ASSERT_STATUS_MSG(
        ALLO_OK, status, "allocator initialization should be succeed");
    allo_pool_assert(&p);

    TEST_ASSERT_NOT_NULL_MESSAGE(p.free_list, "free list should not be NULL");
    void *next = *(void **)p.free_list;

    void *dest;
    status = allo_pool_alloc(&dest, &p);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status,
                                          "allocation should succeed");

    TEST_ASSERT_EQUAL_PTR_MESSAGE(p.start, dest,
                                  "allocated memory should match");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(
        dest, p.free_list,
        "free list should not point to the recently allocated memory");

    TEST_ASSERT_EQUAL_PTR_MESSAGE(
        next, p.free_list, "free list should be pointing to the next chunk");

    allo_pool_assert(&p);
    free(buf);
  }
}

void test_alloc_allocs_till_oom(void) {
  struct {
    size_t buf_size;
    size_t chunk_size;
    size_t align;
  } tests[] = {
      {1024, 128, 2},
      {1024, 256, 4},
      {1024, 512, 8},
      {1024, 1024, 16},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    void *buf;
    void *buf_aligned =
        TEST_UTILS_MALLOC_ALIGNED(&buf, tests[i].buf_size, tests[i].align);

    allo_pool p = {0};
    allo_status status = allo_pool_init(&p, buf_aligned, tests[i].buf_size,
                                        tests[i].chunk_size, tests[i].align);
    ALLO_TEST_ASSERT_STATUS_MSG(
        ALLO_OK, status, "allocator initialization should be succeed");
    allo_pool_assert(&p);

    size_t chunk_counts = (p.end - p.start) / p.chunk_size;
    void *dest;
    for (size_t chunk = 0; chunk < chunk_counts; ++chunk) {

      void *next = *(void **)p.free_list;
      status = allo_pool_alloc(&dest, &p);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status,
                                            "allocation should succeed");

      TEST_ASSERT_NOT_EQUAL_MESSAGE(
          dest, p.free_list,
          "free list should not point to the recently allocated memory");

      TEST_ASSERT_EQUAL_PTR_MESSAGE(
          next, p.free_list, "free list should be pointing to the next chunk");
    }

    TEST_ASSERT_NULL_MESSAGE(
        p.free_list, "free list should now point to NULL as allocator is full");

    status = allo_pool_alloc(&dest, &p);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OOM, status,
                                          "allocation should fail due to OOM");
    allo_pool_assert(&p);
    free(buf);
  }
}

void test_free_one(void) {
  struct {
    size_t buf_size;
    size_t chunk_size;
    size_t align;
    size_t free_index;
  } tests[] = {
      {1024, 256, 2, 0},
      {1024, 256, 4, 1},
      {1024, 256, 8, 3},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    void *buf;
    void *buf_aligned =
        TEST_UTILS_MALLOC_ALIGNED(&buf, tests[i].buf_size, tests[i].align);

    allo_pool p = {0};
    allo_status status = allo_pool_init(&p, buf_aligned, tests[i].buf_size,
                                        tests[i].chunk_size, tests[i].align);
    ALLO_TEST_ASSERT_STATUS_MSG(
        ALLO_OK, status, "allocator initialization should be succeed");
    allo_pool_assert(&p);

    size_t chunk_count = (p.end - p.start) / p.chunk_size;
    void *dest;
    for (size_t chunk = 0; chunk < chunk_count; ++chunk) {
      status = allo_pool_alloc(&dest, &p);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status,
                                            "allocation should succeed");
    }

    void *to_remove =
        (void *)((uintptr_t)p.start + (tests[i].free_index * p.chunk_size));

    allo_pool_free(&p, to_remove);

    TEST_ASSERT_EQUAL_PTR_MESSAGE(
        to_remove, p.free_list,
        "free list should point to the latest freed chunk");

    allo_pool_assert(&p);
    free(buf);
  }
}

void test_free_sequential(void) {
  const size_t buf_size = 1024;
  const size_t align = 16;
  const size_t chunk_size = 256;

  struct {
    size_t free_seq[4];
  } tests[] = {
      {{0, 1, 2, 3}}, {{3, 2, 1, 0}}, {{3, 1, 2, 0}},
      {{1, 3, 0, 2}}, {{2, 0, 3, 1}},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    void *buf;
    void *buf_aligned = TEST_UTILS_MALLOC_ALIGNED(&buf, buf_size, align);

    allo_pool p = {0};
    allo_status status =
        allo_pool_init(&p, buf_aligned, buf_size, chunk_size, align);
    ALLO_TEST_ASSERT_STATUS_MSG(
        ALLO_OK, status, "allocator initialization should be succeed");
    allo_pool_assert(&p);

    for (size_t chunk = 0; chunk < 4; ++chunk) {
      void *dest;
      status = allo_pool_alloc(&dest, &p);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status,
                                            "allocation should succeed");
    }
    TEST_ASSERT_EQUAL_PTR_MESSAGE(NULL, p.free_list,
                                  "free list should point to NULL");

    void *chunk_positions[4];
    for (size_t chunk = 0; chunk < 4; ++chunk) {
      chunk_positions[chunk] = (void *)(p.start + chunk * chunk_size);
    }

    size_t free_count =
        sizeof(tests[i].free_seq) / sizeof(tests[i].free_seq[0]);
    for (size_t to_free = 0; to_free < free_count; ++to_free) {
      allo_pool_free(&p, chunk_positions[to_free]);
      TEST_ASSERT_EQUAL_INT_MESSAGE(
          chunk_positions[to_free], p.free_list,
          "free list should point to the latest freed chunks");
    }

    allo_pool_assert(&p);
    free(buf);
  }
}

void test_sequential(void) {
  const size_t buf_size = 1024;
  const size_t align = 16;
  const size_t chunk_size = 256;

  void *buf;
  void *buf_aligned = TEST_UTILS_MALLOC_ALIGNED(&buf, buf_size, align);

  allo_pool p = {0};
  allo_status status =
      allo_pool_init(&p, buf_aligned, buf_size, chunk_size, align);
  ALLO_TEST_ASSERT_STATUS_MSG(
      ALLO_OK, status, "allocator initialization should be succeed");
  allo_pool_assert(&p);

  void *chunk_positions[4];
  for (size_t chunk = 0; chunk < 4; ++chunk) {
    chunk_positions[chunk] = (void *)((uintptr_t)p.start + chunk * chunk_size);
  }

  void *dest;

  status = allo_pool_alloc(&dest, &p);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status,
                                        "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(chunk_positions[0], dest,
                                "first chunk should be allocated");
  TEST_ASSERT_EQUAL_INT_MESSAGE(chunk_positions[1], p.free_list,
                                "free list should point to the second chunk");

  status = allo_pool_alloc(&dest, &p);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status,
                                        "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(chunk_positions[1], dest,
                                "second chunk should be allocated");
  TEST_ASSERT_EQUAL_INT_MESSAGE(chunk_positions[2], p.free_list,
                                "free list should point to the second chunk");

  status = allo_pool_alloc(&dest, &p);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status,
                                        "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(chunk_positions[2], dest,
                                "third chunk should be allocated");
  TEST_ASSERT_EQUAL_INT_MESSAGE(chunk_positions[3], p.free_list,
                                "free list should point to the second chunk");

  allo_pool_free(&p, chunk_positions[2]);
  TEST_ASSERT_EQUAL_INT_MESSAGE(chunk_positions[2], p.free_list,
                                "free list should point to the second chunk");

  status = allo_pool_alloc(&dest, &p);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status,
                                        "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(chunk_positions[2], dest,
                                "third chunk should be allocated");
  TEST_ASSERT_EQUAL_INT_MESSAGE(chunk_positions[3], p.free_list,
                                "free list should point to the second chunk");

  status = allo_pool_alloc(&dest, &p);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status,
                                        "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(chunk_positions[3], dest,
                                "fourth chunk should be allocated");
  TEST_ASSERT_EQUAL_INT_MESSAGE(NULL, p.free_list,
                                "free list should point to the second chunk");

  status = allo_pool_alloc(&dest, &p);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OOM, status,
                                        "allocation should fail due to OOM");

  allo_pool_assert(&p);
  free(buf);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_init_chunk_size_and_align);
  RUN_TEST(test_init_memory_region);
  RUN_TEST(test_init_null_allocator);
  RUN_TEST(test_init_null_buffer);
  RUN_TEST(test_init_zero_buf_size);
  RUN_TEST(test_init_zero_chunk_size);
  RUN_TEST(test_init_zero_alignment);
  RUN_TEST(test_init_memory_not_aligned);

  RUN_TEST(test_alloc_first_alloc);
  RUN_TEST(test_alloc_allocs_till_oom);

  RUN_TEST(test_free_one);
  RUN_TEST(test_free_sequential);

  RUN_TEST(test_sequential);

  return UNITY_END();
}
