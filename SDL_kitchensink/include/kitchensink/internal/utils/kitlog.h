#ifndef KITLOG_H
#define KITLOG_H

#ifdef NDEBUG
#define LOG(...)
#else
#include <stdio.h>
#define LOG(...) fprintf(stderr, __VA_ARGS__); fflush(stderr)
#endif

#endif // KITLOG_H
