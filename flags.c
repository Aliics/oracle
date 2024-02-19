#include "flags.h"
#include <string.h>

void parse_flags(int argc, char **argv, Flag *flags, int n_flags) {
  for (int i = 0; i < n_flags; ++i) {
    for (int j = 0; j < argc; ++j) {
      if (strcmp(flags[i].flag, argv[j]) == 0) {
        flags[i].fn(true);
        break;
      }
    }
  }
}
