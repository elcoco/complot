#include "yaxis.h"

Axis* axis_init(AxisSide side)
{
    Axis* a = malloc(sizeof(Axis));
    a->side = side;
    a->txsize = 0;
    a->tysize = 0;
    a->txstart = 0;

    a->nwhole = 0;
    a->nfrac = 0;

    a->tdmin = 0;
    a->tdmax = 0;

    a->vdmin = 0;
    a->vdmax = 0;

    a->dmin = 0;
    a->dmax = 0;

    // line linked list
    a->line = NULL;
    a->ltail = malloc(sizeof(Line*));
    *(a->ltail) = NULL;

    a->is_empty = true;
    a->autorange = true;

    return a;
}

void  axis_destroy(Axis* a)
{
    free(a->ltail);
    free(a);
}

void axis_add_line(Axis* a, Line* l)
{
    // connect line in linked list
    if (a->line == NULL) {
        a->line = l;
        *(a->ltail) = l;
    } else {
        Line* prev = *(a->ltail);
        *(a->ltail) = l;
        prev->next = l;
    }
    // add axis stuct to line struct
    l->axis = a;
}

void axis_draw(Axis* a, Plot* pl, State* s)
{
    /* Draw all lines in this axis into Plot */
    if (a->autorange) {
        a->dmin = a->vdmin;
        a->dmax = a->vdmax;
    } else {
        a->dmin = a->tdmin;
        a->dmax = a->tdmax;
    }

    if (a->side == AXIS_LEFT)
        a->txstart = 0;
    else
        a->txstart = pl->xsize - a->txsize;

    // draw y axis tickers
    if (a->side == AXIS_RIGHT) {
        axis_draw_tickers(a, pl, s->pany);

        Line* l = a->line;
        while (l != NULL) {
            plot_draw_candlesticks(pl, l->groups->group, a, s->pany); 

            // Highlight last data in tickers
            if (l->groups->plast != NULL)
                axis_draw_last_data(a, pl, s->pany, l->groups->plast->close);

            l = l->next;
        }
    }
}

void axis_draw_tickers(Axis* a, Plot* pl, int32_t yoffset)
{
    /* calculate axis values and draw them in matrix.
     * Also highlight last value of last point (not group)
     */
    // calculate stepsize between tickers
    double step = (a->dmax - a->dmin) / pl->pysize;

    // calculate first column of axis
    int32_t y=0;

    for (int32_t iy=pl->pystart ; iy<pl->ysize ; iy++, y++) {

        uint32_t ix;
        char buf[50] = {'\0'};
        double ticker = a->dmin + ((y - yoffset)*step);
        get_tickerstr(buf, ticker, a->txsize, a->nwhole, a->nfrac);
        char* pbuf = buf;

        for (ix=a->txstart ; ix<a->txstart+strlen(buf) ; ix++, pbuf++) {
            Cell* c = plot_get_cell(pl, ix, iy);
            c->chr[0] = *pbuf;
            c->fgcol = WHITE;
        }
    }
}

void axis_draw_last_data(Axis* a, Plot* pl, double pany, double lasty)
{
    /* highlight last data in axis */

    // calculate first column of axis
    //uint32_t xstart = pl->xsize - pl->ryaxis_size -1;

    // calculate y index of last data
    int32_t ilasty = map(lasty, a->dmin, a->dmax, pl->pystart, pl->ysize-1) + pany;

    char buf[50] = {'\0'};
    get_tickerstr(buf, lasty, a->txsize, a->nwhole, a->nfrac);
    char* pbuf = buf;

    // if last data is out of range, stick to top/bottom
    if (ilasty < pl->pystart)
        ilasty = pl->pystart;
    if (ilasty >= pl->pystart + pl->pysize -1)
        ilasty = pl->pystart + pl->pysize -1;

    for (int32_t ix=a->txstart ; ix<a->txstart+strlen(buf) ; ix++, pbuf++) {
        if (ix >= pl->xsize)
            break;
        Cell* c = plot_get_cell(pl, ix, ilasty);
        c->chr[0] = *pbuf;
        c->fgcol = GREEN;
    }
    // draw line
    for (int32_t ix=pl->pxstart ; ix<pl->pxstart+pl->pxsize ; ix++, pbuf++) {
        Cell* c = plot_get_cell(pl, ix, ilasty);
        if (c->chr[0] == ' ') {
            strcpy(c->chr, LINE_CHR);
            c->fgcol = MAGENTA;
        }
    }
}


void get_tickerstr(char* buf, double ticker, uint32_t ntotal, uint32_t nwhole, uint32_t nfrac)
{
    // TODO null terminate characters in cell
    //
    /* create ticker with specific amount of decimals and copy to string */
    char tmp[50] = {'\0'};
    char sfrac[50] = {'\0'};

    sprintf(tmp, "%d.", abs(ticker));
    sprintf(sfrac, "%.20f", ticker - abs(ticker));
    strncat(tmp, sfrac+2, nfrac);

    for (int i=0; i<ntotal-strlen(tmp) ; i++)
        strcat(buf, " ");

    strncat(buf, tmp, strlen(tmp));
    //set_status(1, "ry: %d[%d.%d] %d %d >>%s<<", ntotal, nwhole, nfrac, strlen(tmp), ntotal-strlen(tmp), buf);
}
