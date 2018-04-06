#include <ctype.h>
#include <stdlib.h>

#include "log.h"

// Define the log level here.
unsigned char log_level = LOG_LEVEL_NOTICE;

// Log level labels.
const char *log_level_labels[] = {
    "",
    "ERROR: ",
    "SUCCESS: ",
    "NOTICE: ",
    "",
    "",
    "",
};

// Log level colors.
const char *log_level_colors[] = {
    "",
    "\033[0;31m",
    "\033[0;32m",
    "\033[0;33m",
    "\033[0;34m",
    "\033[0;35m",
    "\033[0;36m",
};

const char color_end[] = "\033[0m";

// The log level can be overwritten by the LOG_LEVEL environment variable.
void set_log_level_env()
{
    char *env = getenv("LOG_LEVEL");

    // Only parse env var if it is a number.
    if (env != NULL && env[0] >= '0' && env[0] <= '9')
        log_level = (char)atoi(env);
}
