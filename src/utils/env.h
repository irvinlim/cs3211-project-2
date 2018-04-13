/**
 * Gets the LOG_LEVEL value from the environment.
 * Defaults to LOG_LEVEL_NOTICE.
 */
unsigned char getenv_log_level();

/**
 * Gets the LOG_PROCESS value from the environment.
 * Defaults to -1 (all processes).
 */
int getenv_log_process();

/**
 * Gets the DEBUG_FRAMES value from the environment.
 * Defaults to 0 (debug off).
 */
unsigned char getenv_debug_frames();
