#include "allo/allo.h"
#include "allo/internal/common.h"
#include "allo/pool.h"
#include "allo/status.h"
#include "allo_test/allo_test.h"
#include "unity.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unity_internals.h>

void setUp(void) {}

void tearDown(void) {}

// Alignments for buffers and allocations that tests will use.
const size_t aligns[] = {sizeof(void *), sizeof(void *) << 1,
                         sizeof(void *) << 2};

// Sizes for pool chunks that tests will use.
const size_t chunk_sizes[] = {sizeof(void *), sizeof(void *) * 2,
                              sizeof(void *) * 3, sizeof(void *) * 4};

// Number of chunks to fit in allocator that tests will use.
const size_t chunk_counts[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Tests when init takes in a null allocator.
static void test_allo_pool_init_null_allocator(void) {
  enum {
    CHUNK_SIZE = 32,
    CHUNK_COUNT = 10,
    BUF_SIZE = CHUNK_SIZE * CHUNK_COUNT,
    ALIGN = 16,
  };
  uint8_t buf[BUF_SIZE] __attribute__((aligned(ALIGN))) = {0};
  allo_status status = allo_pool_init(NULL, buf, BUF_SIZE, CHUNK_SIZE, ALIGN);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status, "init must fail");
}

// Tests when init takes in a null buffer.
static void test_allo_pool_init_null_buf(void) {
  enum {
    CHUNK_SIZE = 32,
    CHUNK_COUNT = 10,
    BUF_SIZE = CHUNK_SIZE * CHUNK_COUNT,
    ALIGN = 16,
  };
  allo_pool p = {0};
  allo_status status = allo_pool_init(&p, NULL, BUF_SIZE, CHUNK_SIZE, ALIGN);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status, "init must fail");
}

// Tests when init takes in a zero sized chunk size.
static void test_allo_pool_init_zero_chunk_size(void) {
  enum {
    CHUNK_SIZE = 32,
    CHUNK_COUNT = 10,
    BUF_SIZE = CHUNK_SIZE * CHUNK_COUNT,
    ALIGN = 16,
  };
  uint8_t buf[BUF_SIZE] __attribute__((aligned(ALIGN))) = {0};
  allo_pool p = {0};
  allo_status status = allo_pool_init(&p, buf, BUF_SIZE, 0, ALIGN);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_SIZE, status, "init must fail");
}

// Tests when init takes in a zero sized alignment.
static void test_allo_pool_init_zero_align(void) {
  enum {
    CHUNK_SIZE = 32,
    CHUNK_COUNT = 10,
    BUF_SIZE = CHUNK_SIZE * CHUNK_COUNT,
    ALIGN = 16,
  };
  uint8_t buf[BUF_SIZE] __attribute__((aligned(ALIGN))) = {0};
  allo_pool p = {0};
  allo_status status = allo_pool_init(&p, buf, BUF_SIZE, CHUNK_SIZE, 0);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_ALIGN, status, "init must fail");
}

// Tests when init takes in an alignment smaller than sizeof(void*).
static void test_allo_pool_init_align_too_small(void) {
  enum {
    CHUNK_SIZE = 32,
    CHUNK_COUNT = 10,
    BUF_SIZE = CHUNK_SIZE * CHUNK_COUNT,
    ALIGN = 16,
  };

  for (size_t i = 0; i < sizeof(void *); ++i) {
    uint8_t buf[BUF_SIZE] __attribute__((aligned(ALIGN))) = {0};
    allo_pool p = {0};
    allo_status status = allo_pool_init(&p, buf, BUF_SIZE, CHUNK_SIZE, i);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_ALIGN, status,
                                "init must fail");
  }
}

// Tests when align is not a power of 2.
static void test_allo_pool_init_align_not_pow2(void) {
  enum {
    CHUNK_SIZE = 32,
    CHUNK_COUNT = 10,
    BUF_SIZE = CHUNK_SIZE * CHUNK_COUNT,
    ALIGN = 16,
  };
  const size_t not_pow2[] = {3, 5, 6, 7, 9, 10};
  for (size_t i = 0; i < ALLO_ARR_LEN(not_pow2); ++i) {
    uint8_t buf[BUF_SIZE] __attribute__((aligned(ALIGN))) = {0};
    allo_pool p = {0};
    allo_status status =
        allo_pool_init(&p, buf, BUF_SIZE, CHUNK_SIZE, not_pow2[i]);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_ALIGN, status,
                                "init must fail");
  }
}

