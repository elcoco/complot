#include "yaxis.h"

Yaxis* yaxis_init(WINDOW* parent, AxisSide side)
{
    Yaxis* a = malloc(sizeof(Yaxis));

    a->parent = parent;

    //a->xsize = 0;
    //a->ysize = ysize;

    //int xstart = (side == AXIS_LEFT) ? 0 : getmaxx(parent) - a->xsize;

    // window is created if data is present
    a->win = NULL;
    //a->win = subwin(a->parent, a->ysize, a->xsize, 0, xstart);

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

void  yaxis_destroy(Yaxis* a)
{
    free(a->ltail);
    free(a);
}

void yaxis_add_line(Yaxis* a, Line* l)
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

int8_t yaxis_set_window_width(Yaxis* a)
{
    /* Find window width of y tickers. Return 1 if size has changed since last check */
    if (a->is_empty)
        return -1;

    if (a->autorange) {
        a->dmin = a->vdmin;
        a->dmax = a->vdmax;
    }
    //} else {
    //    a->dmin = a->tdmin;
    //    a->dmax = a->tdmax;
    //}

    // digits before and after the dot
    a->nwhole = find_nwhole(a->dmax); 
    a->nfrac  = find_nfrac(a->dmax - a->dmin);
    uint32_t new_xsize = a->nwhole + 1 + a->nfrac;

    // resize axis window to new ticker width if changed
    if (a->xsize != new_xsize) {
        a->xsize = new_xsize;

        delwin(a->win);
        if (a->side == AXIS_LEFT)
            a->win = subwin(a->parent, a->ysize, a->xsize, 1, 0);
        else
            a->win = subwin(a->parent, a->ysize, a->xsize, 1, getmaxx(a->parent)-new_xsize);
        return 1;
    }
    return 0;
}

void yaxis_draw(Yaxis* a, WINDOW* wtarget, Groups* groups, State* s)
{
    // TODO wtarget should be in axis struct
    /* Draw all lines in this axis into Plot */

    // draw y axis tickers
    if (a->side == AXIS_RIGHT) {
        yaxis_draw_tickers(a, s->pany);

        Line* l = a->line;
        while (l != NULL) {

            if (l->groups == NULL) {
                l = l->next;
                continue;
            }

            // Highlight last data in tickers
            if (l->groups->plast != NULL) {
                if (groups->lineid->ltype == LTYPE_OHLC)
                    yaxis_draw_last_data(a, wtarget, s->pany, l->groups->plast->close);
                else if (groups->lineid->ltype == LTYPE_LINE)
                    yaxis_draw_last_data(a, wtarget, s->pany, l->groups->plast->y);
            }

            if (groups->lineid->ltype == LTYPE_OHLC)
                yaxis_draw_candlesticks(a, wtarget, l->groups->group, s->pany);
            else if (groups->lineid->ltype == LTYPE_LINE)
                yaxis_draw_line(a, wtarget, l->groups->group, s->pany);

            l = l->next;
        }
    }
}

void yaxis_draw_last_data(Yaxis* a, WINDOW* wgraph, double pany, double lasty)
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
    add_str(a->win, a->ysize-ilasty-1, 0, CGREEN, buf);

    for (uint32_t ix=0 ; ix<getmaxx(wgraph) ; ix++) {
        add_str(wgraph, a->ysize-ilasty-1, ix, CMAGENTA, LINE_CHR);
    }
}

void xinterpolate(InterpolateXY* points, double x0, double y0, double x1, double y1)
{
    // calculate grow factor between points: y = xd + y
    double d = (y1 - y0) / (x1 - x0);

    // calculate how many xs we need to interpolate
    uint32_t xlen = x1 - x0;

    for (uint32_t x=1, i=0 ; x<xlen ; x++, i++, points++) {
        points->x = floor(x+x0);
        points->y = floor((points->x - x0) * d + y0);
    }
}

void yinterpolate(WINDOW* wtarget, double x0, double y0, double x1, double y1)
{
    int32_t ylen = y1-y0;
    uint32_t ysize = getmaxy(wtarget);

    // ascending and not above eachother
    if (ylen > 1) {
        for (uint32_t y=y0+1 ; y<y1 ; y++)
            add_str(wtarget, ysize-y-1, x1, CMAGENTA, CS_BLOCK);
    }

    // decending and not above eachother
    else if (ylen < -1) {
        for (uint32_t y=y1+1 ; y<y0 ; y++)
            add_str(wtarget, ysize-y-1, x1, CRED, CS_BLOCK);
    }
}

