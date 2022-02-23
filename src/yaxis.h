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

typedef struct Axis Axis;

// forward declare from plot.h
typedef struct Plot Plot;
typedef struct Line Line;

typedef enum AxisSide {
    AXIS_LEFT,
    AXIS_RIGHT
} AxisSide;

struct Axis {
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

Axis* axis_init(WINDOW* parent, AxisSide side, uint32_t ysize);
void axis_destroy(Axis* a);
void axis_draw(Axis* a, WINDOW* wtarget, Groups* groups, State* s);
void axis_draw_candlesticks(Axis* a, WINDOW* wtarget, Group* g, int32_t yoffset);
void axis_draw_candlestick(WINDOW* win, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose);
void axis_draw_tickers(Axis* a, int32_t yoffset);
int8_t axis_set_window_width(Axis* a);


void  axis_add_line(Axis* a, Line* l);
//void  axis_draw(Axis* a, Plot* pl, State* s);
//void  axis_draw_tickers(Axis* a, Plot* pl, int32_t yoffset);
void axis_draw_last_data(Axis* a, WINDOW* wgraph, double pany, double lasty);

void get_tickerstr(char* buf, double ticker, uint32_t ntotal, uint32_t nwhole, uint32_t nfrac);

#endif
