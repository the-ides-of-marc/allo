#include "unity_internals.h"
#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

#define ALLO_IMPLEMENTATION
#include "allo.h"
#include "unity.h"

void test_init(void) {
  uint8_t buf[0x100];
  struct allo_fixed_bump b;
  enum allo_status status = allo_fixed_bump_init(&b, buf, 0x100);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                "allocator initialization should succeed");
  allo__assert_fixed_bump(&b);

  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      buf, b.start,
      "start should point to the inclusive start of memory range");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      buf + 0x100, b.end,
      "end should point to the exclusive end of memory range");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      b.end, b.cursor,
      "cursor should point to the exclusive end of memory range");
}

void test_alloc_first_alloc(void) {
  struct {
    size_t size;
    size_t align;
  } tests[] = {
      // 1 byte; different alignments
      {1, 1},
      {1, 2},
      {1, 4},
      {1, 8},
      {1, 16},

      // 8 bytes; different alignments
      {8, 1},
      {8, 2},
      {8, 4},
      {8, 8},
      {8, 16},

      // 7 bytes (odd + not power of 2); different alignments
      {7, 1},
      {7, 2},
      {7, 4},
      {7, 8},
      {7, 16},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    uint8_t buf[0x10];
    struct allo_fixed_bump b;
    enum allo_status status = allo_fixed_bump_init(&b, buf, 0x10);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                  "allocator initialization should succeed");
    allo__assert_fixed_bump(&b);

    void *dest = NULL;
    status = allo_fixed_bump_alloc(&dest, &b, tests[i].size, tests[i].align);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(
        dest, b.cursor,
        "dest should be at the current cursor after allocation");
    allo__assert_fixed_bump(&b);
  }
}

void test_alloc_subsequent_allocs(void) {
  struct {
    size_t size;
    size_t align;
    uintptr_t starting_offset;
    uintptr_t expected_offset;
  } tests[] = {
      // 1 byte; different alignments
      {1, 1, 0x10, 0x11},
      {1, 2, 0x10, 0x12},
      {1, 4, 0x10, 0x14},
      {1, 8, 0x10, 0x18},
      {1, 16, 0x10, 0x20},

      // 8 bytes; different alignments
      {8, 1, 0x10, 0x18},
      {8, 2, 0x10, 0x18},
      {8, 4, 0x10, 0x18},
      {8, 8, 0x10, 0x18},
      {8, 16, 0x10, 0x20},

      // 7 bytes (odd + not power of 2); different alignments
      {7, 1, 0x10, 0x17},
      {7, 2, 0x10, 0x18},
      {7, 4, 0x10, 0x18},
      {7, 8, 0x10, 0x18},
      {7, 16, 0x10, 0x20},

      // variable bytes; last possible allocation
      {1, 1, 0xff, 0x100},
      {1, 2, 0xff, 0x100},
      {1, 4, 0xff, 0x100},
      {1, 8, 0xff, 0x100},
      {1, 16, 0xff, 0x100},
      {8, 1, 0xf8, 0x100},
      {8, 2, 0xf8, 0x100},
      {8, 4, 0xf8, 0x100},
      {8, 8, 0xf8, 0x100},
      {8, 16, 0xf8, 0x100},
      {7, 1, 0xf9, 0x100},
      {7, 2, 0xf9, 0x100},
      {7, 4, 0xf9, 0x100},
      {7, 8, 0xf9, 0x100},
      {7, 16, 0xf9, 0x100},

  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    uint8_t buf[0x100] __attribute__((aligned(16)));
    struct allo_fixed_bump b;
    enum allo_status status = allo_fixed_bump_init(&b, buf, 0x100);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                  "allocator initialization should succeed");
    b.cursor -= tests[i].starting_offset;
    allo__assert_fixed_bump(&b);

    void *dest = NULL;
    status = allo_fixed_bump_alloc(&dest, &b, tests[i].size, tests[i].align);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(
        dest, b.cursor,
        "dest should be at the current cursor after allocation");
    TEST_ASSERT_TRUE_MESSAGE(
        (b.end - tests[i].expected_offset) % tests[i].align == 0,
        "expected cursor position should be aligned");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end - tests[i].expected_offset, b.cursor,
                                  "cursor should be at the expected offset");
  }
}

