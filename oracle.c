#include "oracle.h"
#include "flags.h"
#include "log.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  Flag flags[] = {{"--debug", set_debug}};
  parse_flags(argc, argv, flags, 1);

  DC dc;
  init_dc(&dc);
  main_loop(&dc);
  return EXIT_SUCCESS;
}

void init_dc(DC *dc) {
  dc->dpy = XOpenDisplay(null);
  if (dc->dpy == null)
    log_error("Display could not be opened\n");
  log_debug("Display has been opened.\n");

  dc->scr = DefaultScreen(dc->dpy);
  int scr_w, scr_h;
  {
    int n;
    XineramaScreenInfo *xi = XineramaQueryScreens(dc->dpy, &n);
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
  dc->d = XCreateWindow(dc->dpy, XRootWindow(dc->dpy, dc->scr), win_x, win_y,
                        WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                        DefaultDepth(dc->dpy, dc->scr), CopyFromParent,
                        DefaultVisual(dc->dpy, dc->scr),
                        CWOverrideRedirect | CWBackPixel | CWEventMask, &wa);

  XSelectInput(dc->dpy, dc->d,
               StructureNotifyMask | KeyReleaseMask | KeyPressMask |
                   FocusChangeMask);
  XMapWindow(dc->dpy, dc->d);

  XSetInputFocus(dc->dpy, dc->d, dc->scr, null);

  dc->gc = XCreateGC(dc->dpy, dc->d, dc->scr, null);
  {
    XSetForeground(dc->dpy, dc->gc, XWhitePixel(dc->dpy, dc->scr));

    int font_ln;
    char **fonts = XListFonts(dc->dpy, "*mono*", 20, &font_ln);
    XSetFont(dc->dpy, dc->gc, XLoadFont(dc->dpy, fonts[font_ln - 1]));
  }
}

void main_loop(DC *dc) {
  XEvent e;

  // Nicely waits until the window map occurs.
  while (true) {
    XNextEvent(dc->dpy, &e);
    if (e.type == MapNotify)
      break;
  }

  log_debug("Window is ready\n");

  char search[1024] = {};
  int search_ln = 0;

  boolean shift_held = false;
  while (true) {
    draw(dc, search, search_ln);
    if (!handle_events(dc, &e, search, &search_ln, &shift_held)) {
      break;
    }
  }

  XCloseDisplay(dc->dpy);
}

void draw(DC *dc, const char *search, int search_ln) {
  XClearWindow(dc->dpy, dc->d);

  XDrawString(dc->dpy, dc->d, dc->gc, 0, 20, search, search_ln);
  XFlush(dc->dpy);

  XDrawLine(dc->dpy, dc->d, dc->gc, 0, 24, WINDOW_WIDTH, 24);
}

boolean handle_events(DC *dc, XEvent *e, char *search, int *search_ln,
                      boolean *shift_held) {
  XNextEvent(dc->dpy, e);

  unsigned int c = e->xkey.keycode;
  boolean is_shift_key = c == KEYCODE_LSHIFT || c == KEYCODE_RSHIFT;

  switch (e->type) {
  case KeyPress:
    log_debug("Pressed key: %d\n", c);

    if (c == KEYCODE_EXIT) {
      log_debug("Exit using escape key\n");
      return false;
    }

    if (c == KEYCODE_BACKSPACE) {
      log_debug("Removing character\n");
      if (*search_ln > 0)
        (*search_ln)--;
      break;
    }

    if (is_shift_key) {
      *shift_held = true;
      break;
    }

    KeySym ks = XLookupKeysym(&e->xkey, *shift_held);
    if (ks != NoSymbol) {
      log_debug("%c\n", ks);
      search[(*search_ln)++] = ks;
      break;
    }
    break;
  case KeyRelease:
    if (is_shift_key)
      *shift_held = false;
    break;
  case FocusOut:
    log_debug("Focus lost.\n");
    return false;
  }

  return true;
}
