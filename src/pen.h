#include "stdint.h"

#define PEN_UP (uint16_t)32775
#define PEN_DOWN (uint16_t)33031

/* @brief enum containing pen status while in range of tablet
 *        can be UP or DOWN
 */
typedef enum { UP, DOWN } PenStatus;

/* @brief Pen Inforation for one frame
 */
typedef struct {
  PenStatus status;
  uint16_t hpos;
  uint16_t vpos;
  uint16_t pressure;
} PenInfo;

void peninfo_to_chars(PenInfo info, char *buf, int bufsize);

PenInfo peninfo_from_bytes(uint8_t* bytes);


