#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "pen.h"

#define LINE_SIZE 25

/* @brief get file size in bytes
 *
 * @param fp: file pointer
 * @return file size in bytes
 */
long get_filesize(FILE *fp) {
  long cur_pos = ftell(fp);
  fseek(fp, 0, SEEK_END);
  long sz = ftell(fp);
  fseek(fp, cur_pos, SEEK_SET);
  return sz;
}

/* @brief Parse a line of text to PenInfo
 *
 * @param line:  line to process
 * @param lsize: size of lin
 *
 * @return PenInfo
 */
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

  return peninfo_from_bytes(reduced);
}

int main(int argc, char **argv) {
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
    printf("%9u %9u %9u %9u\n", info.status, info.vpos, info.hpos,
           info.pressure);
    cur_pos += LINE_SIZE;
  }

  fclose(fp);
}
