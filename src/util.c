#include "stdio.h"

#include "util.h"

/* @brief Copy src or default into destination if src is NULL
 */
int cpystr_default(char* src, char* dst, size_t size, char* dft) {
  if (!src) {
    snprintf(dst, size, "%s", dft);
    return -1;
  } else {
    snprintf(dst, size, "%s", src);
    return 0;
  }
}
