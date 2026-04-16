#ifndef ALLO_STATUS_H
#define ALLO_STATUS_H

// Enum representing all status codes used by the allo functions across the
// library to indicate success/error.
enum allo_status {
  ALLO_OK = 0,
  ALLO_OOM,
  ALLO_ERR_NULL,
  ALLO_ERR_INVALID_SIZE,
  ALLO_ERR_INVALID_ALIGN,
  ALLO_ERR_MEM_NOT_ALIGNED,
  ALLO_ERR_OUT_OF_BOUNDS,
};

#endif // !ALLO_STATUS_H
