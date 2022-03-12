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
#include "plotwin.h"

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
};

typedef enum PlotError {
    PLOT_ERR_NO_DATA          = -3,
    PLOT_ERR_TOO_SMALL        = -2,
    PLOT_ERR_RESIZE_FAILED    = -1,
    PLOT_SUCCESS              = 0
} PlotError;


Plot*  plot_init(WINDOW* parent);
void   plot_destroy(Plot* pl);
PlotError plot_draw(Plot* pl, State* s);
int8_t plot_resize(Plot* pl);
void   plot_draw_xaxis(Plot* pl, Group* g);

Graph* graph_init();

#endif
