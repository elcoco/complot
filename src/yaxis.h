#ifndef YAXIS_H
#define YAXIS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "assert.h"

#include "plot.h"
#include "utils.h"
#include "index.h"
#include "line.h"

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
    AxisSide side;

    // ticker axis size in matrix
    uint32_t txsize;
    uint32_t tysize;
    uint32_t txstart;

    // amount of digits before and after the dot
    uint32_t nwhole;
    uint32_t nfrac;

    // data dimensions
    // TODO rename to dymin/dymax
    double dmin;
    double dmax;

    // last y data
    double dlast;

    // linked list of Line structs that should be drawn into this axis
    Line* line;
    Line** ltail;

    bool is_empty;
};

Axis* axis_init(AxisSide side);
void  axis_destroy(Axis* a);
void  axis_add_line(Axis* a, Line* l);
void  axis_draw(Axis* a, Plot* pl, State* s);
void  axis_draw_tickers(Axis* a, Plot* pl, int32_t yoffset);
void axis_draw_last_data(Axis* a, Plot* pl, double pany, double lasty);

void axis_set_data(Axis* a, Line* l, Groups* groups);

void get_tickerstr(char* buf, double ticker, uint32_t ntotal, uint32_t nwhole, uint32_t nfrac);

#endif
