#ifndef UI_H
#define UI_H

#define _XOPEN_SOURCE_EXTENDED 1

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

typedef struct Cell {
    char chr[5];
    uint32_t fgcol;
    uint32_t bgcol;
} Cell;

typedef struct Matrix {
    int32_t xsize;
    int32_t ysize;
    Cell** cells;
    bool is_erased;
} Matrix;


int ui_init();
void ui_init_colors();
void ui_cleanup();

void set_color(WINDOW* win, uint32_t fgcolor, uint32_t bgcolor);
void unset_color(WINDOW* win, uint32_t fgcolor, uint32_t bgcolor);

int add_chr(WINDOW* win, int32_t y, int32_t x, int32_t fgcol, int32_t bgcol, char c);
int add_str(WINDOW* win, int32_t y, int32_t x, int32_t fgcol, int32_t bgcol, char* fmt, ...);
//int add_str_color(WINDOW* win, int32_t y, int32_t x, int32_t fgcol, int32_t bgcol, char* fmt, ...);
int add_str_color(WINDOW* win, int32_t y, int32_t x, int32_t fgcol, int32_t bgcol, bool in_bg, char* fmt, ...);

//void ui_show_error(WINDOW* win, char* msg);
void ui_show_error(WINDOW* win, char* fmt, ...);

Matrix* ui_matrix_init(int32_t xsize, int32_t ysize);
Cell* ui_matrix_get(Matrix* m, int32_t y, int32_t x);
void ui_matrix_set(Matrix* m, int32_t y, int32_t x, char* chr, int32_t fgcol, int32_t bgcol);
void ui_matrix_destroy(Matrix* m);

int set_status(uint32_t lineno, char* fmt, ...);

void ui_refresh(WINDOW* win);
void ui_erase(WINDOW* win);

#endif
