#ifndef XAXIS_H
#define XAXIS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <curses.h>
#include "assert.h"

#include "utils.h"
#include "index.h"
#include "ui.h"
#include "config.h"

// chars inbetween tickers on x axis


typedef struct Xaxis Xaxis;

struct Xaxis {
    WINDOW* win;

    int xsize;
    int ysize;

    uint32_t fgcol;
    uint32_t bgcol;
};

Xaxis* xaxis_init();
void   xaxis_destroy(Xaxis* xa);
void xaxis_draw(Xaxis* xa, Group* g, uint32_t xstart, uint32_t width);
void xaxis_draw_grid(Xaxis* xa, WINDOW* wtarget, Group* g);

#endif
