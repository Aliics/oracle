#ifndef _ORACLE_H_
#define _ORACLE_H_

#include <X11/Xlib.h>
#define true 1
#define false 0

#define null 0

typedef int boolean;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define KEYCODE_EXIT 9 // Escape
#define KEYCODE_LSHIFT 50
#define KEYCODE_RSHIFT 62
#define KEYCODE_BACKSPACE 22

#define ENV_PATH "PATH"
char **get_binary_names(int *);

typedef struct DC {
  Display *dpy;
  Drawable d;
  GC gc;
  unsigned int scr;
} DC;

void init_dc(DC *);

void main_loop(DC *);

void draw(DC *, const char *, int);

boolean handle_events(DC *, XEvent *, char *, int *, boolean *);
#endif
