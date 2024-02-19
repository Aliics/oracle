#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

boolean is_debug = false;
void set_debug(boolean debug) { is_debug = debug; }

void log_info(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vprintf(fmt, va);
  va_end(va);
}

void log_debug(const char *fmt, ...) {
  if (!is_debug) return;
  va_list va;
  va_start(va, fmt);
  vprintf(fmt, va);
  va_end(va);
}

void log_error(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vprintf(fmt, va);
  va_end(va);
  exit(1);
}