// Tests when init takes in a chunk size smaller than sizeof(void*)
static void test_allo_pool_init_chunk_size_too_small(void) {
  size_t chunk_count = 10;
  for (size_t chunk_size = 1; chunk_size < sizeof(void *); ++chunk_size) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      size_t buf_size = chunk_size * chunk_count;
      void *buf = {0};
      void *buf_aligned = ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
      allo_pool p = {0};
      allo_status status = allo_pool_init(&p, buf_aligned, buf_size, chunk_size,
                                          aligns[align_i]);
      ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_SIZE, status,
                                  "init must fail");
      free(buf);
    }
  }
}

// Tests when init takes in a chunk size is not a multiple of alignment.
static void test_allo_pool_init_chunk_size_not_a_multiple_of_align(void) {
  size_t chunk_count = 10;
  for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
    size_t chunk_size = aligns[align_i] * 2 + 1;
    TEST_ASSERT_TRUE_MESSAGE(
        chunk_size % aligns[align_i] != 0,
        "chunk size to test must not be a multiple of align");

    size_t buf_size = chunk_size * chunk_count;
    void *buf = {0};
    void *buf_aligned = ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
    allo_pool p = {0};
    allo_status status =
        allo_pool_init(&p, buf_aligned, buf_size, chunk_size, aligns[align_i]);
    ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_SIZE, status,
                                "init must fail");
    free(buf);
  }
}

// Tests when init takes in a chunk size exceeding the buffer size.
static void test_allo_pool_init_chunk_size_exceeds_buf_size(void) {
  enum {
    CHUNK_SIZE = 32,
    CHUNK_COUNT = 10,
    BUF_SIZE = CHUNK_SIZE * CHUNK_COUNT,
    ALIGN = 16,
  };
  uint8_t buf[BUF_SIZE] __attribute__((aligned(ALIGN))) = {0};
  allo_pool p = {0};
  allo_status status = allo_pool_init(&p, buf, BUF_SIZE, BUF_SIZE + 1, ALIGN);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_SIZE, status, "init must fail");
}

// Tests when init takes in a buffer that is not aligned to alignment
// requirements.
static void test_allo_pool_init_buf_not_aligned(void) {
  enum {
    CHUNK_SIZE = 32,
    CHUNK_COUNT = 10,
    BUF_SIZE = CHUNK_SIZE * CHUNK_COUNT,
    ALIGN = 16,
  };
  uint8_t buf[BUF_SIZE] __attribute__((aligned(ALIGN))) = {0};
  allo_pool p = {0};
  allo_status status =
      allo_pool_init(&p, buf + 1, BUF_SIZE - 1, CHUNK_SIZE, ALIGN);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_ALIGN, status, "init must fail");
}

// Tests chunk capacity.
static void test_allo_pool_chunk_cap(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        TEST_ASSERT_EQUAL_MESSAGE(chunk_counts[count_i],
                                  allo_pool_chunk_cap(&p),
                                  "chunk capacity must match");
        free(buf);
      }
    }
  }
}

// Tests that allocator has the correct state on a successful init.
static void test_allo_pool_init_ok(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = NULL;
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        TEST_ASSERT_EQUAL_PTR_MESSAGE(buf_aligned, p.start,
                                      "start of memory region must match");
        TEST_ASSERT_EQUAL_PTR_MESSAGE((uintptr_t)buf_aligned + buf_size, p.end,
                                      "end of memory region must match");
        TEST_ASSERT_EQUAL_PTR_MESSAGE(
            buf_aligned, p.free_list,
            "free list must start at the beginning of the memory region");

        TEST_ASSERT_TRUE_MESSAGE(p.chunk_size % aligns[align_i] == 0,
                                 "chunk size must be a multiple of alignment");

        free(buf);
      }
    }
  }
}

