#include "allo/internal/common.h"
#include <unity.h>
#include <unity_internals.h>

void setUp(void) {}
void tearDown(void) {}

void test_allo_min(void) {
  TEST_ASSERT_EQUAL(0, ALLO_MIN(0, 1));
  TEST_ASSERT_EQUAL(-1, ALLO_MIN(-1, 0));
}

void test_allo_max(void) {
  TEST_ASSERT_EQUAL(1, ALLO_MAX(0, 1));
  TEST_ASSERT_EQUAL(0, ALLO_MAX(-1, 0));
}

void test_allo_arr_len(void) {
  uint32_t buf[1 << 10];
  TEST_ASSERT_EQUAL(1 << 10, ALLO_ARR_LEN(buf));
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_allo_min);
  RUN_TEST(test_allo_max);

  RUN_TEST(test_allo_arr_len);

  return UNITY_END();
}
