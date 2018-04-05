/**
 * Adapted and modified from
 * https://gist.github.com/fclairamb/7441750
 */

#include <stdio.h>
#include <time.h>

enum {
    LOG_LEVEL_NONE,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_SUCCESS,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_VERBOSE,
    LOG_LEVEL_DEBUG,
};

extern unsigned char log_level;
extern const char *log_level_labels[];
extern const char *log_level_colors[];
extern const char color_end[];

#define LOG(level, fmt, arg...)                                             \
    do {                                                                    \
        if (level <= log_level) {                                           \
            time_t timer;                                                   \
            char time_str[26];                                              \
            time(&timer);                                                   \
            strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", localtime(&timer)); \
            fprintf(stderr, "%s[%s] %s" fmt "%s\n",                         \
                log_level_colors[level],                                    \
                time_str,                                                   \
                log_level_labels[level],                                    \
                arg,                                                        \
                color_end);                                                 \
        }                                                                   \
    } while (0)

#define LL(fmt, arg...) LOG(LOG_LEVEL_NONE, fmt, arg)
#define LL_DEBUG(fmt, arg...) LOG(LOG_LEVEL_DEBUG, fmt, arg)
#define LL_VERBOSE(fmt, arg...) LOG(LOG_LEVEL_VERBOSE, fmt, arg)
#define LL_NOTICE(fmt, arg...) LOG(LOG_LEVEL_NOTICE, fmt, arg)
#define LL_SUCCESS(fmt, arg...) LOG(LOG_LEVEL_SUCCESS, fmt, arg)
#define LL_ERROR(fmt, arg...) LOG(LOG_LEVEL_ERROR, fmt, arg)

// The log level can be overwritten by the LOG_LEVEL environment variable.
void set_log_level_env();
