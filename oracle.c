#include "oracle.h"
#include "flags.h"
#include "log.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  Flag flags[] = {{"--debug", set_debug}};
  parse_flags(argc, argv, flags, 1);

  Display *dpy = XOpenDisplay(null);
  assert(dpy);
  log_debug("Display has been opened.\n");

  int scr = DefaultScreen(dpy);
  int scr_w, scr_h;
  {
    int n;
    XineramaScreenInfo *xi = XineramaQueryScreens(dpy, &n);
    scr_w = xi->width;
    scr_h = xi->height;
    log_debug("Screen dimensions from Xinerama: %dx%d\n", scr_w, scr_h);
    free(xi);
  }

  XSetWindowAttributes wa = {
      .override_redirect = true,
      .background_pixmap = ParentRelative,
      .event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask,
  };

  int win_x = scr_w / 2 - WINDOW_WIDTH / 2;
  int win_y = scr_h / 2 - WINDOW_HEIGHT / 2;
  Window win = XCreateWindow(
      dpy, XRootWindow(dpy, scr), win_x, win_y, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
      DefaultDepth(dpy, scr), CopyFromParent, DefaultVisual(dpy, scr),
      CWOverrideRedirect | CWBackPixel | CWEventMask, &wa);

  XSelectInput(dpy, win,
               StructureNotifyMask | KeyReleaseMask | KeyPressMask |
                   FocusChangeMask);
  XMapWindow(dpy, win);

  XSetInputFocus(dpy, win, scr, null);

  GC gc = XCreateGC(dpy, win, scr, null);
  {
    XSetForeground(dpy, gc, XWhitePixel(dpy, scr));

    int font_ln;
    char **fonts = XListFonts(dpy, "*mono*", 20, &font_ln);
    XSetFont(dpy, gc, XLoadFont(dpy, fonts[font_ln - 1]));
  }

  XEvent e;
  {
    while (true) {
      XNextEvent(dpy, &e);
      if (e.type == MapNotify)
        break;
    }

    log_debug("Window is ready\n");

    char search[1024] = {};
    int search_ln = 0;

    boolean window_open = true;
    boolean shift_held = false;
    while (window_open) {
      XClearWindow(dpy, win);

      XDrawString(dpy, win, gc, 0, 20, search, search_ln);
      XFlush(dpy);

      XDrawLine(dpy, win, gc, 0, 24, WINDOW_WIDTH, 24);

      XNextEvent(dpy, &e);

      unsigned int c = e.xkey.keycode;
      boolean is_shift_key = c == KEYCODE_LSHIFT || c == KEYCODE_RSHIFT;

      switch (e.type) {
      case KeyPress:
        log_debug("Pressed key: %d\n", c);

        if (c == KEYCODE_EXIT) {
          log_debug("Exit using escape key\n");
          window_open = false;
          break;
        }

        if (c == KEYCODE_BACKSPACE) {
          log_debug("Removing character\n");
          if (search_ln > 0)
            search_ln--;
          break;
        }

        if (is_shift_key) {
          shift_held = true;
          break;
        }

        KeySym ks = XLookupKeysym(&e.xkey, shift_held);
        if (ks != NoSymbol) {
          log_debug("%c\n", ks);
          search[search_ln++] = ks;
          break;
        }
        break;
      case KeyRelease:
        if (is_shift_key)
          shift_held = false;
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
