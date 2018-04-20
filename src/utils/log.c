#include <ctype.h>
#include <stdlib.h>

#include "env.h"
#include "log.h"

// Define the log level here.
unsigned char log_level = LOG_LEVEL_NOTICE;

// Define the log process here.
// If it is a non-negative number, only that process will log to stderr.
int log_process = -1;

// Log level labels.
const char *log_level_labels[] = {
    "",
    "ERROR: ",
    "SUCCESS: ",
    "NOTICE: ",
    "",
    "",
    "",
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
    "\033[0;36m",
    "\033[0;35m",
    "\033[0;36m",
    "\033[0;36m",
    "\033[0;35m",
};

const char color_end[] = "\033[0m";

void set_log_level_env()
{
    log_level = getenv_log_level();
    log_process = getenv_log_process();
}
