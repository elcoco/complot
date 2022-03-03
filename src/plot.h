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


#define MIN_WINDOW_XSIZE 10
#define MIN_WINDOW_YSIZE 10

#define LEGEND_MAX_SIZE 200
#define UTF8_MAX_LEN 3


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
    WINDOW* parent;

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


Plot*  plot_init(WINDOW* parent);
void   plot_destroy(Plot* pl);
void   plot_draw(Plot* pl, State* s);
int8_t plot_resize(Plot* pl);
void   plot_draw_xaxis(Plot* pl, Group* g);

Graph* graph_init();

#endif
