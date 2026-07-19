#ifndef ALLO_STATUS_H
#define ALLO_STATUS_H

// Enum representing all status codes used by the allo functions across the
// library to indicate success/error.
typedef enum allo_status {
  // Indicates success.
  ALLO_OK = 0,
  // Indicates out of memory.
  ALLO_OOM,
  // Indicates access out of bounds.
  ALLO_ERR_OUT_OF_BOUNDS,
  // Indicates an invalid null.
  ALLO_ERR_INVALID_NULL,
  // Indicates an invalid size.
  ALLO_ERR_INVALID_SIZE,
  // Indicates an invalid alignment.
  ALLO_ERR_INVALID_ALIGNMENT,
  // Indicates that memory is not aligned.
  ALLO_ERR_NOT_ALIGNED,
  // Indicates that an invalid operation was attempted.
  ALLO_ERR_INVALID_OP,

} allo_status;

// Returns a string literal representing the status.
static inline const char *allo_status_str(allo_status status);

static inline const char *allo_status_str(allo_status status) {
  switch (status) {
  case ALLO_OK:
    return "OK";
  case ALLO_OOM:
    return "OUT OF MEMORY";
  case ALLO_ERR_OUT_OF_BOUNDS:
    return "OUT OF BOUNDS";
  case ALLO_ERR_INVALID_NULL:
    return "INVALID NULL";
  case ALLO_ERR_INVALID_SIZE:
    return "INVALID SIZE";
  case ALLO_ERR_INVALID_ALIGNMENT:
    return "INVALID ALIGNMENT";
  case ALLO_ERR_NOT_ALIGNED:
    return "NOT ALIGNED";
  case ALLO_ERR_INVALID_OP:
    return "INVALID OP";
  }
  return "UNKNOWN";
}

#endif // !ALLO_STATUS_H
