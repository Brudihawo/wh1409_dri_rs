#ifndef LOGGING_H
#define LOGGING_H

#define LOG_DEBUG 0
#define LOG_ERR 1

struct location_info {
  const char* name;
  const char* file;
  int line;
};

#define LOC_INFO { __FUNCTION__, __FILE__, __LINE__ }

#define mylog(...) __mylog_internal((struct location_info)LOC_INFO, __VA_ARGS__)


void __mylog_internal(struct location_info loc_info, int log_level, const char* format, ...);

#endif
