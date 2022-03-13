#ifndef UI_H
#define UI_H

#include <curses.h>
#include <sys/time.h>   // for non blocking sleep
#include <stdbool.h>
#include <unistd.h>     // usleep

#include "plot.h"
#include "config.h"

#define CDEFAULT 1
#define CRED     2
#define CGREEN   3
#define CYELLOW  4
#define CBLUE    5
#define CMAGENTA 6
#define CCYAN    7
#define CWHITE   8
#define CBLACK   9

static const int8_t ccolors[] = {-1, COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE, COLOR_BLACK}; 
static const uint8_t ncolors = 9;


int init_ui();
void cleanup_ui();
void init_colors();
int set_status(uint32_t lineno, char* fmt, ...);
int add_str(WINDOW* win, uint32_t y, uint32_t x, uint32_t fgcol, uint32_t bgcol, char* fmt, ...);
int add_chr(WINDOW* win, uint32_t y, uint32_t x, uint32_t fgcol, uint32_t bgcol, char c);
void clear_win(WINDOW* win);
void set_color(WINDOW* win, uint32_t fgcolor, uint32_t bgcolor);


#endif
