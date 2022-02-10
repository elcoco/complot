#include "matrix.h"

/*
int8_t vp_insert(ViewPort* self, Cell* cell)
{
    printf("Do some cell inserting here\n");
    self->cells[cell->x][cell->y] = cell;
}

Cell* cell_init()
{
    Cell* cell = (Cell*)malloc(sizeof(Cell));
    cell->chr = strdup(CELL_CHR);
    return cell;
}
*/

void vp_print(ViewPort* self)
{
    for (int y=self->ysize-1 ; y>=0 ; y--) {
        for (int x=0 ; x<self->xsize ; x++) {
            printf("%s", self->cells[x][y]->chr);
        }
        printf("\n");
    }
}

void vp_free(ViewPort* self)
{
    free(self);
}

void vp_draw_candlestick(ViewPort* self, Group* g, uint32_t ix, double dmin, double dmax)
{
    // TODO why is all data in group 0???
    printf("before draw: %9f %9f %9f %9f\n", g->open, g->high, g->low, g->close);
    int32_t iopen  = map(g->open,  dmin, dmax, 0, self->ysize);
    int32_t ihigh  = map(g->high,  dmin, dmax, 0, self->ysize);
    int32_t ilow   = map(g->low,   dmin, dmax, 0, self->ysize);
    int32_t iclose = map(g->close, dmin, dmax, 0, self->ysize);
    printf("%d %d %d %d\n", iopen, ihigh, ilow, iclose);

    // draw wick
    for (int y=ilow ; y<=ihigh ; y++) {
        Cell* cell = self->vp_get_cell(self, ix, y);
        strcpy(cell->chr, CS_WICK);
    }

    // draw body
    int incr = (iclose > iopen) ? 1 : -1;
    for (int y=iopen ; y<=iclose ; y+=incr) {
        Cell* cell = self->vp_get_cell(self, ix, y);
        strcpy(cell->chr, CS_BODY);
    }

}

Cell* vp_get_cell(ViewPort* self, uint32_t x, uint32_t y)
{
    return self->cells[x][y];
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
            cell->x = x;
            cell->y = y;
            vp->cells[x][y] = cell;
        }
    }


    // func pointers
    vp->vp_print = vp_print;
    vp->vp_free = vp_free;
    vp->vp_get_cell = vp_get_cell;
    vp->vp_draw_candlestick = vp_draw_candlestick;

    return vp;
}
