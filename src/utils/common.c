#include <math.h>
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

/**
 * Joins an array of integers into a string.
 * Warning: This function does not do bounds-checking.
 */
void join_ints(char *buf, char delimiter, int n, int *ints)
{
    int currentlen = 0;

    for (int i = 0; i < n; i++) {
        int len = ints[i] <= 1 ? 1 : ceil(log10(ints[i]));

        // Add the ID to the buffer.
        sprintf(&buf[currentlen], "%d", ints[i]);
        currentlen += len;

        // Add a delimiter.
        if (i < n - 1) buf[currentlen++] = delimiter;
    }

    if (currentlen == 0) {
        sprintf(buf, "None");
        currentlen += 4;
    }

    // Add null-terminating character.
    buf[currentlen++] = 0;
}
