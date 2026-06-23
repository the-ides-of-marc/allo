#include "allo/internal/allo_common.h"
#include "allo/internal/allo_math.h"
#include <stddef.h>
#include <stdint.h>
#include <unity.h>
#include <unity_internals.h>

void setUp(void) {}
void tearDown(void) {}

void test_allo_math_popcount_uint8(void) {
  typedef struct {
    uint8_t n;
    uint8_t expected;
  } testcase;

  testcase testcases[] = {{0x00, 0}, {0x01, 1}, {0x02, 1},  {0x03, 2},
                          {0x04, 1}, {0x05, 2}, {0x06, 2},  {0x07, 3},
                          {0x08, 1}, {0x09, 2}, {0x0a, 2},  {0x0b, 3},
                          {0x0c, 2}, {0x0d, 3}, {0x0e, 3},  {0x0f, 4},
                          {0xf0, 4}, {0xab, 5}, {0x01e, 4}, {0xff, 8}};

  for (size_t i = 0; i < ALLO_ARR_LEN(testcases); ++i) {
    uint8_t popcount = allo_math_popcount_uint8(testcases[i].n);
    if (testcases[i].expected != popcount) {
      enum { BUFSIZE = 1 << 6 };
      char buf[BUFSIZE];
      snprintf(buf, BUFSIZE, "n=%.02x expected=%u actual=%u", testcases[i].n,
               testcases[i].expected, popcount);
      TEST_FAIL_MESSAGE(buf);
    }
  }
}

void test_allo_math_popcount_uint16(void) {
  typedef struct {
    uint16_t n;
    uint8_t expected;
  } testcase;

  testcase testcases[] = {{0x0000, 0}, {0x0001, 1},  {0x0002, 1}, {0x0003, 2},
                          {0x0004, 1}, {0x0005, 2},  {0x0006, 2}, {0x0007, 3},
                          {0x0008, 1}, {0x0009, 2},  {0x000a, 2}, {0x000b, 3},
                          {0x000c, 2}, {0x000d, 3},  {0x000e, 3}, {0x000f, 4},
                          {0x1234, 5}, {0xabcd, 10}, {0x020b, 4}, {0xffff, 16}};

  for (size_t i = 0; i < ALLO_ARR_LEN(testcases); ++i) {
    uint8_t popcount = allo_math_popcount_uint16(testcases[i].n);
    if (testcases[i].expected != popcount) {
      enum { BUFSIZE = 1 << 6 };
      char buf[BUFSIZE];
      snprintf(buf, BUFSIZE, "n=%.04x expected=%u actual=%u", testcases[i].n,
               testcases[i].expected, popcount);
      TEST_FAIL_MESSAGE(buf);
    }
  }
}

void test_allo_math_popcount_uint32(void) {
  typedef struct {
    uint32_t n;
    uint8_t expected;
  } testcase;

  testcase testcases[] = {
      {0x00000000, 0}, {0x00000001, 1},  {0x00000002, 1}, {0x00000003, 2},
      {0x00000004, 1}, {0x00000005, 2},  {0x00000006, 2}, {0x00000007, 3},
      {0x00000008, 1}, {0x00000009, 2},  {0x0000000a, 2}, {0x0000000b, 3},
      {0x0000000c, 2}, {0x0000000d, 3},  {0x0000000e, 3}, {0x0000000f, 4},
      {0x10203040, 5}, {0xa0b0c0d0, 10}, {0x0000020b, 4}, {0xffffffff, 32}};

  for (size_t i = 0; i < ALLO_ARR_LEN(testcases); ++i) {
    uint8_t popcount = allo_math_popcount_uint32(testcases[i].n);
    if (testcases[i].expected != popcount) {
      enum { BUFSIZE = 1 << 6 };
      char buf[BUFSIZE];
      snprintf(buf, BUFSIZE, "n=%.08x expected=%u actual=%u", testcases[i].n,
               testcases[i].expected, popcount);
      TEST_FAIL_MESSAGE(buf);
    }
  }
}

