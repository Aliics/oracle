#ifndef _ORACLE_LOG_H_
#define _ORACLE_LOG_H_

#include "oracle.h"

void set_debug(boolean debug);
void log_info(const char *fmt, ...);
void log_debug(const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif
