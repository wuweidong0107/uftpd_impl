
#include "uftpd.h"

static int loglevel = LOG_INFO;

void logit(int severity, const char *fmt, ...)
{
    FILE *file;
    va_list args;
    
    if (severity > LOG_WARNING)
        file = stdout;
    else
        file = stderr;
    
    va_start(args, fmt);
    if (severity <= loglevel) {
        if (loglevel == LOG_DEBUG)
            fprintf(file, "%d> ", getpid());
        vfprintf(file, fmt, args);
        fflush(file);
    }
    va_end(args);
}