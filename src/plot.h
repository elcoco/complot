#ifndef PLOT_H
#define PLOT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "assert.h"

#include "yaxis.h"
#include "utils.h"
#include "index.h"

#define CS_BODY "█"
#define CS_WICK "┃"
#define EMPTY   " "
#define LINE_CHR "─"

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

// Represents the nodes that are visible on screen
// Holds a subset of Matrix->nodes,
struct Plot {
    // total area dimensions
    int xsize;
    int ysize;

    // plot area dimensions
    int pxsize;
    int pysize;

    // coordinates of bottom left corner of plot area
    int pxstart;
    int pystart;

    // 2d array representing data on screen
    // every index has a cell struct
    // if there are more than one cells (linked list) it means
    // that we have lines drawing on top of other lines
    // In this case do something cool with color and chars
    Cell*** cells;

    int xaxis_xsize;
    int xaxis_xstart;
    int xaxis_ystart;
    int xaxis_ysize;

    // no of lines in status bar
    uint32_t status_size;

    Axis* laxis;
    Axis* raxis;
};

Cell* plot_cell_init(uint32_t x, uint32_t y);
Cell* plot_get_cell(Plot* pl, uint32_t x, uint32_t y);

Plot* plot_init(uint32_t xsize, uint32_t ysize);
void  plot_destroy(Plot* pl);
void  plot_print(Plot* pl);
void  plot_draw_candlesticks(Plot* pl, Group* g, Axis* a, int32_t yoffset);
void  plot_draw_candlestick(Plot* pl, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose);
void  plot_draw_last_data(Plot* pl, double dmin, double dmax, double pany, double lasty);
void  plot_draw_xaxis(Plot* pl, Group* g);
void  plot_draw(Plot* pl, Groups* groups, State* s);
uint32_t plot_set_plot_dimensions(Plot* pl);

#endif
