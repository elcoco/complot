#ifndef MATRIX_H
#define MATRIX_H

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
#define XTICK_SPACING 10

//const int matrix_x = (MATRIX_WIDTH % 2 == 0)  ? MATRIX_WIDTH + 1 : MATRIX_WIDTH;
//const int matrix_y = (MATRIX_HEIGHT % 2 == 0) ? MATRIX_HEIGHT + 1 : MATRIX_HEIGHT;

// Node struct represents a cell in the matrix
typedef struct Cell Cell;
typedef struct ViewPort ViewPort;
typedef struct Matrix Matrix;

// Represents a cell in the matrix and viewport
struct Cell {
    int32_t x;
    int32_t y;

    char* chr;
    uint16_t bgcol;
    uint16_t fgcol;

    // keep changed cells in linked list for easy updating
    Cell* prev;
    Cell* next;
};

// Represents the nodes that are visible on screen
// Holds a subset of Matrix->nodes,
struct ViewPort {
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

void vp_print(ViewPort* vp);
Cell* vp_get_cell(ViewPort* vp, uint32_t x, uint32_t y);
void vp_draw_candlestick(ViewPort* vp, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose);
void vp_draw_candlesticks(ViewPort* vp, Groups* groups, double dmin, double dmax, int32_t yoffset);
void vp_clear_cells(ViewPort* vp);
void vp_draw_ryaxis(ViewPort* vp, State* s);
void vp_draw_last_data(ViewPort* vp, State* s, double lasty);
void vp_draw_xaxis(ViewPort* vp, State* s, Groups* groups);

ViewPort* vp_init(uint32_t xsize, uint32_t ysize);
Cell* cell_init();

#endif
