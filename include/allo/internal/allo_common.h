#ifndef ALLO_COMMON_H
#define ALLO_COMMON_H

#define ALLO_MIN(x, y) ((x) < (y) ? (x) : (y))
#define ALLO_MAX(x, y) ((x) > (y) ? (x) : (y))

#define ALLO_ARR_LEN(arr) (sizeof((arr)) / sizeof((arr)[0]))

#endif // !ALLO_COMMON_H
