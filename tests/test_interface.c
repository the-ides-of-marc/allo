#include "allo.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define ALLO_IMPLEMENTATION

static const uintptr_t MOCK_BASE_ADDR = 0x1000;

struct mock_state {
  size_t alloc_calls;
  size_t free_calls;
  void *ptr_last_allocated;
};

static void *mock_alloc(void *ctx, size_t size, size_t align) {
  struct mock_state *state = ctx;
  ++state->alloc_calls;
  void *addr = (void *)(MOCK_BASE_ADDR + size + align);
  state->ptr_last_allocated = addr;
  return addr;
}

static void mock_free(void *ctx, void *ptr) {
  struct mock_state *state = ctx;
  ++state->free_calls;
  (void)ptr;
}

int main(void) {
  struct mock_state state = {0};
  struct allo_allocator allocator = {
      .ctx = &state,
      .alloc = &mock_alloc,
      .free = &mock_free,
  };

  void *ptr = allo_alloc(allocator, 64, 16);
  assert(ptr == (void *)(MOCK_BASE_ADDR + 64 + 16));
  assert(state.alloc_calls == 1);

  allo_free(allocator, ptr);
  assert(state.free_calls == 1);
  assert(state.ptr_last_allocated == ptr);
}