static void test_allo_pool_free_chunks(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        size_t cap = allo_pool_chunk_cap(&p);
        TEST_ASSERT_EQUAL_MESSAGE(
            cap, allo_pool_free_chunks(&p),
            "free chunks must match max capacity when allocator is empty");

        void *dest = NULL;
        for (size_t i = 0; i < cap; ++i) {
          status = allo_pool_alloc(&dest, &p);
          ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
          allo_pool_assert(&p);
          TEST_ASSERT_EQUAL_MESSAGE(
              cap - i - 1, allo_pool_free_chunks(&p),
              "free chunks must decrement by 1 per allocation");
        }
        TEST_ASSERT_NULL_MESSAGE(p.free_list, "allocator must be full");
        TEST_ASSERT_EQUAL_MESSAGE(
            0, allo_pool_free_chunks(&p),
            "no free chunks must remain when allocator is full");

        free(buf);
      }
    }
  }
}

// Tests the state of the allocator and allocated memory on a successful init on
// an empty allocator.
static void test_allo_pool_alloc_unsafe_ok(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        void *dest = NULL;
        status = allo_pool_alloc_unsafe(&dest, &p);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        allo_pool_assert(&p);

        TEST_ASSERT_EQUAL_PTR_MESSAGE(
            p.start, dest, "alloc must be at the start of the memory region");

        void *expected_free_list = allo_pool_chunk_cap(&p) > 1
                                       ? (void *)(p.start + p.chunk_size)
                                       : NULL;
        TEST_ASSERT_EQUAL_PTR_MESSAGE(
            expected_free_list, p.free_list,
            "free list must be updated to point to the next free chunk");
        free(buf);
      }
    }
  }
}

// Tests when allocation is attempted on an allocator with insufficient free
// memory.
static void test_allo_pool_alloc_unsafe_oom(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        size_t cap = allo_pool_chunk_cap(&p);

        void *dest = NULL;
        for (size_t i = 0; i < cap; ++i) {
          status = allo_pool_alloc_unsafe(&dest, &p);
          ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
          allo_pool_assert(&p);
        }

        TEST_ASSERT_NULL_MESSAGE(p.free_list, "allocator must be full");

        status = allo_pool_alloc(&dest, &p);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OOM, status,
                                    "alloc must result in OOM");
        allo_pool_assert(&p);

        free(buf);
      }
    }
  }
}

// Tests when alloc takes in a null destination.
static void test_allo_pool_alloc_null_dest(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL,
                                    allo_pool_alloc(NULL, &p),
                                    "alloc must fail");
        free(buf);
      }
    }
  }
}

// Tests when alloc takes in a null allocator.
static void test_allo_pool_alloc_null_allocator(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        void *dest = NULL;
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL,
                                    allo_pool_alloc(&dest, NULL),
                                    "alloc must fail");
        free(buf);
      }
    }
  }
}

// Tests the state of the allocator and allocated memory on a successful init on
// an empty allocator.
static void test_allo_pool_alloc_ok(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        void *dest = NULL;
        status = allo_pool_alloc(&dest, &p);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        allo_pool_assert(&p);

        TEST_ASSERT_EQUAL_PTR_MESSAGE(
            p.start, dest, "alloc must be at the start of the memory region");

        void *expected_free_list = allo_pool_chunk_cap(&p) > 1
                                       ? (void *)(p.start + p.chunk_size)
                                       : NULL;
        TEST_ASSERT_EQUAL_PTR_MESSAGE(
            expected_free_list, p.free_list,
            "free list must be updated to point to the next free chunk");
        free(buf);
      }
    }
  }
}

// Tests when allocation is attempted on an allocator with insufficient free
// memory.
static void test_allo_pool_alloc_oom(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        size_t cap = allo_pool_chunk_cap(&p);

        void *dest = NULL;
        for (size_t i = 0; i < cap; ++i) {
          status = allo_pool_alloc(&dest, &p);
          ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
          allo_pool_assert(&p);
        }

        TEST_ASSERT_NULL_MESSAGE(p.free_list, "allocator must be full");

        status = allo_pool_alloc(&dest, &p);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OOM, status,
                                    "alloc must result in OOM");
        allo_pool_assert(&p);

        free(buf);
      }
    }
  }
}

