#include "yaxis.h"

// TODO linked list in plot_axis_init and plot_init is not destroyed
Axis* axis_init(AxisSide side)
{
    Axis* a = malloc(sizeof(Axis));
    a->side = side;
    a->txsize = 0;
    a->tysize = 0;
    a->txstart = 0;

    // candlesticks start here
    a->pxsize = 0;
    a->pysize = 0;
    a->pxstart = 0;

    a->nwhole = 0;
    a->nfrac = 0;

    a->dmin = 0;
    a->dmax = 0;
    a->dlast = 0;

    // line linked list
    a->line = NULL;
    a->ltail = malloc(sizeof(Line*));
    *(a->ltail) = NULL;

    a->is_empty = true;

    return a;
}

void  axis_destroy(Axis* a)
{
    //Line* l = a->line;
    //while (l != NULL) {
    //    line_destroy(l);
    //    free(tmp);
    //}
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
}

void axis_set_data(Axis* a, Line* l)
{
    // set line data in line
    l->group = groups->group;

    // set/update axis dimensions
    if (a->is_empty) {
        a->is_empty = false;
        a->dmin = groups->gmin;
        a->dmax = groups->gmax;
    } else {
        if (groups->dmin < a->dmin)
            a->dmin = groups->dmin;
        if (groups->dmax > a->dmax)
            a->dmax = groups->dmax;
    }
    // digits before and after the dot
    a->nwhole = find_nwhole(a->dmax); 
    a->nfrac  = find_nfrac(a->dmax - a->dmin);
    a->txsize  = a->nwhole + 1 + a->nfrac;
}

void axis_draw(Axis* a, Plot* pl, State* s)
{
    /* Draw all lines in this axis into Plot */
    // TODO yoffset is not passed, so y panning is not possible
    // TODO dmin and dmax are taken from groups struct but to allow non-autoresize it should
    //      be taken from State struct
    // TODO implement left side drawing

    if (a->side == AXIS_LEFT)
        a->txstart = 0;
    else
        a->txstart = pl->xsize - a->txsize;

    Line* l = a->line;
    while (l != NULL) {
        plot_draw_candlesticks(pl, l->group, a, s->pany); 
        l = l->next;
    }

    // draw y axis tickers
    if (a->side == AXIS_RIGHT)
        axis_draw_tickers(a, pl, s->pany);

    //plot_draw_last_data(pl, a, s->pany, (*index->ptail)->close);
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
