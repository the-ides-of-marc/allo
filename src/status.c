#include "allo/status.h"

const char *allo_status_str(allo_status status) {
  switch (status) {
  case ALLO_OK:
    return "allo: ok";
  case ALLO_OOM:
    return "allo: out of memory";
  case ALLO_ERR_ADDR:
    return "allo: invalid address";
  case ALLO_ERR_NULL:
    return "allo: null pointer";
  case ALLO_ERR_SIZE:
    return "allo: invalid size";
  case ALLO_ERR_ALIGN:
    return "allo: invalid alignment";
  }
  return "allo: unknown status";
}
