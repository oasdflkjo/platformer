#ifndef LOGGING_H
#define LOGGING_H

#include <stdbool.h>

// Log levels
typedef enum {
    LOG_TRACE,   // Most detailed information
    LOG_DEBUG,   // Debugging information
    LOG_INFO,    // General information
    LOG_WARN,    // Warnings
    LOG_ERROR,   // Errors
    LOG_FATAL    // Critical errors
} LogLevel;

// Initialize the logging system
void log_init(void);

// Set global minimum log level
void log_set_level(LogLevel level);

// Enable/disable logging for a specific module
void log_enable_module(const char* module, bool enable);

// Check if a module is enabled for logging
bool log_is_module_enabled(const char* module);

// Core logging function (used by macros)
void log_message(LogLevel level, const char* module, const char* file, int line, const char* format, ...);

// Cleanup logging system
void log_cleanup(void);

// Logging macros - simplified to avoid token pasting issues
#define LOG_MODULE_DEFINE(module_name, default_enabled) \
    static const char* LOG_MODULE = module_name;

// Logging macros with module check
#define LOG_TRACE(format, ...) if (log_is_module_enabled(LOG_MODULE)) log_message(LOG_TRACE, LOG_MODULE, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) if (log_is_module_enabled(LOG_MODULE)) log_message(LOG_DEBUG, LOG_MODULE, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  if (log_is_module_enabled(LOG_MODULE)) log_message(LOG_INFO, LOG_MODULE, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  if (log_is_module_enabled(LOG_MODULE)) log_message(LOG_WARN, LOG_MODULE, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) if (log_is_module_enabled(LOG_MODULE)) log_message(LOG_ERROR, LOG_MODULE, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) if (log_is_module_enabled(LOG_MODULE)) log_message(LOG_FATAL, LOG_MODULE, __FILE__, __LINE__, format, ##__VA_ARGS__)

#endif // LOGGING_H 