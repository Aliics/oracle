#include "flags.h"
#include "log.h"
#include <X11/Xlib.h>
#include <assert.h>

int main(int argc, char **argv) {
  Flag flags[] = {{"--debug", set_debug}};
  parse_flags(argc, argv, flags, 1);

  Display *dpy = XOpenDisplay(null);
  assert(dpy);
  log_debug("Display has been opened.\n");
}
