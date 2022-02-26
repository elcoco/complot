#ifndef YAXIS_H
#define YAXIS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <curses.h>
#include "assert.h"

#include "plot.h"
#include "utils.h"
#include "index.h"
#include "line.h"
#include "ui.h"

#define LINE_CHR "─"

typedef struct Yaxis Yaxis;

// forward declare from plot.h
typedef struct Line Line;

typedef enum AxisSide {
    AXIS_LEFT,
    AXIS_RIGHT
} AxisSide;

struct Yaxis {
    WINDOW* parent;
    WINDOW* win;

    int xsize;
    int ysize;

    AxisSide side;

    // amount of digits before and after the dot
    uint32_t nwhole;
    uint32_t nfrac;

    // Total axis data min/max
    double tdmin;
    double tdmax;

    // View or visible axis data min/max
    double vdmin;
    double vdmax;

    // Currently used axis limits for drawing tickers and candlesticks
    double dmin;
    double dmax;

    // linked list of Line structs that should be drawn into this axis
    Line* line;
    Line** ltail;

    bool is_empty;

    bool autorange;
};

Yaxis* yaxis_init(WINDOW* parent, AxisSide side);
void yaxis_destroy(Yaxis* a);
void yaxis_draw(Yaxis* a, WINDOW* wtarget, Groups* groups, State* s);
void yaxis_draw_candlesticks(Yaxis* a, WINDOW* wtarget, Group* g, int32_t yoffset);
void yaxis_draw_candlestick(WINDOW* win, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose);
void yaxis_draw_tickers(Yaxis* a, int32_t yoffset);
int8_t yaxis_set_window_width(Yaxis* a);

void yaxis_add_line(Yaxis* a, Line* l);
void yaxis_draw_last_data(Yaxis* a, WINDOW* wgraph, double pany, double lasty);

void get_tickerstr(char* buf, double ticker, uint32_t ntotal, uint32_t nwhole, uint32_t nfrac);

#endif
