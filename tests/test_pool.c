#define ALLO_POOL_IMPLEMENTATION
#include "allo/allo.h"
#include "test_utils.h"
#include "unity.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Tests that the chunk_size is always at least sizeof(void*) and aligned.
void test_init_chunk_size_and_align(void) {
  for (size_t chunk_size = 1; chunk_size <= sizeof(void *) * 4; ++chunk_size) {
    const size_t alignments[] = {1, 2, 4, 8, 16};
    for (size_t i = 0; i < sizeof(alignments) / sizeof(alignments[0]); ++i) {
      size_t align = alignments[i];

      size_t expected_align = align >= sizeof(void *) ? align : sizeof(void *);

      size_t expected_chunk_size =
          chunk_size >= sizeof(void *) ? chunk_size : sizeof(void *);
      expected_chunk_size =
          (expected_chunk_size + expected_align - 1) & ~(expected_align - 1);

      void *buf;
      size_t bufsize = sizeof(void *) * 64;
      void *buf_aligned = malloc_aligned(&buf, bufsize, expected_align);

      struct allo_pool p = {0};
      enum allo_status status =
          allo_pool_init(&p, buf_aligned, bufsize, chunk_size, align);

      TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                    "allocator initialization should succeed");
      TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                    "allocator initialization should succeed");

      TEST_ASSERT_EQUAL_size_t_MESSAGE(expected_chunk_size, p.allo__chunk_size,
                                       "chunk size should match");
      TEST_ASSERT_EQUAL_size_t_MESSAGE(expected_align, p.allo__align,
                                       "alignment should match");
      TEST_ASSERT_TRUE_MESSAGE(p.allo__chunk_size % p.allo__align == 0,
                               "chunk size must be aligned");

      TEST_ASSERT_EQUAL_PTR_MESSAGE(p.allo__start, p.allo__free_list,
                                    "free list should point to the start");

      allo__assert_pool(&p);

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
    void *buf_aligned = malloc_aligned(&buf, tests[i].buf_size, tests[i].align);
    struct allo_pool p = {0};
    enum allo_status status =
        allo_pool_init(&p, buf_aligned, tests[i].buf_size, tests[i].chunk_size,
                       tests[i].align);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                  "allocator initialization should be succeed");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(buf_aligned, p.allo__start,
                                  "allocator start should match");
    TEST_ASSERT_EQUAL_PTR_MESSAGE((uintptr_t)buf_aligned +
                                      tests[i].expected_end_offset,
                                  p.allo__end, "allocator end should match");

    TEST_ASSERT_EQUAL_PTR_MESSAGE(p.allo__start, p.allo__free_list,
                                  "free list should point to the start");

    free(buf);
  }
}

void test_init_null_allocator(void) {
  uint8_t buf[0x100];
  enum allo_status status = allo_pool_init(NULL, buf, 0x100, 0x10, 0x1);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      ALLO_ERR_NULL, status,
      "error should match for receiving a NULL allocator");
}

void test_init_null_buffer(void) {
  struct allo_pool p = {0};
  enum allo_status status = allo_pool_init(&p, NULL, 0x100, 0x10, 0x1);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      ALLO_ERR_NULL, status, "error should match for receiving a NULL buffer");
}

void test_init_zero_buf_size(void) {
  uint8_t buf[0x100];
  struct allo_pool p = {0};
  enum allo_status status = allo_pool_init(&p, buf, 0, 0x10, 0x1);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      ALLO_ERR_INVALID_SIZE, status,
      "error should match for receiving a zero buf size");
}

void test_init_zero_chunk_size(void) {
  uint8_t buf[0x100];
  struct allo_pool p = {0};
  enum allo_status status = allo_pool_init(&p, buf, 0x100, 0x0, sizeof(void *));
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      ALLO_ERR_INVALID_SIZE, status,
      "error should match for receiving a zero chunk size");
}

