#include "allo/allo_stack.h"
#include "allo/allo_status.h"
#include "allo/internal/allo_math.h"
#include "allo_test/allo_test.h"
#include <stddef.h>
#include <stdint.h>
#include <unity.h>
#include <unity_internals.h>

void setUp(void) {}
void tearDown(void) {}

void test_allo_stack_init(void) {
  enum { BUFSIZE = 1 << 10 };
  uint8_t buf[BUFSIZE];
  allo_stack s;
  allo_status status = allo_stack_init(&s, buf, BUFSIZE);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status, "init should succeed");

  allo_stack_assert(&s);
  TEST_ASSERT_EQUAL_MESSAGE((uintptr_t)buf, s.start,
                            "start should be at the beginning of the buffer");
  TEST_ASSERT_EQUAL_MESSAGE((uintptr_t)buf + BUFSIZE, s.end,
                            "end should be at the end of the buffer");
  TEST_ASSERT_EQUAL_MESSAGE((uintptr_t)s.end, s.cursor,
                            "cursor should be at the end of the buffer");
}

void test_allo_stack_alloc_no_alignment_shifting(void) {
  enum { BUFSIZE = 1 << 10 };
  uint8_t buf[BUFSIZE] __attribute__((aligned(32)));
  allo_stack s = {0};
  allo_status status = allo_stack_init(&s, buf, BUFSIZE);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "initialization should succeed");
  void *dest = NULL;
  status = allo_stack_alloc(&dest, &s, 32, 32);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "allocation should succeed");
  TEST_ASSERT_EQUAL_MESSAGE(
      (uintptr_t)dest - sizeof(uintptr_t), s.cursor,
      "allocated pointer should be the same as the cursor");
  TEST_ASSERT_EQUAL_MESSAGE(
      buf + BUFSIZE - 32 - sizeof(uintptr_t), s.cursor,
      "cursor should have moved by 32 and the uintptr header");
  TEST_ASSERT_EQUAL_MESSAGE(s.end, *(uintptr_t *)s.cursor,
                            "size of the header must match");
  TEST_ASSERT_TRUE_MESSAGE(allo_math_is_aligned((uintptr_t)dest, 32),
                           "allocated pointer should be aligned");
}

void test_allo_stack_alloc_with_alignment_shifting(void) {
  enum { BUFSIZE = 1 << 10 };
  uint8_t buf[BUFSIZE] __attribute__((aligned(8)));
  allo_stack s = {0};
  allo_status status = allo_stack_init(&s, buf, BUFSIZE);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "initialization should succeed");
  void *dest = NULL;
  status = allo_stack_alloc(&dest, &s, 1, 8);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "allocation should succeed");
  TEST_ASSERT_EQUAL_MESSAGE(
      allo_math_align_down((uintptr_t)dest - sizeof(uintptr_t),
                           ALLO_MATH_ALIGNOF(uintptr_t)),
      s.cursor, "allocated pointer should be the same as the cursor");
  TEST_ASSERT_EQUAL_MESSAGE(buf + BUFSIZE - 8 - sizeof(uintptr_t), s.cursor,
                            "cursor should have moved by 8 and the header");
  TEST_ASSERT_EQUAL_MESSAGE(s.end, *(uintptr_t *)s.cursor,
                            "size of the header must match");
  TEST_ASSERT_TRUE_MESSAGE(allo_math_is_aligned((uintptr_t)dest, 8),
                           "allocated pointer should be aligned");
}

void test_allo_stack_free_empty(void) {
  enum { BUFSIZE = 1 << 10 };
  uint8_t buf[BUFSIZE] __attribute__((aligned(8)));
  allo_stack s = {0};
  allo_status status = allo_stack_init(&s, buf, BUFSIZE);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "initialization should succeed");
  allo_stack_assert(&s);
  allo_stack_free(&s);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "free should be a no-op");
  allo_stack_assert(&s);
}

void test_allo_stack_free_not_empty(void) {
  enum { BUFSIZE = 1 << 10 };
  uint8_t buf[BUFSIZE] __attribute__((aligned(8)));
  allo_stack s = {0};
  allo_status status = allo_stack_init(&s, buf, BUFSIZE);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "initialization should succeed");

  void *dest = NULL;
  allo_stack_alloc(&dest, &s, 8, 8);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "allocation should succeed");

  allo_stack_assert(&s);
  allo_stack_free(&s);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "free should be a no-op");
  allo_stack_assert(&s);
}

void test_allo_stack_free_all(void) {
  enum { BUFSIZE = 1 << 10 };
  uint8_t buf[BUFSIZE] __attribute__((aligned(8)));
  allo_stack s = {0};
  allo_status status = allo_stack_init(&s, buf, BUFSIZE);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "initialization should succeed");

  void *dest = NULL;
  status = allo_stack_alloc(&dest, &s, 32, 8);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "allocation should succeed");
  allo_stack_alloc(&dest, &s, 32, 8);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "allocation should succeed");

  allo_stack_free_all(&s);
  TEST_UTILS_ASSERT_ALLO_STATUS_MESSAGE(ALLO_OK, status,
                                        "free all should succeed");
  TEST_ASSERT_EQUAL_MESSAGE(s.end, s.cursor, "cursor must be at the end");
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_allo_stack_init);

  RUN_TEST(test_allo_stack_alloc_no_alignment_shifting);
  RUN_TEST(test_allo_stack_alloc_with_alignment_shifting);

  RUN_TEST(test_allo_stack_free_empty);
  RUN_TEST(test_allo_stack_free_not_empty);

  RUN_TEST(test_allo_stack_free_all);

  return UNITY_END();
}
