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
#include "config.h"

typedef struct Yaxis Yaxis;

// forward declare
typedef struct Line Line;
typedef enum PlotStatus PlotStatus;

typedef enum AxisSide {
    AXIS_LEFT,
    AXIS_RIGHT
} AxisSide;

typedef enum PType {
    DPOINT,
    XINT_POINT,
    YINT_POINT
} PType;

// orientation between 2 points, is used for choosing th eright line drawing character
typedef enum POrientation {
    PO_N,
    PO_NE,
    PO_E,
    PO_SE,
    PO_S,
    PO_SW,
    PO_W,
    PO_NW,
} POrientation;

typedef struct InterpolateXY InterpolateXY;
struct InterpolateXY {
    int32_t x;
    int32_t y;
    InterpolateXY* next;
    InterpolateXY* prev;
    PType ptype;
};

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

    uint32_t bgcol;
};

Yaxis* yaxis_init(WINDOW* parent, AxisSide side);
void yaxis_destroy(Yaxis* a);
void yaxis_draw(Yaxis* a, WINDOW* wtarget, int32_t pany);
void yaxis_draw_line(Yaxis* a, Line* l, WINDOW* wtarget, Group* g, int32_t yoffset);
void yaxis_draw_candlesticks(Yaxis* a, WINDOW* wtarget, Group* g, int32_t yoffset);
void yaxis_draw_candlestick(WINDOW* win, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose);
void yaxis_draw_tickers(Yaxis* a, int32_t yoffset);
PlotStatus yaxis_set_window_width(Yaxis* a, uint32_t yoffset);

GroupContainer* yaxis_get_gc(Yaxis* a);

void yaxis_add_line(Yaxis* a, Line* l);
void yaxis_draw_last_data(Yaxis* a, WINDOW* wgraph, double pany, double lasty);

void get_tickerstr(char* buf, double ticker, uint32_t ntotal, uint32_t nwhole, uint32_t nfrac);

#endif
