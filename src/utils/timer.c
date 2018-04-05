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
 * Returns a formatted string of nanoseconds in seconds.
 */
void format_time(char *time_str, size_t len, long long nanoseconds)
{
    snprintf(time_str, len, "%0.6f", nanoseconds / 1000000000.0);
}
