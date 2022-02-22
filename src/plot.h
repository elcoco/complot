#ifndef PLOT_H
#define PLOT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <curses.h>
#include "assert.h"

#include "yaxis.h"
#include "utils.h"
#include "index.h"

#define CS_BODY "█"
#define CS_WICK "┃"
#define EMPTY   " "


#define RED   1
#define GREEN 2
#define WHITE 7
#define MAGENTA 5
//#define RED   "\033[0;31m"
//#define GREEN "\033[0;32m"
#define RESET "\033[0m"

// data dimensions must have a middle point so should be an odd number
#define MATRIX_WIDTH  1001
#define MATRIX_HEIGHT 1001

#define RAXIS_SIZE 5

// chars inbetween tickers on x axis
#define XTICK_SPACING 15

// Node struct represents a cell in the matrix
typedef struct Cell Cell;
typedef struct Plot Plot;
typedef struct Graph Graph;

// forward declare from yaxis.h and line.h
typedef struct Axis Axis;
typedef struct Line Line;

// Represents a cell in the matrix and viewport
struct Cell {
    int32_t x;
    int32_t y;

    char chr[5];
    uint16_t bgcol;
    uint16_t fgcol;

    // keep changed cells in linked list for easy updating
    Cell* prev;
    Cell* next;
};

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
    WINDOW* win;

    Graph* graph;
    Axis* lyaxis;
    Axis* ryaxis;

    // total plot dimensions
    int xsize;
    int ysize;

    //int xaxis_xsize;
    //int xaxis_xstart;
    //int xaxis_ystart;
    //int xaxis_ysize;
};

Plot* plot_init(WINDOW* parent, uint32_t ysize);
void  plot_destroy(Plot* pl);

void plot_draw(Plot* pl, Groups* groups, State* s);

Graph* graph_init(WINDOW* parent, uint32_t xstart, uint32_t xsize);


void  plot_draw_xaxis(Plot* pl, Group* g);
//uint32_t plot_set_dimensions(Plot* pl);
//void  plot_clear(Plot* pl);

#endif
