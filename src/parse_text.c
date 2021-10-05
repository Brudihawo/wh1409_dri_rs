#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "logging.h"

#define LINE_SIZE 25
#define PEN_UP 32775
#define PEN_DOWN 33031

/* @brief enum containing Pen status while pen is in range of tablet
 *        can be UP or DOWN
 */
typedef enum { UP, DOWN } PenStatus;

typedef struct {
  PenStatus status;
  uint hpos;
  uint vpos;
  uint pressure;
} PenInfo;

long get_filesize(FILE *fp) {
  long cur_pos = ftell(fp);
  fseek(fp, 0, SEEK_END);
  long sz = ftell(fp);
  fseek(fp, cur_pos, SEEK_SET);
  return sz;
}

/* @brief get int form 16 bits in the form of 2 chars
 *
 * @param ms: Most significant byte
 * @param ls: least significant byte
 */
static inline uint uint_from_u16_le(char ms, char ls) {
  return (uint8_t)ms * 256 + (uint8_t)ls;
}

PenInfo parse_line(char *line, int lsize) {
  uint8_t reduced[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  int cur_pos = 0;
  for (int i = 0; i < lsize; i++) {
    if ((64 < line[i]) && (line[i] < 71)) { // Letters A-F
      reduced[cur_pos / 2] += ((uint8_t)line[i] - (uint8_t)65 + (uint8_t)10) *
                              (cur_pos % 2 == 0 ? (uint8_t)16 : (uint8_t)1);
      cur_pos += 1;
    } else if ((47 < line[i]) && (line[i] < 58)) { // Numbers 0-9
      reduced[cur_pos / 2] += ((uint8_t)line[i] - (uint8_t)48) *
                              (cur_pos % 2 == 0 ? (uint8_t)16 : (uint8_t)1);
      cur_pos += 1;
    }
  }

  uint status = uint_from_u16_le(reduced[1], reduced[0]);

  if (status != PEN_UP && status != PEN_DOWN) {
    mylog(LOG_ERR, "Unknown Status %u", status);
    exit(0);
  }

  PenInfo info = {
      .hpos = uint_from_u16_le(reduced[3], reduced[2]),
      .vpos = uint_from_u16_le(reduced[5], reduced[4]),
      .pressure = uint_from_u16_le(reduced[7], reduced[6]),
      .status = (status == 32880 ? UP : DOWN),
  };

  return info;
}

/* @brief write PenInfo string representation to buffer
 *        status (0/1) pressure hpos vpos
 */
void peninfo_to_chars(PenInfo info, char *buf, int bufsize) {
  snprintf(buf, bufsize, "%d %-5u %-5u %-5u",
           info.status == UP ? 1 : 0, info.pressure, info.hpos, info.vpos);
}

int main(int argc, char** argv) {
  char line[LINE_SIZE];
  FILE *fp;

  if (argc < 2) {
    mylog(LOG_ERR, "Not enough arguments. Supply a file name to parse");
    exit(0);
  }

  mylog(LOG_DEBUG, "Filename %s", argv[1]);

  fp = fopen(argv[1], "r");
  long fsize = get_filesize(fp);
  long cur_pos = 0;
  PenInfo info;

  printf("#%8s %9s %9s %9s\n", "status", "vpos", "hpos", "pressure");
  while (cur_pos + LINE_SIZE < fsize) {
    fgets(line, LINE_SIZE, fp);
    fseek(fp, cur_pos, SEEK_SET);

    mylog(LOG_DEBUG, "%ld %s", cur_pos, line);
    info = parse_line(line, LINE_SIZE);
    printf("%9u %9u %9u %9u\n", info.status, info.vpos, info.hpos, info.pressure);
    cur_pos += LINE_SIZE;
  }

  fclose(fp);
}