void interpolate(WINDOW* wtarget, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
{
    uint32_t ysize = getmaxy(wtarget);
    uint32_t npoints = x1-x0-1;
    InterpolateXY* prevp = &(InterpolateXY) {.x=x0, .y=y0};

    // xinterpolate
    if (npoints > 0) {
        InterpolateXY points[npoints];
        InterpolateXY* p = points;

        xinterpolate(points, x0, y0, x1, y1);
        for (uint32_t i=0 ; i<npoints ; i++, p++) {

            // draw intermediate points
            add_str(wtarget, ysize-p->y-1, p->x, CBLUE, CS_BLOCK);

            // draw y interpolated points that are next to eachother
            yinterpolate(wtarget, prevp->x, prevp->y, p->x, p->y);

            prevp = p;

        }
    }
    yinterpolate(wtarget, prevp->x, prevp->y, x1, y1);
}

void yaxis_draw_line(Yaxis* a, WINDOW* wtarget, Group* g, int32_t yoffset)
{
    /* itter groups and draw them in plot */
    // start at this xy indices
    uint32_t ix = 0;

    uint32_t ysize = getmaxy(wtarget);
    
    //TODO COLS is not necessarily the width of the parent window!!!!
    // we have to get more groups from index than we actually need so we need to skip the groups that don't fit in plot
    uint32_t goffset = COLS - getmaxx(wtarget);
    while (goffset != 0) {
        g = g->next;
        goffset--;
    }

    int32_t previx = -1;
    int32_t previy = -1;

    while (g != NULL) {
        if (! g->is_empty) {

            // map data point from data range to terminal rows range
            uint32_t iy  = map(g->y,  a->dmin, a->dmax, 0, ysize-1) + yoffset;
            add_str(wtarget, ysize-iy-1, ix, CGREEN, CS_BLOCK);

            if (previx > 0) {
                interpolate(wtarget, previx, previy, ix, iy);
            }

            // y interpolate
            previx = ix;
            previy = iy;

        }
        g = g->next;
        ix++;
    }
}

void yaxis_draw_tickers(Yaxis* a, int32_t yoffset)
{
    // calculate stepsize between tickers
    double step = (a->dmax - a->dmin) / a->ysize;

    // calculate first column of axis
    for (int32_t iy ; iy<a->ysize ; iy++) {
        char buf[50] = {'\0'};
        double ticker = a->dmin + ((iy - yoffset)*step);
        get_tickerstr(buf, ticker, a->xsize, a->nwhole, a->nfrac);
        add_str(a->win, a->ysize-iy-1, 0, CWHITE, buf);
    }
}

void yaxis_draw_candlestick(WINDOW* win, uint32_t ix, int32_t iopen, int32_t ihigh, int32_t ilow, int32_t iclose)
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

void yaxis_draw_candlesticks(Yaxis* a, WINDOW* wtarget, Group* g, int32_t yoffset)
{
    /* itter groups and draw them in plot */
    // start at this xy indices
    uint32_t ix = 0;

    uint32_t ysize = getmaxy(wtarget);
    
    //TODO COLS is not necessarily the width of the parent window!!!!
    // we have to get more groups from index than we actually need so we need to skip the groups that don't fit in plot
    uint32_t goffset = COLS - getmaxx(wtarget);
    while (goffset != 0) {
        g = g->next;
        goffset--;
    }

    while (g != NULL) {
        if (! g->is_empty) {

            // map data point from data range to terminal rows range
            uint32_t iopen  = map(g->open,  a->dmin, a->dmax, 0, ysize-1) + yoffset;
            uint32_t ihigh  = map(g->high,  a->dmin, a->dmax, 0, ysize-1) + yoffset;
            uint32_t ilow   = map(g->low,   a->dmin, a->dmax, 0, ysize-1) + yoffset;
            uint32_t iclose = map(g->close, a->dmin, a->dmax, 0, ysize-1) + yoffset;

            yaxis_draw_candlestick(wtarget, ix, iopen, ihigh, ilow, iclose);
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
