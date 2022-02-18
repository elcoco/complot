#include "plot.h"


void pl_print(Plot* pl)
{
    for (int y=pl->ysize-1 ; y>=0 ; y--) {
        for (int x=0 ; x<pl->xsize ; x++) {
            printf("%d%s%s",pl->cells[x][y]->fgcol, pl->cells[x][y]->chr, RESET);
        }
        printf("\n");
    }
}

//void pl_draw_candlestick(Plot* pl, uint32_t ix, uint32_t iopen, uint32_t ihigh, uint32_t ilow, uint32_t iclose)
void pl_draw_candlestick(Plot* pl, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose)
{
    //set_status(1, "%d %d %d %d %d", ix, iopen, ihigh, ilow, iclose);
    /* draw one candlestick in viewport */
    // GREEN
    //ilow = (ilow < 0) ? 0 : ilow;
    //ihigh = (ihigh > pl->ysize-1) ? pl->ysize-1 : ihigh;

    if (iopen < iclose) {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < pl->pystart || y > pl->ysize-1)
                continue;
            Cell* cell = pl_get_cell(pl, ix, y);
            strcpy(cell->chr, CS_WICK);
            cell->fgcol = GREEN;
        }
        for (int y=iopen ; y<=iclose ; y++) {
            if (y < pl->pystart || y > pl->ysize-1)
                continue;
            Cell* cell = pl_get_cell(pl, ix, y);
            strcpy(cell->chr, CS_BODY);
            cell->fgcol = GREEN;
        }

    // RED
    }
    else {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < pl->pystart || y > pl->ysize-1)
                continue;
            Cell* cell = pl_get_cell(pl, ix, y);
            strcpy(cell->chr, CS_WICK);
            cell->fgcol = RED;
        }
        for (int y=iclose ; y<=iopen ; y++) {
            if (y < pl->pystart || y > pl->ysize-1)
                continue;
            Cell* cell = pl_get_cell(pl, ix, y);
            strcpy(cell->chr, CS_BODY);
            cell->fgcol = RED;
        }
    }
}

void pl_draw_candlesticks(Plot* pl, Group* g, double dmin, double dmax, int32_t yoffset)
{
    /* itter groups and draw them in viewport */
    // start at this xy indices
    uint32_t ix = pl->pxstart;
    uint32_t iy = pl->pystart;

    
    uint32_t goffset = pl->xsize - pl->pxsize;
    while (goffset != 0) {
        g = g->next;
        goffset--;
    }

    while (g != NULL) {
        if (! g->is_empty) {

            // map data point from data range to terminal rows range
            uint32_t iopen  = map(g->open,  dmin, dmax, iy, pl->ysize-1) + yoffset;
            uint32_t ihigh  = map(g->high,  dmin, dmax, iy, pl->ysize-1) + yoffset;
            uint32_t ilow   = map(g->low,   dmin, dmax, iy, pl->ysize-1) + yoffset;
            uint32_t iclose = map(g->close, dmin, dmax, iy, pl->ysize-1) + yoffset;

            pl_draw_candlestick(pl, ix, iopen, ihigh, ilow, iclose);
        }
        g = g->next;
        ix++;
    }
}

void pl_draw_last_data(Plot* pl, double dmin, double dmax, double pany, double lasty)
{
    /* highlight last data in axis */

    // calculate first column of axis
    //uint32_t xstart = pl->xsize - pl->ryaxis_size -1;
    uint32_t xstart = pl->ryaxis_start;

    // calculate y index of last data
    int32_t ilasty = map(lasty, dmin, dmax, pl->pystart, pl->ysize-1) + pany;

    char buf[50] = {'\0'};
    get_tickerstr(buf, lasty, pl->ryaxis_size, pl->ryaxis_nwhole, pl->ryaxis_nfrac);
    char* pbuf = buf;

    // if last data is out of range, stick to top/bottom
    if (ilasty < pl->pystart)
        ilasty = pl->pystart;
    if (ilasty >= pl->ysize)
        ilasty = pl->ysize-1;

    for (int32_t ix=xstart ; ix<xstart+strlen(buf) ; ix++, pbuf++) {
        if (ix >= pl->xsize)
            break;
        Cell* c = pl_get_cell(pl, ix, ilasty);
        c->chr[0] = *pbuf;
        c->fgcol = GREEN;
    }
    // draw line
    for (int32_t ix=pl->pxstart ; ix<pl->pxstart+pl->pxsize ; ix++, pbuf++) {
        Cell* c = pl_get_cell(pl, ix, ilasty);
        if (c->chr[0] == ' ') {
            strcpy(c->chr, LINE_CHR);
            c->fgcol = MAGENTA;
        }
    }
}

void pl_draw_xaxis(Plot* pl, Group* g)
{
    int32_t ix = pl->xaxis_xstart;

    // TODO we get more groups than we need so we need to skip a bunch
    //      this is super annoying and should be fixed.
    //      The reason this is necessary is because when plot dimensions
    //      are decided there is no information about y axis width.
    //      Therefore we take the same amount of groups as there are columns
    //      in the terminal.
    uint32_t goffset = pl->xsize - pl->pxsize;
    while (goffset != 0) {
        g = g->next;
        goffset--;
    }

    while (g != NULL) {
        if (g->id % XTICK_SPACING == 0) {

            char t[30] = {'\0'};
            sprintf(t, "%5.2f", g->wstart);
            char* pt = t;

            for (uint32_t x=ix ; x<ix+strlen(t) ; x++, pt++) {
                if (x >= pl->xaxis_xstart+pl->xaxis_xsize)
                    break;

                Cell* c = pl_get_cell(pl, x, pl->xaxis_ystart);
                c->chr[0] = *pt;
            }
        }
        g = g->next;
        ix++;
    }
}