void test_allo_math_popcount_uint64(void) {
  typedef struct {
    uint64_t n;
    uint8_t expected;
  } testcase;

  testcase testcases[] = {{0x0000000000000000, 0},  {0x0000000000000001, 1},
                          {0x0000000000000002, 1},  {0x0000000000000003, 2},
                          {0x0000000000000004, 1},  {0x0000000000000005, 2},
                          {0x0000000000000006, 2},  {0x0000000000000007, 3},
                          {0x0000000000000008, 1},  {0x0000000000000009, 2},
                          {0x000000000000000a, 2},  {0x000000000000000b, 3},
                          {0x000000000000000c, 2},  {0x000000000000000d, 3},
                          {0x000000000000000e, 3},  {0x000000000000000f, 4},
                          {0x1020304050607080, 13}, {0x90a0b0c0d0e0f000, 19},
                          {0xdead00000000beef, 24}, {0xffffffffffffffff, 64}};

  for (size_t i = 0; i < ALLO_ARR_LEN(testcases); ++i) {
    uint8_t popcount = allo_math_popcount_uint64(testcases[i].n);
    if (testcases[i].expected != popcount) {
      enum { BUFSIZE = 1 << 6 };
      char buf[BUFSIZE];
      snprintf(buf, BUFSIZE, "n=%lx expected=%u actual=%u", testcases[i].n,
               testcases[i].expected, popcount);
      TEST_FAIL_MESSAGE(buf);
    }
  }
}

void test_allo_math_ctz_size_t(void) {
  typedef struct {
    size_t n;
    uint8_t expected;
  } testcase;
  testcase testcases[] = {
      {0x0, sizeof(size_t)},
      {0x1, 0},
      {0x2, 1},
      {0x3, 0},
      {0x4, 2},
      {0x5, 0},
      {0x6, 1},
      {0x7, 0},
      {0x8, 3},
      {0x9, 0},
      {0xa, 1},
  };
  for (size_t i = 0; i < ALLO_ARR_LEN(testcases); ++i) {
    uint8_t zeroes = allo_math_ctz_size_t(testcases[i].n);
    if (testcases[i].expected != zeroes) {
      enum { BUFSIZE = 1 << 6 };
      char buf[BUFSIZE];
      snprintf(buf, BUFSIZE, "n=%zu expected=%u actual=%u", testcases[i].n,
               testcases[i].expected, zeroes);
      TEST_FAIL_MESSAGE(buf);
    }
  }
}

void test_allo_math_is_pow2(void) {
  TEST_ASSERT_FALSE_MESSAGE(allo_math_is_pow2(0), "zero is not a power of 2");

  enum { BUFSIZE = 1 << 8 };
  char buffer[BUFSIZE];

  for (size_t i = 1; i != 0; i <<= 1) {
    snprintf(buffer, BUFSIZE, "i=%zu is a power of 2", i);
    TEST_ASSERT_TRUE_MESSAGE(allo_math_is_pow2(i), buffer);
  }

  size_t not_pow2[] = {3, 5, 7, 9, 11, 13, 14, 15, 17, 31, 33, 63, 65};

  for (size_t i = 0; i < ALLO_ARR_LEN(not_pow2); ++i) {
    snprintf(buffer, BUFSIZE, "i=%zu is not a power of 2", not_pow2[i]);
    TEST_ASSERT_FALSE_MESSAGE(allo_math_is_pow2(not_pow2[i]), buffer);
  }
}

