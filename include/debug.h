#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>

// ANSI color codes
#define GREEN_TEXT "\033[32m"
#define RED_TEXT   "\033[31m"
#define RESET_TEXT "\033[0m"


#define DEBUG_PRINT(fmt, ...) \
    do { \
        fprintf(stderr, GREEN_TEXT "DEBUG: " fmt RESET_TEXT, ##__VA_ARGS__); \
    } while (0)


#define ERROR_PRINT(fmt, ...) \
    do { \
        fprintf(stderr, RED_TEXT "ERROR: " fmt RESET_TEXT, ##__VA_ARGS__); \
    } while (0)

#define ASSERT(condition, fmt, ...) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, RED_TEXT "ASSERTION FAILED: " fmt RESET_TEXT "\n", ##__VA_ARGS__); \
            fprintf(stderr, RED_TEXT " at %s:%d\n" RESET_TEXT, __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

#endif // DEBUG_H
