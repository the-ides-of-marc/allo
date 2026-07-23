#ifndef ALLO_STATUS_H
#define ALLO_STATUS_H

// Enum representing all status codes used by the allo functions across the
// library to indicate success/error.
typedef enum allo_status {
  // Indicates success.
  ALLO_OK = 0,
  // Indicates out of memory.
  ALLO_OOM,
  // Indicates an error due to an invalid address.
  ALLO_ERR_ADDR,
  // Indicates an error due to a null pointer.
  ALLO_ERR_NULL,
  // Indicates an error due to an invalid size.
  ALLO_ERR_SIZE,
  // Indicates an error due to alignment.
  ALLO_ERR_ALIGN,

} allo_status;

// Returns a string literal representing the status.
const char *allo_status_str(allo_status status);

#endif // !ALLO_STATUS_H