void test_allo_math_is_aligned(void) {

  typedef struct {
    uintptr_t mem;
    size_t align;
    bool expected;
  } testcase;

  testcase testcases[] = {
      {0x0, 0x1, true}, {0x1, 0x1, true},  {0x2, 0x1, true},  {0x3, 0x1, true},
      {0x4, 0x1, true}, {0x5, 0x1, true},  {0x6, 0x1, true},  {0x7, 0x1, true},
      {0x8, 0x1, true}, {0x9, 0x1, true},  {0xa, 0x1, true},  {0xb, 0x1, true},
      {0xc, 0x1, true}, {0xd, 0x1, true},  {0xe, 0x1, true},  {0xf, 0x1, true},

      {0x0, 0x2, true}, {0x1, 0x2, false}, {0x2, 0x2, true},  {0x3, 0x2, false},
      {0x4, 0x2, true}, {0x5, 0x2, false}, {0x6, 0x2, true},  {0x7, 0x2, false},
      {0x8, 0x2, true}, {0x9, 0x2, false}, {0xa, 0x2, true},  {0xb, 0x2, false},
      {0xc, 0x2, true}, {0xd, 0x2, false}, {0xe, 0x2, true},  {0xf, 0x2, false},

      {0x0, 0x4, true}, {0x1, 0x4, false}, {0x2, 0x4, false}, {0x3, 0x4, false},
      {0x4, 0x4, true}, {0x5, 0x4, false}, {0x6, 0x4, false}, {0x7, 0x4, false},
      {0x8, 0x4, true}, {0x9, 0x4, false}, {0xa, 0x4, false}, {0xb, 0x4, false},
      {0xc, 0x4, true}, {0xd, 0x4, false}, {0xe, 0x4, false}, {0xf, 0x4, false},
  };

  enum { BUFSIZE = 1 << 8 };
  char buffer[BUFSIZE] = {0};

  for (size_t i = 0; i < ALLO_ARR_LEN(testcases); ++i) {
    bool is_aligned =
        allo_math_is_aligned(testcases[i].mem, testcases[i].align);
    snprintf(buffer, BUFSIZE, "expected=%d actual=%d mem=0x%lx",
             testcases[i].expected, is_aligned, testcases[i].mem);
    TEST_ASSERT_EQUAL_MESSAGE(testcases[i].expected, is_aligned, buffer);
  }
}

void test_allo_math_align_up(void) {
  typedef struct {
    uintptr_t n;
    size_t align;
    uintptr_t expected;
  } testcase;

  testcase testcases[] = {
      {0x0, 0x1, 0x0}, {0x1, 0x1, 0x1},  {0x2, 0x1, 0x2},  {0x3, 0x1, 0x3},
      {0x4, 0x1, 0x4}, {0x5, 0x1, 0x5},  {0x6, 0x1, 0x6},  {0x7, 0x1, 0x7},
      {0x8, 0x1, 0x8}, {0x9, 0x1, 0x9},  {0xa, 0x1, 0xa},  {0xb, 0x1, 0xb},
      {0xc, 0x1, 0xc}, {0xd, 0x1, 0xd},  {0xe, 0x1, 0xe},  {0xf, 0x1, 0xf},

      {0x0, 0x2, 0x0}, {0x1, 0x2, 0x2},  {0x2, 0x2, 0x2},  {0x3, 0x2, 0x4},
      {0x4, 0x2, 0x4}, {0x5, 0x2, 0x6},  {0x6, 0x2, 0x6},  {0x7, 0x2, 0x8},
      {0x8, 0x2, 0x8}, {0x9, 0x2, 0xa},  {0xa, 0x2, 0xa},  {0xb, 0x2, 0xc},
      {0xc, 0x2, 0xc}, {0xd, 0x2, 0xe},  {0xe, 0x2, 0xe},  {0xf, 0x2, 0x10},

      {0x0, 0x4, 0x0}, {0x1, 0x4, 0x4},  {0x2, 0x4, 0x4},  {0x3, 0x4, 0x4},
      {0x4, 0x4, 0x4}, {0x5, 0x4, 0x8},  {0x6, 0x4, 0x8},  {0x7, 0x4, 0x8},
      {0x8, 0x4, 0x8}, {0x9, 0x4, 0xc},  {0xa, 0x4, 0xc},  {0xb, 0x4, 0xc},
      {0xc, 0x4, 0xc}, {0xd, 0x4, 0x10}, {0xe, 0x4, 0x10}, {0xf, 0x4, 0x10},

  };

  enum { BUFSIZE = 1 << 8 };
  char buffer[BUFSIZE] = {0};

  for (size_t i = 0; i < ALLO_ARR_LEN(testcases); ++i) {
    uintptr_t n = allo_math_align_up(testcases[i].n, testcases[i].align);
    snprintf(buffer, BUFSIZE, "expected=0x%lx actual=0x%lx n=0x%lx align=%zu",
             n, testcases[i].expected, testcases[i].n, testcases[i].align);
    TEST_ASSERT_EQUAL_MESSAGE(testcases[i].expected, n, buffer);
  }
}

