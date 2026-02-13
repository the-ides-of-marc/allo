#include "unity_internals.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define ALLO_IMPLEMENTATION
#include "allo.h"

#include "unity.h"

static const uintptr_t MOCK_BASE_ADDR = 0x1000;

struct mock_state {
  size_t alloc_calls;
  size_t free_calls;
  void *ptr_last_allocated;
};

static enum allo_status mock_alloc(void **dest, void *ctx, size_t size,
                                   size_t align) {
  struct mock_state *state = ctx;
  ++state->alloc_calls;
  *dest = (void *)(MOCK_BASE_ADDR + size + align);
  state->ptr_last_allocated = *dest;
  return ALLO_OK;
}

static void mock_free(void *ctx, void *ptr) {
  struct mock_state *state = ctx;
  ++state->free_calls;
  (void)ptr;
}

static struct allo__allocator_vtable mock_vtable = {
    .alloc = &mock_alloc,
    .free = &mock_free,
};

void setUp(void) {}

void tearDown(void) {}

void test_alloc_and_free(void) {
  struct mock_state state = {0};
  struct allo_allocator allocator = {
      .allo__ptr = &state,
      .allo__vtable = &mock_vtable,
  };

  void *dest = NULL;
  enum allo_status status = allo_alloc(&dest, allocator, 64, 16);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should success");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      MOCK_BASE_ADDR + 64 + 16, dest,
      "ptr should point to mocked allocation of base addr + size + align");
  TEST_ASSERT_EQUAL_size_t_MESSAGE(1, state.alloc_calls,
                                   "alloc counter should be incremented by 1");

  allo_free(allocator, dest);

  TEST_ASSERT_EQUAL_size_t_MESSAGE(1, state.free_calls,
                                   "free counter should be incremented by 1");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      dest, state.ptr_last_allocated,
      "freed pointer should be correctly tracked by mock free");
}

void test_allocator_from_fixed_bump(void) {
  uint8_t buf[0x10] __attribute__((aligned(16)));
  struct allo_fixed_bump b;
  enum allo_status status = allo_fixed_bump_init(&b, buf, 0x10);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status,
                                "allocation initialization should succeed");
  struct allo_allocator a = allo_allocator_from_fixed_bump(&b);
  TEST_ASSERT_EQUAL_PTR_MESSAGE(&b, a.allo__ptr,
                                "ptr should point to underlying allocator");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      &allo__fixed_bump_vtable, a.allo__vtable,
      "vtable should point to bump allocator's vtable");
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_alloc_and_free);
  RUN_TEST(test_allocator_from_fixed_bump);
  return UNITY_END();
}
