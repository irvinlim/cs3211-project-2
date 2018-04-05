#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "log.h"
#include "common.h"

/** 
 * Determines the current time.
 */
long long wall_clock_time()
{
#ifdef LINUX
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (long long)(tp.tv_nsec + (long long)tp.tv_sec * 1000000000ll);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_usec * 1000 + (long long)tv.tv_sec * 1000000000ll);
#endif
}

/**
 * Checks whether the number of arguments are satisfied.
 */
void check_arguments(int argc, char **argv, char *prog)
{
    set_log_level_env();

    if (argc != 3)
    {
        LL("Usage: mpirun -np processors %s specfile.txt outputfile.ppm", prog);
        exit(EXIT_FAILURE);
    }
}
