#ifndef LOGGING_H
#define LOGGING_H

#define LOG_DEBUG 0
#define LOG_ERR 1

void mylog(int log_level, const char* format, ...);

#endif
