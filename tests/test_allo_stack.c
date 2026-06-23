#include "allo/allo_stack.h"
#include "allo/allo_status.h"
#include "test_utils.h"
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
  TEST_ASSERT_EQUAL_MESSAGE((uintptr_t)buf, s.cursor,
                            "cursor should be at the beginning of the buffer");
  TEST_ASSERT_EQUAL_MESSAGE(0, s.last_alloc_size,
                            "last alloc size should be empty");
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_allo_stack_init);

  return UNITY_END();
}