void test_init_zero_alignment(void) {
  uint8_t buf[0x100];
  struct allo_pool p = {0};
  enum allo_status status = allo_pool_init(&p, buf, 0x100, 0x10, 0x0);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      ALLO_ERR_INVALID_ALIGN, status,
      "error should match for receiving zero alignment");
}

void test_init_memory_not_aligned(void) {
  size_t buf_size = 0x100;
  uint8_t buf[0x100];

  size_t align = sizeof(void *);

  uintptr_t aligned_buf = ((uintptr_t)buf + align - 1) & ~(align - 1);
  uint8_t *unaligned_buf = (uint8_t *)(aligned_buf + 1);

  buf_size -= (size_t)((uintptr_t)unaligned_buf - (uintptr_t)buf);

  struct allo_pool p = {0};
  enum allo_status status =
      allo_pool_init(&p, unaligned_buf, buf_size, 0x10, align);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      ALLO_ERR_MEM_NOT_ALIGNED, status,
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
    void *buf_aligned = malloc_aligned(&buf, tests[i].buf_size, tests[i].align);

    struct allo_pool p = {0};
    enum allo_status status =
        allo_pool_init(&p, buf_aligned, tests[i].buf_size, tests[i].chunk_size,
                       tests[i].align);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                  "allocator initialization should be succeed");
    allo__assert_pool(&p);

    TEST_ASSERT_NOT_NULL_MESSAGE(p.allo__free_list,
                                 "free list should not be NULL");
    void *next = *(void **)p.allo__free_list;

    void *dest;
    status = allo_pool_alloc(&dest, &p);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");

    TEST_ASSERT_EQUAL_PTR_MESSAGE(p.allo__start, dest,
                                  "allocated memory should match");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(
        dest, p.allo__free_list,
        "free list should not point to the recently allocated memory");

    TEST_ASSERT_EQUAL_PTR_MESSAGE(
        next, p.allo__free_list,
        "free list should be pointing to the next chunk");

    allo__assert_pool(&p);
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
    void *buf_aligned = malloc_aligned(&buf, tests[i].buf_size, tests[i].align);

    struct allo_pool p = {0};
    enum allo_status status =
        allo_pool_init(&p, buf_aligned, tests[i].buf_size, tests[i].chunk_size,
                       tests[i].align);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                  "allocator initialization should be succeed");
    allo__assert_pool(&p);

    size_t chunk_counts = (p.allo__end - p.allo__start) / p.allo__chunk_size;
    void *dest;
    for (size_t chunk = 0; chunk < chunk_counts; ++chunk) {

      void *next = *(void **)p.allo__free_list;
      status = allo_pool_alloc(&dest, &p);
      TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                    "allocation should succeed");

      TEST_ASSERT_NOT_EQUAL_MESSAGE(
          dest, p.allo__free_list,
          "free list should not point to the recently allocated memory");

      TEST_ASSERT_EQUAL_PTR_MESSAGE(
          next, p.allo__free_list,
          "free list should be pointing to the next chunk");
    }

    TEST_ASSERT_NULL_MESSAGE(
        p.allo__free_list,
        "free list should now point to NULL as allocator is full");

    status = allo_pool_alloc(&dest, &p);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OOM, status,
                                  "allocation should fail due to OOM");
    allo__assert_pool(&p);
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
    void *buf_aligned = malloc_aligned(&buf, tests[i].buf_size, tests[i].align);

    struct allo_pool p = {0};
    enum allo_status status =
        allo_pool_init(&p, buf_aligned, tests[i].buf_size, tests[i].chunk_size,
                       tests[i].align);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                  "allocator initialization should be succeed");
    allo__assert_pool(&p);

    size_t chunk_count = (p.allo__end - p.allo__start) / p.allo__chunk_size;
    void *dest;
    for (size_t chunk = 0; chunk < chunk_count; ++chunk) {
      status = allo_pool_alloc(&dest, &p);
      TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                    "allocation should succeed");
    }

    void *to_remove = (void *)((uintptr_t)p.allo__start +
                               (tests[i].free_index * p.allo__chunk_size));

    allo_pool_free(&p, to_remove);

    TEST_ASSERT_EQUAL_PTR_MESSAGE(
        to_remove, p.allo__free_list,
        "free list should point to the latest freed chunk");

    allo__assert_pool(&p);
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
    void *buf_aligned = malloc_aligned(&buf, buf_size, align);

    struct allo_pool p = {0};
    enum allo_status status =
        allo_pool_init(&p, buf_aligned, buf_size, chunk_size, align);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                  "allocator initialization should be succeed");
    allo__assert_pool(&p);

    for (size_t chunk = 0; chunk < 4; ++chunk) {
      void *dest;
      status = allo_pool_alloc(&dest, &p);
      TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                    "allocation should succeed");
    }
    TEST_ASSERT_EQUAL_PTR_MESSAGE(NULL, p.allo__free_list,
                                  "free list should point to NULL");

    void *chunk_positions[4];
    for (size_t chunk = 0; chunk < 4; ++chunk) {
      chunk_positions[chunk] = (void *)(p.allo__start + chunk * chunk_size);
    }

    size_t free_count =
        sizeof(tests[i].free_seq) / sizeof(tests[i].free_seq[0]);
    for (size_t to_free = 0; to_free < free_count; ++to_free) {
      allo_pool_free(&p, chunk_positions[to_free]);
      TEST_ASSERT_EQUAL_INT_MESSAGE(
          chunk_positions[to_free], p.allo__free_list,
          "free list should point to the latest freed chunks");
    }

    allo__assert_pool(&p);
    free(buf);
  }
}