Cell* pl_get_cell(Plot* pl, uint32_t x, uint32_t y)
{
    return pl->cells[x][y];
}

Plot* pl_init(uint32_t xsize, uint32_t ysize)
{
    Plot* pl = malloc(sizeof(Plot));

    pl->xsize = xsize;
    pl->ysize = ysize;

    pl->status_size = 2;
    pl->lyaxis_size = 0;
    pl->ryaxis_size = 15;
    pl->xaxis_ysize = 1;

    uint32_t x, y;

    // TODO do this in one malloc call: pl->cells = malloc(sizeof(Cell*[pl->xsize][pl->ysize]));
    pl->cells = malloc(pl->xsize * sizeof(Cell**));

    for (x=0 ; x<pl->xsize ; x++) {
        pl->cells[x] = malloc(pl->ysize * sizeof(Cell*));

        for (y=0 ; y<pl->ysize ; y++)
            pl->cells[x][y] = pl_cell_init(x, y);
    }
    return pl;
}

Cell* pl_cell_init(uint32_t x, uint32_t y)
{
    Cell* c = malloc(sizeof(Cell));
    strcpy(c->chr, EMPTY);
    c->fgcol = WHITE;
    c->x = x;
    c->y = y;
    return c;
}

void pl_destroy(Plot* pl)
{
    for (int x=0 ; x<pl->xsize ; x++) {
        for (int y=0 ; y<pl->ysize ; y++)
            free(pl->cells[x][y]);

        free(pl->cells[x]);
    }
    free(pl->cells);
    free(pl);
}

void pl_draw_ryaxis(Plot* pl, double dmin, double dmax, int32_t yoffset)
{
    /* calculate axis values and draw them in matrix.
     * Also highlight last value of last point (not group)
     */
    // calculate stepsize between tickers
    double step = (dmax - dmin) / pl->pysize;

    // calculate first column of axis
    uint32_t xstart = pl->ryaxis_start;
    int32_t y=0;

    // TODO create funtion that resets all dimensions of components
    //
    for (int32_t iy=pl->pystart ; iy<pl->ysize ; iy++, y++) {

        uint32_t ix;
        char buf[50] = {'\0'};
        double ticker = dmin + ((y - yoffset)*step);
        get_tickerstr(buf, ticker, pl->ryaxis_size, pl->ryaxis_nwhole, pl->ryaxis_nfrac);
        char* pbuf = buf;

        for (ix=xstart ; ix<xstart+strlen(buf) ; ix++, pbuf++) {
            Cell* c = pl_get_cell(pl, ix, iy);
            c->chr[0] = *pbuf;
            c->fgcol = WHITE;
        }
    }
}

void pl_set_dimensions(Plot* pl, double dmin, double dmax)
{
    /* calculate all dimensions and start indices */
    uint8_t spacer = 1;

    pl->ryaxis_nfrac  = find_nfrac((dmax - dmin)); // digits behind the comma
    //pl->ryaxis_nfrac  = find_nfrac((dmax - dmin)/pl->pysize); // digits behind the comma
    pl->ryaxis_nwhole = find_nwhole(dmax); // digits before the comma
    pl->ryaxis_size   = pl->ryaxis_nwhole + 1 + pl->ryaxis_nfrac;
    pl->ryaxis_start  = pl->xsize - pl->ryaxis_size;

    pl->pysize = pl->ysize - pl->status_size - pl->xaxis_ysize;
    pl->pxsize = pl->xsize - pl->lyaxis_size - pl->ryaxis_size - spacer; // spacer for ryaxis

    pl->xaxis_xsize = pl->pxsize;
    pl->xaxis_xstart = pl->lyaxis_size;
    pl->xaxis_ystart = pl->status_size;

    pl->pxstart = pl->lyaxis_size;
    pl->pystart = pl->status_size + pl->xaxis_ysize;
}

void pl_draw(Plot* pl, Index* index, Groups* groups, State* s)
{
    /* Draw all elements to internal matrix */
    pl_set_dimensions(pl, s->dmin, s->dmax);
    pl_draw_ryaxis(pl, s->dmin, s->dmax, s->pany);
    pl_draw_candlesticks(pl, groups->group, s->dmin, s->dmax, s->pany);
    pl_draw_last_data(pl, s->dmin, s->dmax, s->pany, (*index->ptail)->close);
    pl_draw_xaxis(pl, groups->group);
}

void get_tickerstr(char* buf, double ticker, uint32_t ntotal, uint32_t nwhole, uint32_t nfrac)
{
    // TODO null terminate characters in cell
    //
    /* create ticker with specific amount of decimals and copy to string */
    char tmp[50] = {'\0'};
    char sfrac[50] = {'\0'};

    sprintf(tmp, "%d.", abs(ticker));
    sprintf(sfrac, "%g", ticker - abs(ticker));
    strncat(tmp, sfrac+2, nfrac);

    for (int i=0; i<ntotal-strlen(tmp) ; i++)
        strcat(buf, " ");

    strncat(buf, tmp, strlen(tmp));
    //set_status(1, "ry: %d[%d.%d] %d %d >>%s<<", ntotal, nwhole, nfrac, strlen(tmp), ntotal-strlen(tmp), buf);
}

