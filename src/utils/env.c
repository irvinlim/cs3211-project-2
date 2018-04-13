#include <ctype.h>
#include <stdlib.h>

#include "env.h"
#include "log.h"

unsigned char getenv_log_level()
{
    char *env = getenv("LOG_LEVEL");
    if (env != NULL && env[0] >= '0' && env[0] <= '9')
        return (char)atoi(env);

    return LOG_LEVEL_NOTICE;
}

int getenv_log_process()
{
    char *env = getenv("LOG_PROCESS");
    if (env != NULL)
        return atoi(env);

    return -1;
}

unsigned char getenv_debug_frames()
{
    char *env = getenv("DEBUG_FRAMES");
    if (env != NULL && env[0] >= '0' && env[0] <= '1')
        return (char)atoi(env);

    return 0;
}