void test_alloc_oom(void) {
  struct {
    size_t size;
    size_t align;
    uintptr_t offset;
  } tests[] = {
      // single byte; cursor at the start
      {1, 1, 0x100},
      {1, 2, 0x100},
      {1, 4, 0x100},
      {1, 8, 0x100},
      {1, 16, 0x100},

      // 8 bytes; cursor at the start
      {8, 1, 0x100},
      {8, 2, 0x100},
      {8, 4, 0x100},
      {8, 8, 0x100},
      {8, 16, 0x100},

      // 8 bytes; cursor after the start + insufficient space
      {8, 1, 0xfa},
      {8, 2, 0xfa},
      {8, 4, 0xfa},
      {8, 8, 0xfa},
      {8, 16, 0xfa},

      // 7 bytes (odd + not power of 2); cursor at the start
      {7, 1, 0x100},
      {7, 2, 0x100},
      {7, 4, 0x100},
      {7, 8, 0x100},
      {7, 16, 0x100},

      // 7 bytes; cursor after the start + insufficient space
      {7, 1, 0xfe},
      {7, 2, 0xfe},
      {7, 4, 0xfe},
      {7, 8, 0xfe},
      {7, 16, 0xfe},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    uint8_t buf[0x100] __attribute__((aligned(16)));
    struct allo_fixed_bump b;
    enum allo_status status = allo_fixed_bump_init(&b, buf, 0x100);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                  "allocator initialization should succeed");
    b.cursor -= tests[i].offset;
    allo__assert_fixed_bump(&b);

    void *dest = NULL;
    status = allo_fixed_bump_alloc(&dest, &b, tests[i].size, tests[i].align);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OOM, status,
                                  "allocation should fail due to OOM");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(
        b.end - tests[i].offset, b.cursor,
        "cursor should remain at its original position");
    allo__assert_fixed_bump(&b);
  }
}

void test_reset(void) {
  struct {
    uintptr_t offset;
  } tests[] = {
      // cursor already at the end
      {0x0},
      // cursor at the middle
      {0x2},
      // cursor at the start
      {0x4},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    uint8_t buf[0x4] __attribute__((aligned(4)));
    struct allo_fixed_bump b;
    enum allo_status status = allo_fixed_bump_init(&b, buf, 0x4);
    TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                  "allocator initialization should succeed");
    b.cursor -= tests[i].offset;
    allo__assert_fixed_bump(&b);

    allo_fixed_bump_reset(&b);
    allo__assert_fixed_bump(&b);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end, b.cursor,
                                  "cursor should reset to the end");
  }
}

void test_sequential(void) {
  uint8_t buf[0x100] __attribute__((aligned(128)));
  struct allo_fixed_bump b;
  enum allo_status status = allo_fixed_bump_init(&b, buf, 0x100);
  allo__assert_fixed_bump(&b);

  void *dest = NULL;

  status = allo_fixed_bump_alloc(&dest, &b, 1, 1);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end - 1, b.cursor,
                                "cursor should shift by 1");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 1, 8);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end - 8, b.cursor,
                                "cursor should shift to an alignment of 8");
  TEST_ASSERT_TRUE_MESSAGE(b.cursor % 8 == 0, "cursor should be aligned");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 8, 8);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end - 16, b.cursor,
                                "cursor should shift to an alignment of 8");
  TEST_ASSERT_TRUE_MESSAGE(b.cursor % 8 == 0, "cursor should be aligned");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 15, 16);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end - 32, b.cursor,
                                "cursor should shift to an alignment of 16");
  TEST_ASSERT_TRUE_MESSAGE(b.cursor % 16 == 0, "cursor should be aligned");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 1, 128);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end - 128, b.cursor,
                                "cursor should shift to an alignment of 128");
  TEST_ASSERT_TRUE_MESSAGE(b.cursor % 128 == 0, "cursor should be aligned");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 128, 128);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(b.start, b.cursor,
                                "cursor should shift to an alignment of 128 "
                                "and at the start of the memory range");
  TEST_ASSERT_TRUE_MESSAGE(b.cursor % 128 == 0, "cursor should be aligned");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 1, 1);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OOM, status,
                                "allocation should fail due to OOM");
  allo__assert_fixed_bump(&b);

  allo_fixed_bump_reset(&b);
  allo__assert_fixed_bump(&b);
  TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end, b.cursor, "cursor should be reset");

  status = allo_fixed_bump_alloc(&dest, &b, 1, 1);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(b.end - 1, b.cursor,
                                "allocatoun should shift by 1");
}

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_init);
  RUN_TEST(test_alloc_first_alloc);
  RUN_TEST(test_alloc_subsequent_allocs);
  RUN_TEST(test_alloc_oom);
  RUN_TEST(test_reset);
  RUN_TEST(test_sequential);
  return UNITY_END();
}
