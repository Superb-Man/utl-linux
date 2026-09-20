#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

// ANSI color codes
#define GREEN_TEXT "\033[32m"
#define RED_TEXT   "\033[31m"
#define RESET_TEXT "\033[0m"

// formats into a local buffer and writes with write(2) -- avoids stdio's non-signal-safe locking
static inline void safe_log(const char* tag, const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (n <= 0) return;
    if ((size_t)n >= sizeof(buf)) n = sizeof(buf) - 1;

    write(STDERR_FILENO, tag, strlen(tag));
    write(STDERR_FILENO, buf, (size_t)n);
}

#define DEBUG_PRINT(fmt, ...) safe_log(GREEN_TEXT "DEBUG: ", fmt RESET_TEXT, ##__VA_ARGS__)
#define ERROR_PRINT(fmt, ...) safe_log(RED_TEXT "ERROR: ", fmt RESET_TEXT, ##__VA_ARGS__)

#define ASSERT(condition, fmt, ...) \
    do { \
        if (!(condition)) { \
            safe_log(RED_TEXT "ASSERTION FAILED: ", fmt RESET_TEXT "\n", ##__VA_ARGS__); \
            safe_log(RED_TEXT, " at %s:%d\n" RESET_TEXT, __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

#endif // DEBUG_H