/**
 * Adapted and modified from
 * https://gist.github.com/fclairamb/7441750
 */

#include <stdio.h>
#include <time.h>

#include "multiproc.h"

#ifndef LOG_H
#define LOG_H
typedef enum log_level_t {
    LOG_LEVEL_NONE,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_SUCCESS,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_VERBOSE,
    LOG_LEVEL_MPI,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_DEBUG2,
} LogLevel;
#endif

extern unsigned char log_level;
extern int log_process;
extern const char *log_level_labels[];
extern const char *log_level_colors[];
extern const char color_end[];

#define LOG(level, fmt, arg...)                                                           \
    do {                                                                                  \
        if (level <= log_level && (log_process < 0 || log_process == get_process_id())) { \
            time_t timer;                                                                 \
            char time_str[26];                                                            \
            time(&timer);                                                                 \
            strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", localtime(&timer));               \
            fprintf(stderr, "%s[%s] %02d ~  %s" fmt "%s\n",                               \
                log_level_colors[level],                                                  \
                time_str,                                                                 \
                get_process_id(),                                                         \
                log_level_labels[level],                                                  \
                arg,                                                                      \
                color_end);                                                               \
            fflush(stderr);                                                               \
        }                                                                                 \
    } while (0)

#define LL(fmt, arg...) LOG(LOG_LEVEL_NONE, fmt, arg)
#define LL_DEBUG2(fmt, arg...) LOG(LOG_LEVEL_DEBUG2, fmt, arg)
#define LL_DEBUG(fmt, arg...) LOG(LOG_LEVEL_DEBUG, fmt, arg)
#define LL_MPI(fmt, arg...) LOG(LOG_LEVEL_MPI, fmt, arg)
#define LL_VERBOSE(fmt, arg...) LOG(LOG_LEVEL_VERBOSE, fmt, arg)
#define LL_NOTICE(fmt, arg...) LOG(LOG_LEVEL_NOTICE, fmt, arg)
#define LL_SUCCESS(fmt, arg...) LOG(LOG_LEVEL_SUCCESS, fmt, arg)
#define LL_ERROR(fmt, arg...) LOG(LOG_LEVEL_ERROR, fmt, arg)

// The log level can be overwritten by the LOG_LEVEL environment variable.
void set_log_level_env();
