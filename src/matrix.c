#include "matrix.h"


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
    //ilow = (ilow < 0) ? 0 : ilow;
    //ihigh = (ihigh > vp->ysize-1) ? vp->ysize-1 : ihigh;

    if (iopen < iclose) {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < vp->pystart || y > vp->ysize-1)
                continue;
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_WICK);
            cell->fgcol = GREEN;
        }
        for (int y=iopen ; y<=iclose ; y++) {
            if (y < vp->pystart || y > vp->ysize-1)
                continue;
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_BODY);
            cell->fgcol = GREEN;
        }

    // RED
    }
    else {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < vp->pystart || y > vp->ysize-1)
                continue;
            Cell* cell = vp_get_cell(vp, ix, y);
            strcpy(cell->chr, CS_WICK);
            cell->fgcol = RED;
        }
        for (int y=iclose ; y<=iopen ; y++) {
            if (y < vp->pystart || y > vp->ysize-1)
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

    // start at this xy indices
    uint32_t ix = vp->pxstart;
    uint32_t iy = vp->pystart;

    while (g != NULL) {
        if (! g->is_empty) {

            // map data point from data range to terminal rows range
            uint32_t iopen  = map(g->open,  dmin, dmax, iy, vp->ysize-1) + yoffset;
            uint32_t ihigh  = map(g->high,  dmin, dmax, iy, vp->ysize-1) + yoffset;
            uint32_t ilow   = map(g->low,   dmin, dmax, iy, vp->ysize-1) + yoffset;
            uint32_t iclose = map(g->close, dmin, dmax, iy, vp->ysize-1) + yoffset;

            vp_draw_candlestick(vp, ix, iopen, ihigh, ilow, iclose);
        }
        g = g->next;
        ix++;
    }
}

void vp_draw_ryaxis_old(ViewPort* vp, State* s)
{
    /* calculate axis values and draw them in matrix.
     * Also highlight last value of last point (not group)
     */
    // calculate stepsize between tickers
    double step = (s->dmax - s->dmin) / vp->pxsize;

    // calculate first column of axis
    uint32_t xstart = vp->xsize - vp->ryaxis_size -1;

    for (int32_t iy=vp->pystart ; iy<vp->ysize ; iy++) {

        char t[30] = {'\0'};
        sprintf(t, "%.7f", s->dmin + ((iy - s->pany)*step));
        char* ptr = t;

        for (uint32_t ix=xstart ; ix<xstart+strlen(t) ; ix++, ptr++) {
            if (ix >= vp->xsize)
                break;
            Cell* c = vp_get_cell(vp, ix, iy);
            c->chr[0] = *ptr;
            c->fgcol = WHITE;
        }
    }
}

void get_tickerstr(char* buf, double ticker, uint32_t nfrac)
{
    /* create ticker with specific amount of decimals and copy to string */
    sprintf(buf, "%d.", abs(ticker));
    char sfrac[50] = {'\0'};
    sprintf(sfrac, "%g", ticker - abs(ticker));
    strncat(buf, sfrac+2, nfrac);
}

void vp_draw_ryaxis(ViewPort* vp, State* s)
{
    /* calculate axis values and draw them in matrix.
     * Also highlight last value of last point (not group)
     */
    // calculate stepsize between tickers
    double step = (s->dmax - s->dmin) / vp->pxsize;

    // digits behind the comma
    uint32_t nfrac = find_nfrac((s->dmax - s->dmin)/vp->pxsize);
    uint32_t nwhole = find_nwhole(s->dmax);
    vp->ryaxis_size = nfrac+nwhole+1;

    vp_set_dimensions(vp);

    // calculate first column of axis
    uint32_t xstart = vp->xsize - vp->ryaxis_size -1;

    // TODO create funtion that resets all dimensions of components
    //
    for (int32_t iy=vp->pystart ; iy<vp->ysize ; iy++) {

        uint32_t ix;
        char buf[50] = {'\0'};
        double ticker = s->dmin + ((iy - s->pany)*step);
        get_tickerstr(buf, ticker, nfrac);
        char* pbuf = buf;

        for (ix=xstart ; ix<xstart+strlen(buf) ; ix++, pbuf++) {
            Cell* c = vp_get_cell(vp, ix, iy);
            //c->chr[0] = '*';
            c->chr[0] = *pbuf;
            c->fgcol = WHITE;
        }

    }
}

void vp_draw_last_data(ViewPort* vp, State* s, double lasty)
{
    /* highlight last data in axis */

    // calculate first column of axis
    uint32_t xstart = vp->xsize - vp->ryaxis_size -1;

    // calculate y index of last data
    int32_t ilasty = map(lasty, s->dmin, s->dmax, vp->pystart, vp->ysize-1) + s->pany;

    char t[20] = {'\0'};
    sprintf(t, "%1.7f", lasty);
    char* ptr = t;

    // if last data is out of range, stick to top/bottom
    if (ilasty < vp->pystart)
        ilasty = vp->pystart;
    if (ilasty >= vp->ysize)
        ilasty = vp->ysize-1;

    for (int32_t ix=xstart ; ix<xstart+strlen(t) ; ix++, ptr++) {
        if (ix >= vp->xsize)
            break;
        Cell* c = vp_get_cell(vp, ix, ilasty);
        c->chr[0] = *ptr;
        c->fgcol = GREEN;
    }
    // draw line
    for (int32_t ix=vp->pxstart ; ix<vp->pxstart+vp->pxsize ; ix++, ptr++) {
        Cell* c = vp_get_cell(vp, ix, ilasty);
        if (c->chr[0] == ' ') {
            strcpy(c->chr, LINE_CHR);
            c->fgcol = MAGENTA;
        }
    }
}

void vp_draw_xaxis(ViewPort* vp, State* s, Groups* groups)
{
    Group* g = groups->group;
    int32_t ix = vp->xaxis_xstart;

    while (g != NULL) {
        if (g->id % XTICK_SPACING == 0) {

            char t[30] = {'\0'};
            sprintf(t, "%5.2f", g->wstart);
            char* tptr = t;

            for (uint32_t x=ix ; x<ix+strlen(t) ; x++, tptr++) {
                if (x >= vp->xaxis_xstart+vp->xaxis_xsize)
                    break;

                Cell* c = vp_get_cell(vp, x, vp->xaxis_ystart);
                c->chr[0] = *tptr;
            }
        }
        g = g->next;
        ix++;
    }
}

Cell* vp_get_cell(ViewPort* vp, uint32_t x, uint32_t y)
{
    return vp->cells[x][y];
}

void vp_set_dimensions(ViewPort* vp)
{
    /* calculate all dimensions and start indices */
    vp->pysize = vp->ysize - vp->status_size - vp->xaxis_ysize;
    vp->pxsize = vp->xsize - vp->lyaxis_size - vp->ryaxis_size - 2;

    vp->xaxis_xsize = vp->pxsize;
    vp->xaxis_xstart = vp->lyaxis_size;
    vp->xaxis_ystart = vp->status_size;

    vp->pxstart = vp->lyaxis_size;
    vp->pystart = vp->status_size + vp->xaxis_ysize;
}

ViewPort* vp_init(uint32_t xsize, uint32_t ysize)
{
    ViewPort* vp = malloc(sizeof(ViewPort));

    vp->xsize = xsize;
    vp->ysize = ysize;

    vp->status_size = 2;
    vp->lyaxis_size = 0;
    vp->ryaxis_size = 15;
    vp->xaxis_ysize = 1;

    vp_set_dimensions(vp);

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
