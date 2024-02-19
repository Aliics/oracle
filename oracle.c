#include "oracle.h"
#include "flags.h"
#include "log.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  Flag flags[] = {{"--debug", set_debug}};
  parse_flags(argc, argv, flags, 1);

  Display *dpy = XOpenDisplay(null);
  assert(dpy);
  log_debug("Display has been opened.\n");

  int scr = DefaultScreen(dpy);
  int scr_w = XDisplayWidth(dpy, scr);
  int scr_h = XDisplayHeight(dpy, scr);

  log_debug("Screen dimensions: %dx%d\n", scr_w, scr_h);

  XSetWindowAttributes wa = {
      .override_redirect = true,
      .background_pixmap = ParentRelative,
      .event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask,
  };

  int win_x = scr_w / 2 - WINDOW_WIDTH / 2;
  int win_y = scr_h / 2 - WINDOW_HEIGHT / 2;
  // Window win = XCreateWindow(
  //     dpy, XRootWindow(dpy, scr), win_x, win_y, WINDOW_WIDTH, WINDOW_HEIGHT,
  //     0, DefaultDepth(dpy, scr), CopyFromParent, DefaultVisual(dpy, scr),
  //     CWOverrideRedirect | CWBackPixel | CWEventMask, &wa);
  Window win = XCreateSimpleWindow(dpy, XRootWindow(dpy, scr), win_x, win_y,
                                   WINDOW_WIDTH, WINDOW_HEIGHT, 2,
                                   BlackPixel(dpy, scr), WhitePixel(dpy, scr));

  XSelectInput(dpy, win, StructureNotifyMask | KeyPressMask | FocusChangeMask);
  XMapWindow(dpy, win);

  XEvent e;
  {
    while (true) {
      XNextEvent(dpy, &e);
      if (e.type == MapNotify)
        break;
    }

    log_debug("Window is ready\n");

    boolean window_open = true;
    while (window_open) {
      XNextEvent(dpy, &e);
      switch (e.type) {
      case KeyPress:
        log_debug("Pressed key: %d\n", e.xkey.keycode);
        break;
      case FocusOut:
        log_debug("Focus lost.\n");
        window_open = false;
        break;
      }
    }
  }

  XCloseDisplay(dpy);

  return EXIT_SUCCESS;
}
