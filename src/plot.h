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

#define CS_BODY "█"
#define CS_WICK "┃"
#define EMPTY   " "

#define MIN_WINDOW_XSIZE 10
#define MIN_WINDOW_YSIZE 10

#define RED   1
#define GREEN 2
#define WHITE 7
#define MAGENTA 5

// chars inbetween tickers on x axis
#define XTICK_SPACING 15

// Node struct represents a cell in the matrix
typedef struct Plot Plot;
typedef struct Graph Graph;

// forward declare from yaxis.h and line.h
typedef struct Yaxis Yaxis;

struct Graph {
    WINDOW* parent;
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
    WINDOW* wplot;

    Graph* graph;
    Yaxis*  lyaxis;
    Yaxis*  ryaxis;
    Xaxis* xaxis;

    // total plot dimensions
    int xsize;
    int ysize;
};

Plot* plot_init(WINDOW* parent);
void  plot_destroy(Plot* pl);

void   plot_draw(Plot* pl, Groups* groups, State* s);
int8_t plot_resize(Plot* pl);
void   plot_draw_xaxis(Plot* pl, Group* g);

Graph* graph_init(WINDOW* parent, uint32_t ysize, uint32_t xsize, uint32_t xstart);

#endif
