#ifndef _ORACLE_FLAGS_H_
#define _ORACLE_FLAGS_H_

#include "oracle.h"

typedef struct Flag {
  const char *flag;
  void (*fn)(boolean);
} Flag;

void parse_flags(int argc, char **argv, Flag *flags, int n_flags);

#endif
