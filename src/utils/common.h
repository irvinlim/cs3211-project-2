#include "types.h"
#define DEBUG 1

/**
 * Prints debug statements.
 */
void debug(char *msg);

/** 
 * Determines the current time.
 */
long long wall_clock_time();

/**
 * Checks whether the number of arguments are satisfied.
 */
void check_arguments(int argc, char **argv, char *prog);

/**
 * Reads the specification file.
 */
Spec read_spec_file(char *specfile);

/**
 * Debug prints the Spec.
 */
void print_spec(Spec spec);
