#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define log_error(...) fprintf(stderr, __VA_ARGS__), putc('\n', stderr)
#define log_info(...) printf(__VA_ARGS__), putc('\n', stdout)

#endif
