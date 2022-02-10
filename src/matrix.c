#include "matrix.h"

void vp_print(ViewPort* vp)
{
    for (int y=vp->ysize-1 ; y>=0 ; y--) {
        for (int x=0 ; x<vp->xsize ; x++) {
            printf("%s%s%s",vp->cells[x][y]->fgcol, vp->cells[x][y]->chr, RESET);
        }
        printf("\n");
    }
}

void vp_draw_candlestick(ViewPort* vp, Group* g, uint32_t ix, double dmin, double dmax)
{
    // map value from data range to terminal rows range
    int32_t iopen  = map(g->open,  dmin, dmax, 0, vp->ysize-1);
    int32_t ihigh  = map(g->high,  dmin, dmax, 0, vp->ysize-1);
    int32_t ilow   = map(g->low,   dmin, dmax, 0, vp->ysize-1);
    int32_t iclose = map(g->close, dmin, dmax, 0, vp->ysize-1);

    // draw wick

    // GREEN
    if (iopen < iclose) {
        for (int y=ilow ; y<=ihigh ; y++) {
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_WICK);
            strcpy(cell->fgcol, GREEN);
        }

        for (int y=iopen ; y<=iclose ; y++) {
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_BODY);
            strcpy(cell->fgcol, GREEN);
        }


    // RED
    } else {
        for (int y=ilow ; y<=ihigh ; y++) {
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_WICK);
            strcpy(cell->fgcol, RED);
        }
        for (int y=iopen ; y>=iclose ; y--) {
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_BODY);
            strcpy(cell->fgcol, RED);
        }

    }
}

Cell* vp_get_cell(ViewPort* vp, uint32_t x, uint32_t y)
{
    return vp->cells[x][y];
}

ViewPort* vp_init(uint32_t xsize, uint32_t ysize)
{
    ViewPort* vp = malloc(sizeof(ViewPort*));

    vp->xsize = xsize;
    vp->ysize = ysize;

    int x, y;

    // TODO do this in one malloc call: vp->cells = malloc(sizeof(Cell*[vp->xsize][vp->ysize]));
    vp->cells = malloc(vp->xsize * sizeof(Cell**));

    for (x=0 ; x<vp->xsize ; x++) {
        vp->cells[x] = malloc(vp->ysize * sizeof(Cell*));

        for (y=0 ; y<vp->ysize ; y++) {
            Cell* cell = malloc(sizeof(Cell));
            cell->chr = strdup(".");
            cell->fgcol = strdup("");
            cell->x = x;
            cell->y = y;
            vp->cells[x][y] = cell;
        }
    }

    // func pointers
    //vp->vp_print = vp_print;
    //vp->vp_free = vp_free;
    //vp->vp_get_cell = vp_get_cell;
    //vp->vp_draw_candlestick = vp_draw_candlestick;

    return vp;
}
