#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include <stdio.h>
#include <stdarg.h>

void glog_init(FILE *logfile);
void glog_destroy(void);
void glog_printf(const char *fmt, ...);

#endif // __GLOBAL_H__
