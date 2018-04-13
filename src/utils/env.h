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
