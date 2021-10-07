#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "logging.h"

#define TIME_STR_SIZE 20
#define BLUE "\e[34m"
#define RED "\e[31m"
#define NC "\e[0m"

void __mylog_internal(struct location_info loc_info, int log_level,
                      const char *format, ...) {
  char time_part[TIME_STR_SIZE];

  time_t t = time(NULL);
  struct tm *tmp;

  tmp = localtime(&t);

  strftime(time_part, TIME_STR_SIZE, "%d.%m.%Y %H:%M:%S", tmp);

  if (log_level == LOG_DEBUG) {

#ifndef DEBUG // Return if log level is debug and debugging is off
    return;
#endif

    dprintf(STDERR_FILENO, "%s " BLUE "DBG: " NC, time_part);
  } else {
    dprintf(STDERR_FILENO,
            "%s " RED "ERR " NC " from calling function %s at %s:%i: ",
            time_part, loc_info.name, loc_info.file, loc_info.line);
  }

  va_list args;
  va_start(args, format);
  vdprintf(STDERR_FILENO, format, args);
  va_end(args);
  dprintf(STDERR_FILENO, NC "\n");
}
