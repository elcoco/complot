#ifndef PLOT_H
#define PLOT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <curses.h>
#include "assert.h"

#include "yaxis.h"
#include "xaxis.h"
#include "utils.h"
#include "index.h"
#include "status.h"
#include "legend.h"

// Node struct represents a cell in the matrix
typedef struct Plot Plot;
typedef struct Graph Graph;
typedef struct Legend Legend;

// forward declare from yaxis.h and line.h
typedef struct Yaxis Yaxis;
typedef struct Xaxis Xaxis;

// forward declare from status.h
typedef struct StatusBar StatusBar;


struct Graph {
    WINDOW* win;
    int xsize;
    int ysize;
};

// Represents the nodes that are visible on screen
// Holds a subset of Matrix->nodes,
struct Plot {
    // main window containing the subwindows
    WINDOW* win;

    Graph* graph;
    Yaxis*  lyaxis;
    Yaxis*  ryaxis;
    Xaxis* xaxis;
    Legend* llegend;
    Legend* rlegend;
    StatusBar* status;

    // total plot dimensions
    int xsize;
    int ysize;

    bool show_status;
    bool show_legend;
    bool show_xaxis;
    bool show_grid;

    // y offset when drawing
    int32_t pany;
};

typedef enum PlotStatus {
    PLERR_NO_DATA            = -4,
    PLERR_WINDOW_TOO_SMALL   = -3,
    PLERR_RESIZE_FAILED      = -2,
    PLSUCCESS                =  0,
    PLSTATUS_YAXIS_UNCHANGED =  1,
    PLSTATUS_YAXIS_CHANGED   =  2
} PlotStatus;


Plot*      plot_init(WINDOW* parent);
void       plot_destroy(Plot* pl);
PlotStatus plot_draw(Plot* pl);
PlotStatus plot_resize(Plot* pl);
void       plot_draw_xaxis(Plot* pl, Group* g);
void       plot_print_error(PlotStatus plstatus);

Graph* graph_init();

#endif