void test_allo_math_align_down(void) {
  typedef struct {
    uintptr_t n;
    size_t align;
    uintptr_t expected;
  } testcase;

  testcase testcases[] = {
      {0x0, 0x1, 0x0}, {0x1, 0x1, 0x1}, {0x2, 0x1, 0x2}, {0x3, 0x1, 0x3},
      {0x4, 0x1, 0x4}, {0x5, 0x1, 0x5}, {0x6, 0x1, 0x6}, {0x7, 0x1, 0x7},
      {0x8, 0x1, 0x8}, {0x9, 0x1, 0x9}, {0xa, 0x1, 0xa}, {0xb, 0x1, 0xb},
      {0xc, 0x1, 0xc}, {0xd, 0x1, 0xd}, {0xe, 0x1, 0xe}, {0xf, 0x1, 0xf},

      {0x0, 0x2, 0x0}, {0x1, 0x2, 0x0}, {0x2, 0x2, 0x2}, {0x3, 0x2, 0x2},
      {0x4, 0x2, 0x4}, {0x5, 0x2, 0x4}, {0x6, 0x2, 0x6}, {0x7, 0x2, 0x6},
      {0x8, 0x2, 0x8}, {0x9, 0x2, 0x8}, {0xa, 0x2, 0xa}, {0xb, 0x2, 0xa},
      {0xc, 0x2, 0xc}, {0xd, 0x2, 0xc}, {0xe, 0x2, 0xe}, {0xf, 0x2, 0xe},

      {0x0, 0x4, 0x0}, {0x1, 0x4, 0x0}, {0x2, 0x4, 0x0}, {0x3, 0x4, 0x0},
      {0x4, 0x4, 0x4}, {0x5, 0x4, 0x4}, {0x6, 0x4, 0x4}, {0x7, 0x4, 0x4},
      {0x8, 0x4, 0x8}, {0x9, 0x4, 0x8}, {0xa, 0x4, 0x8}, {0xb, 0x4, 0x8},
      {0xc, 0x4, 0xc}, {0xd, 0x4, 0xc}, {0xe, 0x4, 0xc}, {0xf, 0x4, 0xc},

  };

  enum { BUFSIZE = 1 << 8 };
  char buffer[BUFSIZE] = {0};

  for (size_t i = 0; i < ALLO_ARR_LEN(testcases); ++i) {
    uintptr_t n = allo_math_align_down(testcases[i].n, testcases[i].align);
    snprintf(buffer, BUFSIZE, "expected=0x%lx actual=0x%lx n=0x%lx align=%zu",
             n, testcases[i].expected, testcases[i].n, testcases[i].align);
    TEST_ASSERT_EQUAL_MESSAGE(testcases[i].expected, n, buffer);
  }
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_allo_math_popcount_uint8);
  RUN_TEST(test_allo_math_popcount_uint16);
  RUN_TEST(test_allo_math_popcount_uint32);
  RUN_TEST(test_allo_math_popcount_uint64);

  RUN_TEST(test_allo_math_ctz_size_t);

  RUN_TEST(test_allo_math_is_pow2);

  RUN_TEST(test_allo_math_is_aligned);

  RUN_TEST(test_allo_math_align_up);

  RUN_TEST(test_allo_math_align_down);

  return UNITY_END();
}
