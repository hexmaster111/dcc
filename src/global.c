#include "global.h"
#include "assume.h"

typedef struct global
{
    FILE *log_file;
} Global;

Global g;

void glog_init(FILE *logfile)
{
    ASSUME(logfile != NULL);

    g.log_file = logfile;
    glog_printf("Log file opened\n");
}

void glog_destroy(void)
{
    fclose(g.log_file);
}

void glog_printf(const char *fmt, ...)
{
    ASSUME(g.log_file != NULL);
    va_list args;
    va_start(args, fmt);
    vfprintf(g.log_file, fmt, args);
    va_end(args);
    fflush(g.log_file);
}
