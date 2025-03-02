#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Extract the filename from __FILE__ using a preprocessor trick
#define LOG_FILENAME_STRIP(path) (strrchr(path, '/') ? strrchr(path, '/') + 1 : \
                                  (strrchr(path, '\\') ? strrchr(path, '\\') + 1 : path))

#ifdef _MSC_VER // MSVC Compiler
    #define LOG_FILENAME LOG_FILENAME_STRIP(__FILE__)
#else // GCC / Clang (Works for Linux/macOS/MinGW on Windows)
    #define LOG_FILENAME LOG_FILENAME_STRIP(__BASE_FILE__)
#endif

// Define a per-file logging flag
#define LOG_MODULE_DEFINE(file, enabled) \
    static const bool LOG_ENABLED_##file = enabled

// Logging function with only filename and line number
#define _LOG_PRINT(fmt, ...) \
    printf("[%s:%d] " fmt "\n", LOG_FILENAME, __LINE__, ##__VA_ARGS__)

// If logging is enabled for this module, use real logging. Else, define an empty function.
#define LOG(...) \
    do { if (LOG_ENABLED_##__FILE__) _LOG_PRINT(__VA_ARGS__); } while(0)

#endif // LOGGING_H
