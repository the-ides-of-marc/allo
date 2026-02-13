#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

#define ALLO_IMPLEMENTATION
#include "allo.h"

void test_init(void) {
  uint8_t buf[0x100];
  struct allo_bump b;
  enum allo_status status = allo_bump_init(&b, buf, 0x100);

  allo__assert_bump(&b);
  assert(status == ALLO_OK);
  assert(b.start == (uintptr_t)buf);
  assert(b.end == (uintptr_t)(buf + 0x100));
  assert(b.cursor == b.end);
}

void test_alloc(void) {
  alignas(128) uint8_t buf[0x100];
  struct allo_bump b;
  enum allo_status status = allo_bump_init(&b, buf, 0x100);
  allo__assert_bump(&b);

  void *dest;

  status = allo_bump_alloc(&dest, &b, 1, 1);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == (uintptr_t)(buf + 0xff) && "cursor should shift by 1");
  allo__assert_bump(&b);

  status = allo_bump_alloc(&dest, &b, 1, 8);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == (uintptr_t)(buf + 0xf8) && b.cursor % 8 == 0 &&
         "cursor should shift to an alignment of 8");
  allo__assert_bump(&b);

  status = allo_bump_alloc(&dest, &b, 8, 8);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == (uintptr_t)(buf + 0xf0) &&
         "cursor should shift to an alignment of 8");
  allo__assert_bump(&b);

  status = allo_bump_alloc(&dest, &b, 15, 16);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == (uintptr_t)(buf + 0xe0) &&
         "cursor should shift to an alignment of 16");
  allo__assert_bump(&b);

  status = allo_bump_alloc(&dest, &b, 1, 128);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == (uintptr_t)(buf + 0x80) &&
         "cursor should shift to an alignment of 128");
  allo__assert_bump(&b);

  status = allo_bump_alloc(&dest, &b, 128, 128);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == (uintptr_t)buf &&
         "cursor should shift to an alignment of 128");
  allo__assert_bump(&b);
}

void test_alloc_oom(void) {
  uint8_t buf[0x4];
  struct allo_bump b;
  enum allo_status status = allo_bump_init(&b, buf, 0x4);
  allo__assert_bump(&b);

  void *dest;
  status = allo_bump_alloc(&dest, &b, 8, 8);
  assert(status == ALLO_OOM &&
         "allocation is too large and should fail with OOM");
  assert(b.cursor == (uintptr_t)(buf + 0x4) &&
         "cursor should remain at the end");
  allo__assert_bump(&b);

  status = allo_bump_alloc(&dest, &b, 2, 2);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == (uintptr_t)(buf + 0x2) &&
         "cursor should shift to an alignment of 2");
  allo__assert_bump(&b);

  status = allo_bump_alloc(&dest, &b, 8, 8);
  assert(status == ALLO_OOM &&
         "allocation is too large and should fail with OOM");
  assert(b.cursor == (uintptr_t)(buf + 0x2) &&
         "cursor should remain at its original position");
  allo__assert_bump(&b);

  status = allo_bump_alloc(&dest, &b, 2, 2);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == (uintptr_t)buf &&
         "cursor should shift to an alignment of 2");
  allo__assert_bump(&b);

  status = allo_bump_alloc(&dest, &b, 8, 8);
  assert(status == ALLO_OOM &&
         "allocation is too large and should fail with OOM");
  assert(b.cursor == (uintptr_t)buf &&
         "cursor should remain at its original position");
  allo__assert_bump(&b);
}

void test_reset(void) {
  uint8_t buf[0x4];
  struct allo_bump b;
  enum allo_status status = allo_bump_init(&b, buf, 0x4);
  allo__assert_bump(&b);

  allo_bump_reset(&b);
  assert(b.cursor == (uintptr_t)buf + 0x4 &&
         "cursor should be at the end of memory range");
  allo__assert_bump(&b);

  void *dest;
  status = allo_bump_alloc(&dest, &b, 2, 2);
  assert(status == ALLO_OK && "allocation should succeed");
  assert(b.cursor == (uintptr_t)(buf + 0x2) &&
         "cursor should shift to an alignment of 2");
  allo__assert_bump(&b);

  allo_bump_reset(&b);
  assert(b.cursor == (uintptr_t)buf + 0x4 &&
         "cursor should be at the end of memory range");
  allo__assert_bump(&b);
}

int main(void) {
  test_init();
  test_alloc();
  test_alloc_oom();
  test_reset();
}
