#include "allo/status.h"

const char *allo_status_str(allo_status status) {
  switch (status) {
  case ALLO_OK:
    return "OK";
  case ALLO_OOM:
    return "OUT OF MEMORY";
  case ALLO_ERR_INVALID_ADDR:
    return "INVALID ADDRESS";
  case ALLO_ERR_INVALID_NULL:
    return "INVALID NULL";
  case ALLO_ERR_INVALID_SIZE:
    return "INVALID SIZE";
  case ALLO_ERR_INVALID_ALIGN:
    return "INVALID ALIGNMENT";
  }
  return "UNKNOWN";
}
