#ifndef LEGEND_H
#define LEGEND_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <curses.h>
#include "assert.h"

#include "yaxis.h"

// forward declare
typedef struct Yaxis Yaxis;

typedef struct Legend Legend;
struct Legend {
    WINDOW* win;
    int xsize;
    int ysize;
    Yaxis* yaxis;
    uint32_t bgcol;
};

Legend* legend_init(Yaxis* ya);
void legend_draw(Legend* le);
void legend_destroy(Legend* le);

#endif
