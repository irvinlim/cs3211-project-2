#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

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
 * Returns the time passed relative to a given start time in seconds.
 */
void get_time_passed(char *time_str, long long start_time)
{
    long long end_time = wall_clock_time();
    sprintf(time_str, "%0.6f", (end_time - start_time) / 1000000000.0);
}
