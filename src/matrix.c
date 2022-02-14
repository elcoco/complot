#include "matrix.h"

uint32_t global_y = 0;

void vp_print(ViewPort* vp)
{
    for (int y=vp->ysize-1 ; y>=0 ; y--) {
        for (int x=0 ; x<vp->xsize ; x++) {
            printf("%d%s%s",vp->cells[x][y]->fgcol, vp->cells[x][y]->chr, RESET);
        }
        printf("\n");
    }
}

//void vp_draw_candlestick(ViewPort* vp, uint32_t ix, uint32_t iopen, uint32_t ihigh, uint32_t ilow, uint32_t iclose)
void vp_draw_candlestick(ViewPort* vp, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose)
{
    //set_status(1, "%d %d %d %d %d", ix, iopen, ihigh, ilow, iclose);
    /* draw one candlestick in viewport */
    // GREEN
    if (iopen < iclose) {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < 0 || y > vp->ysize-1)
                continue;
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_WICK);
            cell->fgcol = GREEN;
        }
        for (int y=iopen ; y<=iclose ; y++) {
            if (y < 0 || y > vp->ysize-1)
                continue;
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_BODY);
            cell->fgcol = GREEN;
        }

    // RED
    }
    else {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < 0 || y > vp->ysize-1)
                continue;
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_WICK);
            cell->fgcol = RED;
        }
        for (int y=iclose ; y<=iopen ; y++) {
            if (y < 0 || y > vp->ysize-1)
                continue;
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_BODY);
            cell->fgcol = RED;
        }
    }
}

void vp_draw_candlesticks(ViewPort* vp, Groups* groups, double dmin, double dmax, int32_t yoffset)
{
    /* itter groups and draw them in viewport */
    Group* g = groups->group;
    uint32_t ix = 0;

    while (g != NULL) {
        if (! g->is_empty) {

            // map data point from data range to terminal rows range
            uint32_t iopen  = map(g->open,  dmin, dmax, 0, vp->ysize-1) + yoffset;
            uint32_t ihigh  = map(g->high,  dmin, dmax, 0, vp->ysize-1) + yoffset;
            uint32_t ilow   = map(g->low,   dmin, dmax, 0, vp->ysize-1) + yoffset;
            uint32_t iclose = map(g->close, dmin, dmax, 0, vp->ysize-1) + yoffset;


            vp_draw_candlestick(vp, ix, iopen, ihigh, ilow, iclose);
        }
        g = g->next;
        ix++;
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
            cell->chr = strdup(EMPTY);
            cell->fgcol = WHITE;
            cell->x = x;
            cell->y = y;
            vp->cells[x][y] = cell;
        }
    }
    return vp;
}
