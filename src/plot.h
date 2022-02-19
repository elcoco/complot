#ifndef PLOT_H
#define PLOT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "assert.h"

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

    // width of axis
    int lyaxis_size;
    int ryaxis_size;

    int ryaxis_nfrac;       // digits after decimal point
    int ryaxis_nwhole;      // digits before decimal point

    int lyaxis_start;
    int ryaxis_start;

    int xaxis_xsize;
    int xaxis_xstart;
    int xaxis_ystart;
    int xaxis_ysize;

    // no of lines in status bar
    uint32_t status_size;
};

void pl_print(Plot* pl);
Cell* pl_get_cell(Plot* pl, uint32_t x, uint32_t y);
void pl_draw_candlestick(Plot* pl, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose);
void pl_draw_candlesticks(Plot* pl, Group* g, double dmin, double dmax, int32_t yoffset);
void pl_clear_cells(Plot* pl);
void pl_draw_ryaxis(Plot* pl, double dmin, double dmax, int32_t yoffset);
void pl_draw_last_data(Plot* pl, double dmin, double dmax, double pany, double lasty);
void pl_draw_xaxis(Plot* pl, Group* g);
void pl_set_dimensions(Plot* pl, double dmin, double dmax);
void pl_draw(Plot* pl, Index* index, Groups* groups, State* s);

Plot* pl_init(uint32_t xsize, uint32_t ysize);
void pl_destroy(Plot* pl);
Cell* pl_cell_init(uint32_t x, uint32_t y);

void get_tickerstr(char* buf, double ticker, uint32_t ntotal, uint32_t nwhole, uint32_t nfrac);

#endif
