#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define MAX_MODULES 50
#define MODULE_NAME_MAX 32

typedef struct {
    char name[MODULE_NAME_MAX];
    bool enabled;
} LogModule;

static LogLevel current_level = LOG_INFO;
static LogModule modules[MAX_MODULES];
static int module_count = 0;
static FILE* log_file = NULL;
static bool log_to_file = false;

// Initialize the logging system
void log_init(void) {
    // Default initialization
    current_level = LOG_INFO;
    module_count = 0;
    
    // You could add file logging initialization here
    // log_file = fopen("game.log", "w");
    // log_to_file = (log_file != NULL);
}

// Set global minimum log level
void log_set_level(LogLevel level) {
    current_level = level;
}

// Find a module by name
static int find_module(const char* module) {
    for (int i = 0; i < module_count; i++) {
        if (strcmp(modules[i].name, module) == 0) {
            return i;
        }
    }
    return -1;
}

// Enable/disable logging for a specific module
void log_enable_module(const char* module, bool enable) {
    int index = find_module(module);
    
    if (index >= 0) {
        modules[index].enabled = enable;
    } else if (module_count < MAX_MODULES) {
        strncpy(modules[module_count].name, module, MODULE_NAME_MAX - 1);
        modules[module_count].name[MODULE_NAME_MAX - 1] = '\0';
        modules[module_count].enabled = enable;
        module_count++;
    }
}

// Check if a module is enabled for logging
bool log_is_module_enabled(const char* module) {
    int index = find_module(module);
    
    if (index >= 0) {
        return modules[index].enabled;
    } else if (module_count < MAX_MODULES) {
        // Auto-register module with default enabled state
        strncpy(modules[module_count].name, module, MODULE_NAME_MAX - 1);
        modules[module_count].name[MODULE_NAME_MAX - 1] = '\0';
        modules[module_count].enabled = true;  // Default to enabled
        module_count++;
        return true;
    }
    
    return false;
}

// Get string representation of log level
static const char* log_level_to_string(LogLevel level) {
    switch (level) {
        case LOG_TRACE: return "TRACE";
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO ";
        case LOG_WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
        default:        return "UNKN ";
    }
}

// Get color code for log level (for console output)
static const char* log_level_color(LogLevel level) {
    switch (level) {
        case LOG_TRACE: return "\033[90m"; // Dark gray
        case LOG_DEBUG: return "\033[36m"; // Cyan
        case LOG_INFO:  return "\033[32m"; // Green
        case LOG_WARN:  return "\033[33m"; // Yellow
        case LOG_ERROR: return "\033[31m"; // Red
        case LOG_FATAL: return "\033[35m"; // Magenta
        default:        return "\033[0m";  // Reset
    }
}

// Core logging function
void log_message(LogLevel level, const char* module, const char* file, int line, const char* format, ...) {
    // Skip if level is below current minimum level
    if (level < current_level) {
        return;
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Extract filename from path
    const char* filename = strrchr(file, '/');
    if (filename) {
        filename++; // Skip the slash
    } else {
        filename = file;
    }
    
    // Format the log message
    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Print to console with colors
    printf("%s[%s][%s][%s:%d] %s\033[0m\n", 
           log_level_color(level),
           time_str,
           log_level_to_string(level),
           module,
           line,
           message);
    
    // Write to log file if enabled
    if (log_to_file && log_file) {
        fprintf(log_file, "[%s][%s][%s:%d] %s\n", 
                time_str,
                log_level_to_string(level),
                module,
                line,
                message);
        fflush(log_file);
    }
}

// Cleanup logging system
void log_cleanup(void) {
    if (log_to_file && log_file) {
        fclose(log_file);
        log_file = NULL;
    }
} 