# Allo

A C99 Library providing allocators that manage fixed sized contiguous memory regions.

The allocators do not reallocate or deallocate any memory and strictly manage the memory
they are given upon initialization.
See [Allocators](#allocators) for a list of supported allocators.

See [BUILD.md](BUILD.md) for building the library from source.

## Usage

All operations that can succeed or fail return `allo_status` which is an enumeration of statuses
that spans the entire library's usage.

The library indicates errors either via `allo_status` or asserts, where `allo_status` can be handled
programmatically while asserts crash the program.

### Configuration

To enable asserts, you can define `ALLO_ENABLE_ASSERT`.

> [!NOTE]
> Library asserts use `assert` from `<assert.h>` when enabled, and is a no op when disabled.

## Allocators

List of allocators
- `allo_bump`
- `allo_stack`
- `allo_pool`

Allocators often provide similar operations with the following patterns:
- `allo_*_init`:  Initializes the allocator with the relevant parameters.
- `allo_*_alloc`/`allo_*_alloc_unsafe`: Allocates memory to the allocator.
  The unsafe variant omits parameter validation performance penalty at the
  risk of undefined behaviour on invalid values.
  The parameters taken in for allocation varies per allocator's design.
- `allo_*_free`/`allo_*_free_unsafe`: Frees memory from the allocator.
  The unsafe variant omits parameter validation performance penalty at the 
  risk of undefined behaviour on invalid values.
- `allo_*_free_all`: Frees all allocated memory from the allocator.

> [!NOTE]
> The unsafe variants, `allo_*_alloc_unsafe` and `allo_*_free_unsafe`, are useful when allocating/freeing
> in a hot loop where safety can be guaranteed by the caller before entering a hot loop of allocation/freeing.

Allocators may also implement other functions that are specific to their implementation/design.

### `allo_bump`

This is the bump allocator, allocating memory downwards in a memory region.

On initialization, `allo_bump_init`, the bump allocator takes in a contiguous
memory region to manage, with a cursor pointing to the end (top) of the memory region.

On allocation, `allo_bump_alloc` the bump allocator takes in the required size of the allocation,
and the required alignment, and shifts the cursor down in memory to the a suitable address
for allocation and writes the address to the `dest` argument.

Typically bump allocators do not free individual allocated memory, only freeing all the 
allocated memory at once.
Freeing all allocated memory at once can be done via `allo_bump_free_all`
However, to provide flexibility to the caller, `allo_bump_set_cursor` can be used to manually set
the cursor to a position in the allocator. With proper management of allocated addresses, the caller
can effectively free all allocations up to a specific point of time.

### `allo_stack`

This is a stack allocator, allocating memory downwards in a memory region.
The stack allocator is very similar to the bump allocator. The main difference is that it manages
allocations like a stack (at the cost of some memory overhead per allocation), allowing it to free
the latest allocation.

> As mentioned in the `allo_bump` section, it is possible to simulate a stack allocator by having
> the caller manage the allocated addresses. This eliminates the overhead incurred in the stack
> allocator.

On initialization, `allo_stack_init`, the stack allocator similarly takes in a contiguous memory
region to manage, with a cursor pointing to the end of the memory region.

On allocation, `allo_stack_alloc`, shifts the cursor down similarly to accommodate the size and
alignment requirements and writes the address to the `dest` argument. The cursor, however, is moved
down by a `sizeof(uintptr_t)` as a header that will contain the address of the previous allocation
(the end of the memory region if no earlier allocation exists).

On freeing the latest allocation, `allo_stack_free`, the cursor is shifted back to the header of the
previous allocation.

Freeing all allocations is also possible with `allo_stack_free_all`.

### `allo_pool`

This is a pool allocator, managing a set of fixed sized chunks of fixed size and alignment in a
contiguous memory region.

On initialization, `allo_pool_init`, the memory region is validated to be of a compatible alignment
and managing at least one chunk. If successful, an intrusive free list is initialized where chunks
are allocated in sequence from the start to the end of the memory region.

On allocation, `allo_pool_alloc`, the pool allocates an address from the free list and writes the
address to the `dest` argument. The free list is then updated to point to the next free chunk.

On freeing a chunk, `allo_pool_free`, the pool address takes back the chunk's address and adds it
to the front of the free list.

Upon freeing all chunks, `allo_pool_free_all`, the pool simply resets the entire free list across
the entire memory region, thus any dangling pointers from previous allocations must not be used.

## Examples

Here is an example of using a pool allocator, `allo_pool`.

```c
#include "allo/allo.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

struct message {
    uint8_t header[128];
    uint8_t payload[4096];
};

int main(void) {
    // Define the memory region for the allocator to manage.
    enum {
        CHUNK_COUNT = 1 << 8,
        BUF_SIZE = CHUNK_COUNT * sizeof(struct message),
        ALIGN = alignof(struct message),
    };
    uint8_t buf[BUF_SIZE] __attribute__(aligned(ALIGN));

    // Define and initialize the allocator.
    allo_pool pool_allocator;
    size_t chunk_size = sizeof(struct message);
    allo_status status = allo_pool_init(&pool_allocator, buf, BUF_SIZE, chunk_size, ALIGN);
    if (status != ALLO_OK) {
        fprintf(stderr, "error initializing allocator: %s\n", allo_status_str(status));
        return 1;
    }

    // Allocate memory for struct.
    void *dest = NULL;
    status = allo_pool_alloc(&dest, &pool_allocator);

    // Handle when allocation is not successful.
    if (status != ALLO_OK) {
        // Handle when there is no more space in the allocator.
        if (status == ALLO_OOM) {
            fprintf(stderr, "insufficient space in allocator: %s\n", allo_status_str(status));
            return 1;
        }
        fprintf(stderr, "error allocating memory: %s\n", allo_status_str(status));
        return 1;
    }

    // Use allocated memory.
    struct message* msg = dest;

    return 0;
}
```
