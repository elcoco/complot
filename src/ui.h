#ifndef UI_H
#define UI_H

#include <curses.h>
#include <sys/time.h>   // for non blocking sleep
#include <stdbool.h>
#include <unistd.h>     // usleep

#include "plot.h"
#include "config.h"

#define CRED     1
#define CGREEN   2
#define CYELLOW  3
#define CBLUE    4
#define CMAGENTA 5
#define CCYAN    6
#define CWHITE   7
#define CBLACK   8



int init_ui();
void cleanup_ui();
void init_colors();
int set_status(uint32_t lineno, char* fmt, ...);
int add_str(WINDOW* win, uint32_t y, uint32_t x, uint32_t color, char* fmt, ...);
int add_chr(WINDOW* win, uint32_t y, uint32_t x, uint32_t color, char c);
void clear_win(WINDOW* win);


#endif
