#include "allo/allo.h"
#include "allo/internal/math.h"
#include "allo_test/allo_test.h"
#include "unity.h"
#include <stddef.h>
#include <stdint.h>

void setUp(void) {}

void tearDown(void) {}

static const uintptr_t MOCK_BASE_ADDR = 0x1000;

struct mock_state {
  size_t alloc_calls;
  size_t free_calls;
  size_t free_all_calls;
  void *ptr_last_allocated;
};

static void mock_state_init(struct mock_state *m) {
  m->alloc_calls = 0;
  m->free_calls = 0;
  m->free_all_calls = 0;
  m->ptr_last_allocated = NULL;
}

static allo_status mock_alloc(void *restrict *restrict dest, void *restrict ctx,
                              size_t size, size_t align) {
  struct mock_state *state = ctx;
  ++state->alloc_calls;
  *dest = (void *)(MOCK_BASE_ADDR + size + align);
  state->ptr_last_allocated = *dest;
  return ALLO_OK;
}

static allo_status mock_free(void *ctx, void *ptr) {
  struct mock_state *state = ctx;
  ++state->free_calls;
  (void)ptr;
  return ALLO_OK;
}

static allo_status mock_free_all(void *ctx) {
  struct mock_state *state = ctx;
  ++state->free_all_calls;
  return ALLO_OK;
}

static allo_allocator_vtable mock_vtable = {
    .alloc = &mock_alloc,
    .free = &mock_free,
    .free_all = &mock_free_all,
};

void test_alloc_and_free(void) {
  struct mock_state state = {0};
  mock_state_init(&state);

  allo_allocator allocator = {
      .allocator = &state,
      .vtable = &mock_vtable,
  };

  void *dest = NULL;
  allo_status status = allo_alloc(&dest, allocator, 64, 16);
  ALLO_TEST_ASSERT_STATUS_MSG(ALLO_OK, status, "allocation should succeed");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      MOCK_BASE_ADDR + 64 + 16, dest,
      "ptr should point to mocked allocation of base addr + size + align");
  TEST_ASSERT_EQUAL_size_t_MESSAGE(1, state.alloc_calls,
                                   "alloc counter should be incremented by 1");
  TEST_ASSERT_TRUE_MESSAGE(allo_math_is_aligned((uintptr_t)dest, 16),
                           "allocated address should be aligned");

  allo_free(allocator, dest);

  TEST_ASSERT_EQUAL_size_t_MESSAGE(1, state.free_calls,
                                   "free counter should be incremented by 1");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      dest, state.ptr_last_allocated,
      "freed pointer should be correctly tracked by mock free");
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_alloc_and_free);
  return UNITY_END();
}
