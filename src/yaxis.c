#include "yaxis.h"

Axis* axis_init(WINDOW* parent, AxisSide side)
{
    Axis* a = malloc(sizeof(Axis));

    a->parent = parent;

    // TODO should be calculated
    a->xsize = 0;
    a->ysize = getmaxy(parent);;

    int xstart = (side == AXIS_LEFT) ? 0 : getmaxx(parent) - a->xsize;

    a->win = subwin(a->parent, a->ysize, a->xsize, 0, xstart);

    a->side = side;

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

void axis_set_window_width(Axis* a)
{
    if (a->autorange) {
        a->dmin = a->vdmin;
        a->dmax = a->vdmax;
    } else {
        a->dmin = a->tdmin;
        a->dmax = a->tdmax;
    }

    // digits before and after the dot
    a->nwhole = find_nwhole(a->dmax); 
    a->nfrac  = find_nfrac(a->dmax - a->dmin);
    uint32_t new_xsize = a->nwhole + 1 + a->nfrac;

    // TODO this is only valid for right y axis
    // resize axis window to new ticker width
    if (a->xsize != new_xsize) {
        a->xsize = new_xsize;
        a->ysize = getmaxy(a->parent);

        delwin(a->win);
        if (a->side == AXIS_LEFT)
            a->win = subwin(a->parent, a->ysize, a->xsize, 0, 0);
        else
            a->win = subwin(a->parent, a->ysize, a->xsize, 0, getmaxx(a->parent)-new_xsize);
    }
}

void axis_draw(Axis* a, WINDOW* wtarget, Groups* groups, State* s)
{
    // TODO wtarget should be in axis struct
    /* Draw all lines in this axis into Plot */
    //if (a->autorange) {
    //    a->dmin = a->vdmin;
    //    a->dmax = a->vdmax;
    //}

    // draw y axis tickers
    if (a->side == AXIS_RIGHT) {
        axis_draw_tickers(a, s->pany);

        Line* l = a->line;
        while (l != NULL) {
            axis_draw_candlesticks(a, wtarget, l->groups->group, s->pany);

            // Highlight last data in tickers
            if (l->groups->plast != NULL)
                axis_draw_last_data(a, s->pany, l->groups->plast->close);

            l = l->next;
        }
    }
}

void axis_clear_drange(Axis* a)
{
    a->is_empty = true;
}

void axis_draw_last_data(Axis* a, double pany, double lasty)
{
    /* highlight last data in axis */
    // calculate y index of last data
    int32_t ilasty = map(lasty, a->dmin, a->dmax, 0, a->ysize-1) + pany;

    // if last data is out of range, stick to top/bottom
    if (ilasty < 0)
        ilasty = 0;
    if (ilasty >= a->ysize -1)
        ilasty = a->ysize -1;

    char buf[50] = {'\0'};
    get_tickerstr(buf, lasty, a->xsize, a->nwhole, a->nfrac);
    add_str(a->win, a->ysize-ilasty, 0, CGREEN, buf);
}

void axis_draw_tickers(Axis* a, int32_t yoffset)
{
    // calculate stepsize between tickers
    double step = (a->dmax - a->dmin) / a->ysize;

    // calculate first column of axis
    int32_t y=0;
    for (int32_t iy=0 ; iy<a->ysize ; iy++, y++) {
        char buf[50] = {'\0'};
        double ticker = a->dmin + ((y - yoffset)*step);
        get_tickerstr(buf, ticker, a->xsize, a->nwhole, a->nfrac);
        add_str(a->win, a->ysize-iy, 0, CWHITE, buf);
    }
}

void axis_draw_candlestick(WINDOW* win, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose)
{
    uint32_t ysize = getmaxy(win);

    // GREEN
    if (iopen < iclose) {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < 0 || y > getmaxy(win)-1)
                continue;
            add_str(win, ysize-y-1, ix, CGREEN, CS_WICK);
        }
        for (int y=iopen ; y<=iclose ; y++) {
            if (y < 0 || y > getmaxy(win)-1)
                continue;
            add_str(win, ysize-y-1, ix, CGREEN, CS_BODY);
        }
    // RED
    }
    else {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < 0 || y > getmaxy(win)-1)
                continue;
            add_str(win, ysize-y-1, ix, CRED, CS_WICK);
        }
        for (int y=iclose ; y<=iopen ; y++) {
            if (y < 0 || y > getmaxy(win)-1)
                continue;
            add_str(win, ysize-y-1, ix, CRED, CS_BODY);

        }
    }
}

void axis_draw_candlesticks(Axis* a, WINDOW* wtarget, Group* g, int32_t yoffset)
{
    /* itter groups and draw them in plot */
    // start at this xy indices
    uint32_t ix = 0;
    uint32_t iy = 0;

    uint32_t ysize = getmaxy(wtarget);
    
    // TODO fix this
    /*
    // we have to get more groups from index than we actually need so we need to skip the groups that don't fit in plot
    uint32_t goffset = COLS - xsize;
    while (goffset != 0) {
        g = g->next;
        goffset--;
    }
    */

    while (g != NULL) {
        if (! g->is_empty) {

            // map data point from data range to terminal rows range
            uint32_t iopen  = map(g->open,  a->dmin, a->dmax, iy, ysize-1) + yoffset;
            uint32_t ihigh  = map(g->high,  a->dmin, a->dmax, iy, ysize-1) + yoffset;
            uint32_t ilow   = map(g->low,   a->dmin, a->dmax, iy, ysize-1) + yoffset;
            uint32_t iclose = map(g->close, a->dmin, a->dmax, iy, ysize-1) + yoffset;

            axis_draw_candlestick(wtarget, ix, iopen, ihigh, ilow, iclose);
        }
        g = g->next;
        ix++;
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
        strcat(buf, "0");

    strncat(buf, tmp, strlen(tmp));
    //set_status(1, "ry: %d[%d.%d] %d %d >>%s<<", ntotal, nwhole, nfrac, strlen(tmp), ntotal-strlen(tmp), buf);
}
