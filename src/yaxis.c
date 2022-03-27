#include "yaxis.h"

Yaxis* yaxis_init(WINDOW* parent, AxisSide side)
{
    Yaxis* a = malloc(sizeof(Yaxis));

    a->parent = parent;

    // window is created if data is present
    a->win = NULL;

    a->side = side;

    a->xsize = 0;

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

    a->bgcol = CDEFAULT;

    return a;
}

void  yaxis_destroy(Yaxis* a)
{
    Line* l = a->line;
    Line* prev;

    while (l != NULL) {
        prev = l;
        l = l->next;
        line_destroy(prev);
    }

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

GroupContainer* yaxis_get_gc(Yaxis* a)
{
    /* Find first available data for a line.
     * Used mainly to find data so we can generate the Xaxis */
    if (a->is_empty)
        return NULL;

    Line* l = a->line;
    while (l != NULL) {
        if (l->gc != NULL)
            return l->gc;

        l = l->next;
    }
    return NULL;
}

PlotStatus yaxis_set_window_width(Yaxis* a, uint32_t yoffset)
{
    /* Find window width of y tickers. Return 1 if size has changed since last check */
    if (a->is_empty)
        return PLERR_NO_DATA;

    if (a->autorange) {
        a->dmin = a->vdmin;
        a->dmax = a->vdmax;
    }

    // digits before and after the dot
    a->nwhole = find_nwhole(a->dmax); 
    a->nfrac  = find_nfrac2(a->dmin, a->dmax);
    //a->nfrac  = find_nfrac(a->dmax - a->dmin);
    uint32_t new_xsize = a->nwhole + 1 + a->nfrac;

    // resize axis window to new ticker width if changed
    if (a->xsize != new_xsize) {
        a->xsize = new_xsize;
        assert(a->parent != NULL);

        delwin(a->win);
        if (a->side == AXIS_LEFT) {
            a->win = derwin(a->parent, a->ysize, a->xsize, yoffset, 0);
            assert(a->win && "Failed to create left axis window");
        }
        else {
            a->win = derwin(a->parent, a->ysize, a->xsize, yoffset, getmaxx(a->parent)-new_xsize);
            assert(a->win && "Failed to create right axis window");
        }
        return PLSTATUS_YAXIS_CHANGED;
    }
    return PLSTATUS_YAXIS_UNCHANGED;
}

void yaxis_draw(Yaxis* a, WINDOW* wtarget, int32_t pany)
{
    /* Draw all lines in this axis into Plot */
    if (a->is_empty)
        return;

    yaxis_draw_tickers(a, pany);

    Line* l = a->line;
    while (l != NULL && l->is_enabled) {

        GroupContainer* gc = l->gc;

        if (gc == NULL) {
            l = l->next;
            continue;
        }

        // Highlight last data in tickers
        if (gc->plast != NULL) {
            if (gc->lineid->ltype == LTYPE_OHLC)
                yaxis_draw_last_data(a, wtarget, pany, gc->plast->close);
            else if (gc->lineid->ltype == LTYPE_LINE)
                yaxis_draw_last_data(a, wtarget, pany, gc->plast->y);
        }

        if (gc->lineid->ltype == LTYPE_OHLC)
            yaxis_draw_candlesticks(a, wtarget, gc->group, pany);
        else if (gc->lineid->ltype == LTYPE_LINE)
            yaxis_draw_line(a, l, wtarget, gc->group, pany);

        l = l->next;
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
    add_str(a->win, a->ysize-ilasty-1, 0, CGREEN, a->bgcol, buf);

    for (uint32_t ix=0 ; ix<getmaxx(wgraph) ; ix++) {
        add_str_color(wgraph, a->ysize-ilasty-1, ix, CMAGENTA, CDEFAULT, YAXIS_LDATA_LINE_CHR);
    }
}


InterpolateXY* int_point_init(InterpolateXY* prev, InterpolateXY* next, int32_t x, int32_t y)
{
    /* Create a new point.
     * If prev!=NULL append point after prev in linked list
     * If next!=NULL insert point before next in linked list
     */
    InterpolateXY* p = malloc(sizeof(InterpolateXY));

    if (prev != NULL) {
        prev->next = p;
    }
    if (next != NULL) {
        next->prev = p;
    }
    p->prev = prev;
    p->next = next;
    p->x = x;
    p->y = y;
    p->ptype = DPOINT;
    return p;
}

InterpolateXY* xinterpolate(InterpolateXY* p, double x0, double y0, double x1, double y1)
{
    // calculate grow factor between points: y = xd + y
    double d = (y1 - y0) / (x1 - x0);

    // calculate how many xs we need to interpolate
    uint32_t xlen = x1 - x0;

    for (uint32_t x=1 ; x<xlen ; x++) {
        p = int_point_init(p, NULL, floor(x+x0), floor((p->x - x0) * d + y0));
        p->ptype = XINT_POINT;
    }
    return p;
}

InterpolateXY* yinterpolate(InterpolateXY* p, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    int32_t ylen   = abs(y1-y0);

    assertf(p->ptype == XINT_POINT || p->ptype == DPOINT, "point is not an X point or a Data point");

    // ascending
    if (y0 < y1) {
        int32_t ystart = y0+1;
        int32_t yend   = ystart + ylen -1;

        for (int32_t y=ystart ; y<yend ; y++) {
            p = int_point_init(p, p->next, x1, y);
            p->ptype = YINT_POINT;
        }
    }

    // descending
    else if (y0 > y1) {
        int32_t ystart = y0-1;
        int32_t yend   = ystart - ylen +1;

        for (int32_t y=ystart ; y>yend ; y--) {
            p = int_point_init(p, p->next, x1, y);
            p->ptype = YINT_POINT;
        }
    }

    return p;
}

void print_point(InterpolateXY* p, WINDOW* wtarget, Line* l)
{
    int32_t ysize = getmaxy(wtarget);

    if (!y_is_in_view(wtarget, ysize-p->y-1))
        return;

    switch (p->ptype) {
        case DPOINT:
            add_str_color(wtarget, ysize-p->y-1, p->x, CBLUE, CDEFAULT, l->chr);
            break;
        case XINT_POINT:
            add_str_color(wtarget, ysize-p->y-1, p->x, CMAGENTA, CDEFAULT, l->chr);
            break;
        case YINT_POINT:
            add_str_color(wtarget, ysize-p->y-1, p->x, CGREEN, CDEFAULT, l->chr);
            break;
    }
}

InterpolateXY* interpolate(Line* l, WINDOW* wtarget, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    /* Interpolate points in x and y direction */

    // create head of linked list from first point
    InterpolateXY* head = int_point_init(NULL, NULL, x0, y0);

    // add X-interpolated points to ll and append last point to end
    int_point_init(xinterpolate(head, x0, y0, x1, y1), NULL, x1, y1);

    InterpolateXY* xp = head;

    // yinterpolate inbetween x points
    while (xp->next) {
        xp = yinterpolate(xp, xp->x, xp->y, xp->next->x, xp->next->y);
        xp = xp->next;
    }

    return head;
}

void yaxis_draw_line(Yaxis* a, Line* l, WINDOW* wtarget, Group* g, int32_t yoffset)
{
    /* itter groups and draw them in plot */
    // start at this xy indices
    uint32_t ix = 0;

    uint32_t ysize = getmaxy(wtarget);
    
    //TODO COLS is not necessarily the width of the parent window!!!!
    // we have to get more groups from index than we actually need so we need to skip the groups that don't fit in plot
    uint32_t goffset = COLS - getmaxx(wtarget);
    //uint32_t goffset = getmaxx(a->win) - getmaxx(wtarget);
    while (goffset != 0) {
        g = g->next;
        goffset--;
    }

    int32_t previx = -1;
    int32_t previy = -1;

    while (g != NULL) {
        if (! g->is_empty) {

            // map data point from data range to terminal rows range
            int32_t iy  = map(g->y,  a->dmin, a->dmax, 0, ysize-1) + yoffset;

            if (previx > 0) {
                InterpolateXY* p = interpolate(l, wtarget, previx, previy, ix, iy);
                while (p != NULL) {
                    print_point(p, wtarget, l);
                    p = p->next;
                }
            }

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
    for (int32_t iy=0 ; iy<a->ysize ; iy++) {
        char buf[50] = {'\0'};
        double ticker = a->dmin + ((iy - yoffset)*step);
        get_tickerstr(buf, ticker, a->xsize, a->nwhole, a->nfrac);
        add_str(a->win, a->ysize-iy-1, 0, CWHITE, a->bgcol, buf);
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
            if (y >= iopen && y <= iclose)
                add_str_color(win, ysize-y-1, ix, CGREEN, CDEFAULT, YAXIS_OHLC_BODY);
            else
                add_str_color(win, ysize-y-1, ix, CGREEN, CDEFAULT, YAXIS_OHLC_WICK);
        }
    // RED
    }
    else {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < 0 || y > getmaxy(win)-1)
                continue;
            if (y <= iopen && y >= iclose)
                add_str_color(win, ysize-y-1, ix, CRED, CDEFAULT, YAXIS_OHLC_BODY);
            else
                add_str_color(win, ysize-y-1, ix, CRED, CDEFAULT, YAXIS_OHLC_WICK);
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
    // TODO pad zeros on left
    //
    /* create ticker with specific amount of decimals and copy to string */
    char tmp[50] = {'\0'};
    char sfrac[50] = {'\0'};


    sprintf(tmp, "%d.", abs(ticker));

    for (int i=0 ; i<nwhole - (strlen(tmp)-1) ; i++) {
        buf[i] = '0';
        buf[i+1] = '\0';
        //strcat(buf, "0");
    }

    sprintf(sfrac, "%.20f", ticker - abs(ticker));
    strncat(tmp, sfrac+2, nfrac);


    // NOTE this segfaults
    //for (int i=0; i<ntotal-strlen(tmp) ; i++)
    //    strcat(buf, "0");

    strncat(buf, tmp, strlen(tmp));
}
