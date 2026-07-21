#ifndef ALLO_TEST_IO_H
#define ALLO_TEST_IO_H

#include <stddef.h>
void allo_test_snprintf(char *restrict buffer, size_t maxlen, size_t *offset,
                        const char *restrict format, ...)
    __attribute__((format(printf, 4, 5)));

#endif // !ALLO_TEST_IO_H
