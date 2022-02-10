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
    uint8_t bgcolor;
    uint8_t fgcolor;

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

    void(*vp_print)(ViewPort* self);
    void(*vp_free)(ViewPort* self);
    //int8_t(*vp_insert)(ViewPort* self, Cell* c);
    Cell*(*vp_get_cell)(ViewPort* self, uint32_t x, uint32_t y);
    void(*vp_draw_candlestick)(ViewPort* self, Group* g, uint32_t ix, double dmin, double dmax);
};

ViewPort* vp_init(uint32_t xsize, uint32_t ysize);
Cell* cell_init();

#endif
