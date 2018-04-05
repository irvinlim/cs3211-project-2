/** 
 * Determines the current time.
 */
long long wall_clock_time();

/**
 * Returns a formatted string of nanoseconds in seconds.
 */
void format_time(char *time_str, size_t len, long long nanoseconds);
