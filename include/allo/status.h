#ifndef ALLO_STATUS_H
#define ALLO_STATUS_H

// Enum representing all status codes used by the allo functions across the
// library to indicate success/error.
typedef enum allo_status {
  // Indicates success.
  ALLO_OK = 0,
  // Indicates out of memory.
  ALLO_OOM,
  // Indicates an invalid address.
  ALLO_ERR_INVALID_ADDR,
  // Indicates an invalid null.
  ALLO_ERR_INVALID_NULL,
  // Indicates an invalid size.
  ALLO_ERR_INVALID_SIZE,
  // Indicates an invalid alignment.
  ALLO_ERR_INVALID_ALIGN,

} allo_status;

// Returns a string literal representing the status.
const char *allo_status_str(allo_status status);

#endif // !ALLO_STATUS_H
