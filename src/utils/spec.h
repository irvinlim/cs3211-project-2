#include "types.h"

/**
 * Reads the specification file.
 */
Spec read_spec_file(char *specfile);

/**
 * Debug prints the Spec.
 */
void print_spec(Spec spec);

/**
 * Returns the length of the size of the pool.
 */
int get_pool_length();
