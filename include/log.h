#pragma once
#include <stdio.h>

#define __LOG_NC "\033[0m"
#define __LOG_ERROR "\033[31m"
#define __LOG_WARNING "\033[33m"
#define __LOG_INFO "\033[32m"
#define __LOG_DEBUG "\033[34m"

#define __LOG_ERROR_STR "ERROR"
#define __LOG_WARNING_STR "WARNING"
#define __LOG_INFO_STR "INFO"
#define __LOG_DEBUG_STR "DEBUG"

#define LOG_ERROR(...) fprintf(stderr, __LOG_ERROR __LOG_ERROR_STR __LOG_NC ": " __VA_ARGS__)
#define LOG_WARNING(...) fprintf(stderr, __LOG_WARNING __LOG_WARNING_STR __LOG_NC ": " __VA_ARGS__)
#define LOG_INFO(...) fprintf(stderr, __LOG_INFO __LOG_INFO_STR __LOG_NC ": " __VA_ARGS__)

#ifdef RELESE
#define LOG_DEBUG(...)
#else
#define LOG_DEBUG(...) fprintf(stderr, __LOG_DEBUG __LOG_DEBUG_STR __LOG_NC ": " __VA_ARGS__)
#endif
