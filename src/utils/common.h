#include "log.h"
#include "types.h"

/**
 * Checks whether the number of arguments are satisfied.
 */
void check_arguments(int argc, char **argv, char *prog);

/**
 * Joins an array of integers into a string.
 */
void join_ints(char *buf, char delimiter, int n, int *ints);

/**
 * Prints an array of integers, separated with commas.
 */
void print_ints(LogLevel log_level, char *msg, int n, int *ints);