void test_sequential(void) {
  const size_t buf_size = 1024;
  const size_t align = 16;
  const size_t chunk_size = 256;

  void *buf;
  void *buf_aligned = malloc_aligned(&buf, buf_size, align);

  struct allo_pool p = {0};
  enum allo_status status =
      allo_pool_init(&p, buf_aligned, buf_size, chunk_size, align);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                "allocator initialization should be succeed");
  allo__assert_pool(&p);

  void *chunk_positions[4];
  for (size_t chunk = 0; chunk < 4; ++chunk) {
    chunk_positions[chunk] =
        (void *)((uintptr_t)p.allo__start + chunk * chunk_size);
  }

  void *dest;

  status = allo_pool_alloc(&dest, &p);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(chunk_positions[0], dest,
                                "first chunk should be allocated");
  TEST_ASSERT_EQUAL_INT_MESSAGE(chunk_positions[1], p.allo__free_list,
                                "free list should point to the second chunk");

  status = allo_pool_alloc(&dest, &p);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(chunk_positions[1], dest,
                                "second chunk should be allocated");
  TEST_ASSERT_EQUAL_INT_MESSAGE(chunk_positions[2], p.allo__free_list,
                                "free list should point to the second chunk");

  status = allo_pool_alloc(&dest, &p);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(chunk_positions[2], dest,
                                "third chunk should be allocated");
  TEST_ASSERT_EQUAL_INT_MESSAGE(chunk_positions[3], p.allo__free_list,
                                "free list should point to the second chunk");

  allo_pool_free(&p, chunk_positions[2]);
  TEST_ASSERT_EQUAL_INT_MESSAGE(chunk_positions[2], p.allo__free_list,
                                "free list should point to the second chunk");

  status = allo_pool_alloc(&dest, &p);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(chunk_positions[2], dest,
                                "third chunk should be allocated");
  TEST_ASSERT_EQUAL_INT_MESSAGE(chunk_positions[3], p.allo__free_list,
                                "free list should point to the second chunk");

  status = allo_pool_alloc(&dest, &p);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(chunk_positions[3], dest,
                                "fourth chunk should be allocated");
  TEST_ASSERT_EQUAL_INT_MESSAGE(NULL, p.allo__free_list,
                                "free list should point to the second chunk");

  status = allo_pool_alloc(&dest, &p);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OOM, status,
                                "allocation should fail due to OOM");

  allo__assert_pool(&p);
  free(buf);
}

void setUp(void) {}

void tearDown(void) {}

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
