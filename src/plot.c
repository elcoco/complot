#include "plot.h"


Plot* plot_init(uint32_t xsize, uint32_t ysize)
{
    Plot* pl = malloc(sizeof(Plot));

    pl->xsize = xsize;
    pl->ysize = ysize;

    pl->status_size = 2;
    pl->xaxis_ysize = 2;

    // TODO do this in one malloc call: pl->cells = malloc(sizeof(Cell*[pl->xsize][pl->ysize]));
    pl->cells = malloc(pl->xsize * sizeof(Cell**));

    for (int x=0 ; x<pl->xsize ; x++) {
        pl->cells[x] = malloc(pl->ysize * sizeof(Cell*));

        for (int y=0 ; y<pl->ysize ; y++)
            pl->cells[x][y] = plot_cell_init(x, y);
    }

    pl->laxis = axis_init(AXIS_LEFT);
    pl->raxis = axis_init(AXIS_RIGHT);
    return pl;
}

void plot_destroy(Plot* pl)
{
    for (int x=0 ; x<pl->xsize ; x++) {
        for (int y=0 ; y<pl->ysize ; y++)
            free(pl->cells[x][y]);
        free(pl->cells[x]);
    }
    free(pl->cells);
    axis_destroy(pl->laxis);
    axis_destroy(pl->raxis);
    free(pl);
}

void plot_draw(Plot* pl, Groups* groups, State* s)
{
    /* Draw all elements to internal matrix */
    // update plot dimensions from axis dimensions
    plot_set_dimensions(pl);
    axis_draw(pl->raxis, pl, s);
    axis_draw(pl->laxis, pl, s);
    plot_draw_xaxis(pl, groups->group);
}

void plot_draw_candlestick(Plot* pl, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose)
{
    // GREEN
    if (iopen < iclose) {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < pl->pystart || y > pl->ysize-1)
                continue;
            Cell* cell = plot_get_cell(pl, ix, y);
            strcpy(cell->chr, CS_WICK);
            cell->fgcol = GREEN;
        }
        for (int y=iopen ; y<=iclose ; y++) {
            if (y < pl->pystart || y > pl->ysize-1)
                continue;
            Cell* cell = plot_get_cell(pl, ix, y);
            strcpy(cell->chr, CS_BODY);
            cell->fgcol = GREEN;
        }
    // RED
    }
    else {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < pl->pystart || y > pl->ysize-1)
                continue;
            Cell* cell = plot_get_cell(pl, ix, y);
            strcpy(cell->chr, CS_WICK);
            cell->fgcol = RED;
        }
        for (int y=iclose ; y<=iopen ; y++) {
            if (y < pl->pystart || y > pl->ysize-1)
                continue;
            Cell* cell = plot_get_cell(pl, ix, y);
            strcpy(cell->chr, CS_BODY);
            cell->fgcol = RED;
        }
    }
}

void plot_draw_candlesticks(Plot* pl, Group* g, Axis* a, int32_t yoffset)
{
    /* itter groups and draw them in plot */
    // start at this xy indices
    uint32_t ix = pl->pxstart;
    uint32_t iy = pl->pystart;
    
    // we have to get more groups from index than we actually need so we need to skip the groups that don't fit in plot
    uint32_t goffset = pl->xsize - pl->pxsize;
    while (goffset != 0) {
        g = g->next;
        goffset--;
    }

    while (g != NULL) {
        if (! g->is_empty) {

            // map data point from data range to terminal rows range
            uint32_t iopen  = map(g->open,  a->dmin, a->dmax, iy, pl->ysize-1) + yoffset;
            uint32_t ihigh  = map(g->high,  a->dmin, a->dmax, iy, pl->ysize-1) + yoffset;
            uint32_t ilow   = map(g->low,   a->dmin, a->dmax, iy, pl->ysize-1) + yoffset;
            uint32_t iclose = map(g->close, a->dmin, a->dmax, iy, pl->ysize-1) + yoffset;

            plot_draw_candlestick(pl, ix, iopen, ihigh, ilow, iclose);
        }
        g = g->next;
        ix++;
    }
}

void plot_draw_xaxis(Plot* pl, Group* g)
{
    int32_t ix = pl->xaxis_xstart;

    uint32_t goffset = pl->xsize - pl->pxsize;
    while (goffset != 0) {
        g = g->next;
        goffset--;
    }

    while (g != NULL) {
        if (g->id % XTICK_SPACING == 0 && !g->is_empty) {

            char dbuf[15] = {'\0'};
            char tbuf[15] = {'\0'};

            char* pdbuf = dbuf;
            char* ptbuf = tbuf;

            ts_to_dt(g->wstart, "%Y-%m-%d", dbuf, sizeof(dbuf));
            ts_to_dt(g->wstart, "%H:%M:%S", tbuf, sizeof(tbuf));

            for (uint32_t x=ix ; x<ix+strlen(dbuf) ; x++, pdbuf++) {
                if (x >= pl->xsize)
                    break;

                Cell* c = plot_get_cell(pl, x, pl->xaxis_ystart);
                c->chr[0] = *pdbuf;
            }

            for (uint32_t x=ix ; x<ix+strlen(tbuf) ; x++, ptbuf++) {
                if (x >= pl->xsize)
                    break;

                Cell* c = plot_get_cell(pl, x, pl->xaxis_ystart+1);
                c->chr[0] = *ptbuf;
            }
        }
        g = g->next;
        ix++;
    }
}

void  plot_clear(Plot* pl)
{
    /* Clear all cells in plot and all limits in axis */
    for (uint32_t x=0 ; x<pl->xsize ; x++) {
        for (uint32_t y=0 ; y<pl->ysize ; y++) {
            Cell* c = plot_get_cell(pl, x, y);
            strcpy(c->chr, EMPTY);
            c->fgcol = WHITE;
        }
    }

    // clear axis limits
    pl->laxis->is_empty = true;

    if (pl->raxis->autorange)
        pl->raxis->is_empty = true;
}

Cell* plot_get_cell(Plot* pl, uint32_t x, uint32_t y)
{
    return pl->cells[x][y];
}

Cell* plot_cell_init(uint32_t x, uint32_t y)
{
    Cell* c = malloc(sizeof(Cell));
    strcpy(c->chr, EMPTY);
    c->fgcol = WHITE;
    c->x = x;
    c->y = y;
    return c;
}

uint32_t plot_set_dimensions(Plot* pl)
{
    /* Set dimensions for plot area and x axis */
    uint8_t spacer = 1;
    pl->pxsize = pl->xsize - pl->laxis->txsize - pl->raxis->txsize - spacer;
    pl->pxstart = pl->laxis->txsize;

    pl->pysize = pl->ysize - pl->status_size - pl->xaxis_ysize;

    pl->xaxis_xsize = pl->pxsize;
    pl->xaxis_xstart = pl->laxis->txsize;
    pl->xaxis_ystart = pl->status_size;

    pl->pxstart = pl->laxis->txsize;
    pl->pystart = pl->status_size + pl->xaxis_ysize;
    return 0;
}

void plot_print(Plot* pl)
{
    for (int y=pl->ysize-1 ; y>=0 ; y--) {
        for (int x=0 ; x<pl->xsize ; x++) {
            printf("%d%s%s",pl->cells[x][y]->fgcol, pl->cells[x][y]->chr, RESET);
        }
        printf("\n");
    }
}
