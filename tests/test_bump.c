#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

#define ALLO_IMPLEMENTATION
#include "allo.h"

void test_init(void) {
  uint8_t buf[0x100];
  struct allo_fixed_bump b;
  enum allo_status status = allo_fixed_bump_init(&b, buf, 0x100);
  assert(status == ALLO_OK && "allocator initialization should succeed");
  allo__assert_fixed_bump(&b);

  assert(b.start == (uintptr_t)buf);
  assert(b.end == (uintptr_t)(buf + 0x100));
  assert(b.cursor == b.end);
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
    assert(status == ALLO_OK && "allocator initialization should succeed");
    allo__assert_fixed_bump(&b);

    void *dest;
    status = allo_fixed_bump_alloc(&dest, &b, tests[i].size, tests[i].align);
    assert(status == ALLO_OK && "allocation should succeed");
    assert((uintptr_t)dest == b.cursor && "dest should be a updated cursor");
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
    alignas(16) uint8_t buf[0x100];
    struct allo_fixed_bump b;
    enum allo_status status = allo_fixed_bump_init(&b, buf, 0x100);
    assert(status == ALLO_OK && "allocator initialization should succeed");
    b.cursor -= tests[i].starting_offset;
    allo__assert_fixed_bump(&b);

    void *dest;
    status = allo_fixed_bump_alloc(&dest, &b, tests[i].size, tests[i].align);
    assert(status == ALLO_OK);
    assert((uintptr_t)dest == b.cursor && "dest should be a updated cursor");
    assert(b.cursor == b.end - tests[i].expected_offset &&
           "cursor should be at expected offset");
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
    alignas(16) uint8_t buf[0x100];
    struct allo_fixed_bump b;
    enum allo_status status = allo_fixed_bump_init(&b, buf, 0x100);
    assert(status == ALLO_OK && "allocator initialization should succeed");
    b.cursor -= tests[i].offset;
    allo__assert_fixed_bump(&b);

    void *dest;
    status = allo_fixed_bump_alloc(&dest, &b, tests[i].size, tests[i].align);
    assert(status == ALLO_OOM);
    assert(b.cursor == b.end - tests[i].offset &&
           "cursor should remain at its original position");
    allo__assert_fixed_bump(&b);
  }
}

void test_alloc(void) {
  test_alloc_first_alloc();
  test_alloc_subsequent_allocs();
  test_alloc_oom();
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
    uint8_t buf[0x4];
    struct allo_fixed_bump b;
    enum allo_status status = allo_fixed_bump_init(&b, buf, 0x4);
    assert(status == ALLO_OK && "allocator initialization should succeed");
    b.cursor -= tests[i].offset;
    allo__assert_fixed_bump(&b);

    allo_fixed_bump_reset(&b);
    allo__assert_fixed_bump(&b);
    assert(b.cursor == b.end && "cursor should reset to the end");
  }
}

void test_sequential(void) {
  alignas(128) uint8_t buf[0x100];
  struct allo_fixed_bump b;
  enum allo_status status = allo_fixed_bump_init(&b, buf, 0x100);
  allo__assert_fixed_bump(&b);

  void *dest;

  status = allo_fixed_bump_alloc(&dest, &b, 1, 1);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == b.end - 1 && "cursor should shift by 1");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 1, 8);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == b.end - 8 && b.cursor % 8 == 0 &&
         "cursor should shift to an alignment of 8");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 8, 8);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == b.end - 16 && b.cursor % 8 == 0 &&
         "cursor should shift to an alignment of 8");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 15, 16);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == b.end - 32 && b.cursor % 16 == 0 &&
         "cursor should shift to an alignment of 16");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 1, 128);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == b.end - 128 && b.cursor % 128 == 0 &&
         "cursor should shift to an alignment of 128");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 128, 128);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == b.start && b.cursor % 128 == 0 &&
         "cursor should shift to an alignment of 128");
  allo__assert_fixed_bump(&b);

  status = allo_fixed_bump_alloc(&dest, &b, 1, 1);
  assert(status == ALLO_OOM && "allocation should fail due to OOM");
  allo__assert_fixed_bump(&b);

  allo_fixed_bump_reset(&b);
  allo__assert_fixed_bump(&b);
  assert(b.cursor == b.end && "cursor should be reset");

  status = allo_fixed_bump_alloc(&dest, &b, 1, 1);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == b.end - 1 && "allocatoun should shift by 1");
}

int main(void) {
  test_init();
  test_alloc();
  test_reset();
  test_sequential();
}
