#include "oracle.h"
#include "flags.h"
#include "log.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  Flag flags[] = {{"--debug", set_debug}};
  parse_flags(argc, argv, flags, 1);

  int bin_names_ln;
  char **bin_names = get_bin_names(&bin_names_ln);

  log_debug("Loaded %d binary paths\n", bin_names_ln);

  DC dc;
  init_dc(&dc);
  main_loop(&dc, bin_names, bin_names_ln);
  return EXIT_SUCCESS;
}

char **get_bin_names(int *n) {
  char **out = malloc(0);

  char *path = getenv(ENV_PATH);
  int path_ln = strlen(path);

  char dirpath[256];
  int dirpath_ln = 0;
  for (int i = 0; i < path_ln; ++i) {
    dirpath[dirpath_ln++] = path[i];

    if (path[i] == ':' || i == path_ln - 1) {
      boolean include_last = path[i] == ':';
      dirpath[dirpath_ln - include_last] = '\0';

      DIR *d = opendir(dirpath);
      if (d == null) {
        log_debug("%d: %s does not exist\n", *n, dirpath);
        goto clean;
      }

      struct dirent *de;
      while ((de = readdir(d)) != null) {
        if (de->d_type == DT_DIR)
          continue;

        int d_name_ln = strlen(de->d_name);

        out = realloc(out, (*n + 1) * sizeof(char *));
        out[*n] = malloc((d_name_ln + 1) * sizeof(char));
        strcpy(out[*n], de->d_name);
        ++(*n);
      }

      closedir(d);

    clean:
      dirpath_ln = 0;
    }
  }

  return out;
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

void main_loop(DC *dc, char **bin_names, int bin_names_ln) {
  XEvent e;

  // Nicely waits until the window map occurs.
  while (true) {
    XNextEvent(dc->dpy, &e);
    if (e.type == MapNotify)
      break;
  }

  log_debug("Window is ready\n");

  LC lc = {
    .bin_names = bin_names,
    .bin_names_ln = bin_names_ln,
    .search = (char[1024]){},
    .search_ln = 0,
    .sel_idx = 0,
    .shift_held = false,
  };

  while (true) {
    draw(dc, &lc);
    if (!handle_events(dc, &e, &lc)) {
      break;
    }
  }

  XCloseDisplay(dc->dpy);
}

void draw(DC *dc, LC *lc) {
  XClearWindow(dc->dpy, dc->d);

  XDrawString(dc->dpy, dc->d, dc->gc, 0, 20, lc->search, lc->search_ln);
  XFlush(dc->dpy);

  XDrawLine(dc->dpy, dc->d, dc->gc, 0, 24, WINDOW_WIDTH, 24);
  XFlush(dc->dpy);

  int match_i = 0;
  for (int i = 0; i < lc->bin_names_ln; ++i) {
    if (strstr(lc->bin_names[i], lc->search) == null)
      continue;
    ++match_i;

    int y = 40 + (20 * match_i);
    if (y > WINDOW_HEIGHT)
      continue;

    int ln = strlen(lc->bin_names[i]);
    XDrawString(dc->dpy, dc->d, dc->gc, 0, y, lc->bin_names[i], ln);
  }

  XFlush(dc->dpy);
}

boolean handle_events(DC *dc, XEvent *e, LC *lc) {
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
      if (lc->search_ln > 0) {
        lc->search[--lc->search_ln] = '\0';
      }
      break;
    }

    if (is_shift_key) {
      lc->shift_held = true;
      break;
    }

    KeySym ks = XLookupKeysym(&e->xkey, lc->shift_held);
    if (ks != NoSymbol) {
      log_debug("%c\n", ks);
      lc->search[lc->search_ln++] = ks;
      break;
    }
    break;
  case KeyRelease:
    if (is_shift_key)
      lc->shift_held = false;
    break;
  case FocusOut:
    log_debug("Focus lost.\n");
    return false;
  }

  return true;
}
