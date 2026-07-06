# Allo

A C99 Library providing allocators that manage fixed sized contiguous memory regions.

The allocators do not reallocate or deallocate any memory and strictly manage the memory
it is given upon initialization.

Allocators
- allo_bump
- allo_stack
- allo_pool

All allocators can be wrapped in an `allo_allocator` type, a fat pointer, which provides
common allocator operations.
Each allocator provides its own vtable found in its own source file.
> Not all allocators support every operation provided by `allo_allocator`.

See [BUILD.md](BUILD.md) for building the library from source.

## Usage

All operations that can succeed or fail return `allo_status` which is a enumeration of statuses
that spans the entire library's usage.

The library indicate errors either via `allo_status` or asserts, where `allo_status` can be handled
programmatically while asserts crash the program. Due to the terminating nature of asserts,
they are disabled by default and can be toggled on by defining `ALLO_ENABLE_ASSERT`.

This is done to allow operations such as allocations and frees to be fast, such as hot loops.
When verification of correctness is needed, then enabling asserts provides that layer of checking.

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
    allo_status status = allo_pool_init(&pool_allocator, buf, BUF_SIZE, CHUNK_SIZE, ALIGN);
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

Here is an example of using a bump allocator, `allo_bump`,
through the generic `allo_allocator` interface.

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
    enum { BUF_SIZE = 1 << 10 };
    uint8_t buf[BUF_SIZE];

    // Define and initialize the allocator.
    allo_bump bump_allocator;
    allo_status status = allo_bump_init(&bump_allocator, buf, BUF_SIZE);
    if (status != ALLO_OK) {
        fprintf(stderr, "error initializing allocator: %s\n", allo_status_str(status));
        return 1;
    }
    allo_allocator allocator = allo_allocator_from_bump(&bump_allocator);

    // Allocate memory for struct.
    void *dest = NULL;
    status = allo_alloc(&dest, allocator, sizeof(struct message), alignof(struct message));

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

## Allocators

List of allocators
- `allo_bump`
- `allo_stack`
- `allo_pool`

### `allo_bump`

This is the bump allocator, allocating memory downwards in a memory region.

On initialization, `allo_bump_init`, the bump allocator is quite simple just takes in a contiguous
memory region to manage, with a cursor pointing to the end (top) of the memory region.

On allocation, `allo_bump_alloc` the bump allocator takes in the required size of the allocation,
and the required alignment, and shifts the cursor down in memory to the a suitable address
for allocation and writes the address to the `dest` argument.

Typically bump allocators do not freeing individual allocated memory, only freeing all the 
allocated memory at once.
Freeing all allocated memory at once can be done via `allo_bump_reset`
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

On initialization, `allo_stack_init`, the stack allocation similarly takes in a contiguous memory
region to manage, with a cursor pointing to the end of the memory region.

On allocation, `allo_stack_alloc`, shifts the cursor down similarly to accomodate the size and
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

