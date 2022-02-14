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

#define RED   1
#define GREEN 2
#define WHITE 7
//#define RED   "\033[0;31m"
//#define GREEN "\033[0;32m"
#define RESET "\033[0m"

// data dimensions must have a middle point so should be an odd number
#define MATRIX_WIDTH  1001
#define MATRIX_HEIGHT 1001

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
    // cols and rows sizes
    int xsize;
    int ysize;

    // 2d array representing data on screen
    // every index has a cell struct
    // if there are more than one cells (linked list) it means
    // that we have lines drawing on top of other lines
    // In this case do something cool with color and chars
    Cell*** cells;

    // bottom right coordinates
    int xend;
    int yend;

    // limits of data on y axis. New points need to be mapped to index on screen
    int32_t dymin;
    int32_t dymax;

    //void(*vp_print)(ViewPort* vp);
    //void(*vp_free)(ViewPort* vp);
    //Cell*(*vp_get_cell)(ViewPort* vp, uint32_t x, uint32_t y);
    //void(*vp_draw_candlestick)(ViewPort* vp, Group* g, uint32_t ix, double dmin, double dmax);
};

void vp_print(ViewPort* vp);
Cell* vp_get_cell(ViewPort* vp, uint32_t x, uint32_t y);
void vp_draw_candlestick(ViewPort* vp, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose);
void vp_draw_candlesticks(ViewPort* vp, Groups* groups, double dmin, double dmax, int32_t yoffset);
void vp_clear_cells(ViewPort* vp);

ViewPort* vp_init(uint32_t xsize, uint32_t ysize);
Cell* cell_init();

#endif
