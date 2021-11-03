#include "stdio.h"
#include "stdlib.h"

#include "pen.h"
#include "logging.h"

/* @brief write PenInfo string representation to buffer
 *        status (0/1) pressure hpos vpos
 */
void peninfo_to_chars(PenInfo info, char *buf, int bufsize) {
  snprintf(buf, bufsize, "%d %5u %5u %5u", info.status == UP ? 1 : 0,
           info.pressure, info.hpos, info.vpos);
}

/* @brief get int form 16 bits in the form of 2 chars
 *
 * @param ms: Most significant byte (as char)
 * @param ls: least significant byte (as char)
 */
static inline uint16_t uint_from_u16_le(uint8_t ms, uint8_t ls) {
  if (ls > 255) {
  }
  return (uint16_t)ms * 256 + (uint16_t)ls;
}

/* @brief convert 8 Bytes in the form of uint8_t to PenInfo
 */
PenInfo peninfo_from_bytes(uint8_t *bytes) {
  uint16_t status = uint_from_u16_le(bytes[1], bytes[0]);

  if (status != PEN_UP && status != PEN_DOWN) {
    mylog(LOG_ERR, "Unknown Status %x", status);
    exit(0);
  }

  PenInfo info = {
      .hpos = uint_from_u16_le(bytes[3], bytes[2]),
      .vpos = uint_from_u16_le(bytes[5], bytes[4]),
      .pressure = uint_from_u16_le(bytes[7], bytes[6]),
      .status = (status == PEN_UP ? UP : DOWN),
  };

  return info;
}