// Tests freeing a chunk from the allocator.
static void test_allo_pool_free_unsafe_ok(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        void *dest = NULL;
        status = allo_pool_alloc(&dest, &p);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        allo_pool_assert(&p);

        status = allo_pool_free_unsafe(&p, dest);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "free must succeed");

        TEST_ASSERT_EQUAL_PTR_MESSAGE(
            dest, p.free_list, "free list must point to latest freed memory");
        allo_pool_assert(&p);

        free(buf);
      }
    }
  }
}

// Tests when free takes in a null allocator.
static void test_allo_pool_free_null_allocator(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        void *dest = NULL;
        status = allo_pool_alloc(&dest, &p);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        allo_pool_assert(&p);

        status = allo_pool_free(NULL, dest);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL,
                                    allo_pool_free(NULL, dest),
                                    "free must fail");
        allo_pool_assert(&p);

        free(buf);
      }
    }
  }
}

// Tests when free takes in a null ptr.
static void test_allo_pool_free_null_ptr(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        void *dest = NULL;
        status = allo_pool_alloc(&dest, &p);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        allo_pool_assert(&p);

        status = allo_pool_free(&p, NULL);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status,
                                    "free must fail");
        allo_pool_assert(&p);

        free(buf);
      }
    }
  }
}

// Tests when free takes in a pointer outside of the allocator's memory region.
static void test_allo_pool_free_ptr_is_out_of_bounds(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        void *dest = NULL;
        status = allo_pool_alloc(&dest, &p);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        allo_pool_assert(&p);

        status = allo_pool_free(&p, (void *)(p.start - 1));
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_ADDR, status,
                                    "free must fail");
        allo_pool_assert(&p);

        status = allo_pool_free(&p, (void *)(p.end));
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_ADDR, status,
                                    "free must fail");
        allo_pool_assert(&p);

        status = allo_pool_free(&p, (void *)(p.end + 1));
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_ADDR, status,
                                    "free must fail");
        allo_pool_assert(&p);

        free(buf);
      }
    }
  }
}

// Tests when free takes in a pointer is within the allocator's memory region
// but is not a valid chunk's address.
static void test_allo_pool_free_ptr_is_in_memory_region_but_not_valid(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      // skip chunks of size 1 as any address within region is valid.
      if (chunk_sizes[size_i] == 1) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        void *dest = NULL;
        status = allo_pool_alloc(&dest, &p);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        allo_pool_assert(&p);

        uintptr_t invalid_ptr = p.start + chunk_sizes[size_i] - 1;
        TEST_ASSERT_TRUE_MESSAGE(
            invalid_ptr % p.chunk_size != 0,
            "invalid ptr must not be a valid chunk address");
        status = allo_pool_free(&p, (void *)invalid_ptr);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_ADDR, status,
                                    "free must fail");
        allo_pool_assert(&p);

        free(buf);
      }
    }
  }
}

// Tests freeing a chunk from the allocator.
static void test_allo_pool_free_ok(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        void *dest = NULL;
        status = allo_pool_alloc(&dest, &p);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
        allo_pool_assert(&p);

        status = allo_pool_free(&p, dest);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "free must succeed");

        TEST_ASSERT_EQUAL_PTR_MESSAGE(
            dest, p.free_list, "free list must point to latest freed memory");
        allo_pool_assert(&p);

        free(buf);
      }
    }
  }
}

// Tests when free all takes in a null allocator.
static void test_allo_pool_free_all_null_allocator(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        status = allo_pool_free_all(NULL);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status,
                                    "free all must fail");

        size_t cap = allo_pool_chunk_cap(&p);

        void *dest = NULL;
        for (size_t i = 0; i < cap; ++i) {
          status = allo_pool_alloc(&dest, &p);
          ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
          allo_pool_assert(&p);
        }

        TEST_ASSERT_NULL_MESSAGE(p.free_list, "allocator must be full");

        status = allo_pool_free_all(NULL);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_ERR_INVALID_NULL, status,
                                    "free all must fail");
        allo_pool_assert(&p);

        free(buf);
      }
    }
  }
}

