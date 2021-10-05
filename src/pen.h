#include "stdint.h"

#define PEN_UP (uint8_t)32775
#define PEN_DOWN (uint8_t)33031

/* @brief enum containing pen status while in range of tablet
 *        can be UP or DOWN
 */
typedef enum { UP, DOWN } PenStatus;

/* @brief Pen Inforation for one frame
 */
typedef struct {
  PenStatus status;
  uint8_t hpos;
  uint8_t vpos;
  uint8_t pressure;
} PenInfo;

void peninfo_to_chars(PenInfo info, char *buf, int bufsize);

PenInfo peninfo_from_bytes(uint8_t* bytes);


