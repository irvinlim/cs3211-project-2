#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "log.h"

#define INT_MAX_LEN 12

/**
 * Checks whether the number of arguments are satisfied.
 */
void check_arguments(int argc, char **argv, char *prog)
{
    if (argc < 3) {
        LL("Usage: mpirun -np processors %s specfile.txt outputfile.ppm [report.txt]", prog);
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
        int len = ints[i] <= 0 ? ceil(log10(abs(ints[i]) + 1)) + 1 : ceil(log10(ints[i] + 1));

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

/**
 * Prints an array of integers, separated with commas.
 */
void print_ints(LogLevel level, char *msg, int n, int *ints)
{
    char *logbuf = malloc(n * INT_MAX_LEN);
    assert(logbuf != NULL);

    join_ints(logbuf, ',', n, ints);
    LOG(level, "%s: %s", msg, logbuf);

    free(logbuf);
}