// Tests freeing all allocated chunks
static void test_allo_pool_free_all_ok(void) {
  for (size_t size_i = 0; size_i < ALLO_ARR_LEN(chunk_sizes); ++size_i) {
    for (size_t align_i = 0; align_i < ALLO_ARR_LEN(aligns); ++align_i) {
      if (chunk_sizes[size_i] < aligns[align_i] ||
          chunk_sizes[size_i] % aligns[align_i] != 0) {
        continue;
      }
      for (size_t count_i = 0; count_i < ALLO_ARR_LEN(chunk_counts);
           ++count_i) {
        size_t buf_size = chunk_counts[count_i] * chunk_sizes[size_i];
        void *buf = {0};
        void *buf_aligned =
            ALLO_TEST_MEM_ALLOC(&buf, buf_size, aligns[align_i]);
        allo_pool p = {0};
        allo_status status = allo_pool_init(
            &p, buf_aligned, buf_size, chunk_sizes[size_i], aligns[align_i]);
        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "init must succeed");
        allo_pool_assert(&p);

        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, allo_pool_free_all(&p),
                                    "free all must succeed");

        size_t cap = allo_pool_chunk_cap(&p);

        void *dest = NULL;
        for (size_t i = 0; i < cap; ++i) {
          status = allo_pool_alloc(&dest, &p);
          ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "alloc must succeed");
          allo_pool_assert(&p);
        }

        TEST_ASSERT_NULL_MESSAGE(p.free_list, "allocator must be full");

        ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, allo_pool_free_all(&p),
                                    "free all must succeed");
        allo_pool_assert(&p);

        TEST_ASSERT_EQUAL_MESSAGE(
            allo_pool_chunk_cap(&p), allo_pool_free_chunks(&p),
            "free chunks must be equal to the max capacity of the allocator");

        free(buf);
      }
    }
  }
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_allo_pool_init_null_allocator);
  RUN_TEST(test_allo_pool_init_null_buf);
  RUN_TEST(test_allo_pool_init_zero_chunk_size);
  RUN_TEST(test_allo_pool_init_zero_align);
  RUN_TEST(test_allo_pool_init_align_too_small);
  RUN_TEST(test_allo_pool_init_align_not_pow2);
  RUN_TEST(test_allo_pool_init_chunk_size_too_small);
  RUN_TEST(test_allo_pool_init_chunk_size_not_a_multiple_of_align);
  RUN_TEST(test_allo_pool_init_chunk_size_exceeds_buf_size);
  RUN_TEST(test_allo_pool_init_buf_not_aligned);
  RUN_TEST(test_allo_pool_init_ok);

  RUN_TEST(test_allo_pool_chunk_cap);

  RUN_TEST(test_allo_pool_free_chunks);

  RUN_TEST(test_allo_pool_alloc_unsafe_ok);
  RUN_TEST(test_allo_pool_alloc_unsafe_oom);

  RUN_TEST(test_allo_pool_alloc_null_dest);
  RUN_TEST(test_allo_pool_alloc_null_allocator);
  RUN_TEST(test_allo_pool_alloc_ok);
  RUN_TEST(test_allo_pool_alloc_oom);

  RUN_TEST(test_allo_pool_free_unsafe_ok);

  RUN_TEST(test_allo_pool_free_null_allocator);
  RUN_TEST(test_allo_pool_free_null_ptr);
  RUN_TEST(test_allo_pool_free_ptr_is_out_of_bounds);
  RUN_TEST(test_allo_pool_free_ptr_is_in_memory_region_but_not_valid);
  RUN_TEST(test_allo_pool_free_ok);

  RUN_TEST(test_allo_pool_free_all_null_allocator);
  RUN_TEST(test_allo_pool_free_all_ok);

  return UNITY_END();
}
