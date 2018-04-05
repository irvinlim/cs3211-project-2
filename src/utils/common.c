#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "log.h"

/**
 * Checks whether the number of arguments are satisfied.
 */
void check_arguments(int argc, char **argv, char *prog)
{
    if (argc != 3) {
        LL("Usage: mpirun -np processors %s specfile.txt outputfile.ppm", prog);
        exit(EXIT_FAILURE);
    }
}
