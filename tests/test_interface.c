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

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();

  struct mock_state state = {0};
  struct allo_allocator allocator = {
      .ctx = &state,
      .alloc = &mock_alloc,
      .free = &mock_free,
  };

  void *ptr;
  enum allo_status status = allo_alloc(&ptr, allocator, 64, 16);
  TEST_ASSERT_EQUAL_INT_MESSAGE(ALLO_OK, status, "allocation should success");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      MOCK_BASE_ADDR + 64 + 16, ptr,
      "ptr should point to mocked allocation of base addr + size + align");
  TEST_ASSERT_EQUAL_size_t_MESSAGE(1, state.alloc_calls,
                                   "alloc counter should be incremented by 1");

  allo_free(allocator, ptr);

  TEST_ASSERT_EQUAL_size_t_MESSAGE(1, state.free_calls,
                                   "free counter should be incremented by 1");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(
      ptr, state.ptr_last_allocated,
      "freed pointer should be correctly tracked by mock free");

  return UNITY_END();
}
