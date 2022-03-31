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

    for (uint32_t ix=0 ; ix<getmaxx(wgraph) ; ix++)
        add_str_color(wgraph, a->ysize-ilasty-1, ix, CMAGENTA, CDEFAULT, true, YAXIS_LDATA_LINE_CHR);
}

InterpolateXY* ipoint_init(InterpolateXY* prev, InterpolateXY* next, int32_t x, int32_t y)
{
    /* Create a new point, used to interpolate sets of points.
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

void ipoint_destroy(InterpolateXY* p)
{
    while (p != NULL) {
        InterpolateXY* tmp = p->next;
        free(p);
        p = tmp;
    }
}

InterpolateXY* ipoint_xinterpolate(InterpolateXY* p, double x0, double y0, double x1, double y1)
{
    // calculate grow factor between points: y = xd + y
    double d = (y1 - y0) / (x1 - x0);

    // calculate how many xs we need to interpolate
    uint32_t xlen = x1 - x0;

    for (uint32_t x=1 ; x<xlen ; x++) {
        p = ipoint_init(p, NULL, floor(x+x0), floor((x1 - x0) * d + y0));
        p->ptype = XINT_POINT;
    }
    return p;
}

InterpolateXY* ipoint_yinterpolate(InterpolateXY* p, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    int32_t ylen   = abs(y1-y0);

    assertf(p->ptype == XINT_POINT || p->ptype == DPOINT, "point is not an X point or a Data point");

    // ascending
    if (y0 < y1) {
        int32_t ystart = y0;
        int32_t yend   = ystart + ylen;

        // NOTE uncomment to connect points diagonally
        //int32_t ystart = y0+1;
        //int32_t yend   = ystart + ylen -1;

        for (int32_t y=ystart ; y<yend ; y++) {
            p = ipoint_init(p, p->next, x1, y);
            p->ptype = YINT_POINT;
        }
    }

    // descending
    else if (y0 > y1) {
        int32_t ystart = y0;
        int32_t yend   = ystart - ylen;

        // NOTE uncomment to connect points diagonally
        //int32_t ystart = y0-1;
        //int32_t yend   = ystart - ylen +1;

        for (int32_t y=ystart ; y>yend ; y--) {
            p = ipoint_init(p, p->next, x1, y);
            p->ptype = YINT_POINT;
        }
    }
    return p;
}

POrientation ipoint_get_orientation(InterpolateXY* p1, InterpolateXY* p2)
{
    /* Return orientation of p1 to p2 */
    // if point1 is LEFT from p2
    if (p1->x < p2->x)
        return PO_W;

    // if point1 is RIGHT from p2
    else if (p1->x > p2->x)
        return PO_E;

    // if ABOVE eachother
    else {
        if (p1->y > p2->y)
            return PO_N;
        else if (p1->y < p2->y)
            return PO_S;
        else
            return PO_ERROR;
    }
}

void yaxis_draw_pipeline_chr(InterpolateXY* cp, WINDOW* wtarget, Line* l)
{
    /* Find orientation of 2 points (left and right) relative to middle point.
     * Print the right pipe unicode character to connect the line.,
     * cp is a point from a linked list of interpolated points. */

    int32_t ysize = getmaxy(wtarget);

    InterpolateXY* lp = cp->prev;
    InterpolateXY* rp = cp->next;
    POrientation lo;
    POrientation ro;

    if (!y_is_in_view(wtarget, ysize-cp->y-1))
        return;

    if (!lp)
        lo = PO_W;
    else
        lo = ipoint_get_orientation(lp, cp);

    if (!rp)
        ro = PO_E;
    else
        ro = ipoint_get_orientation(rp, cp);

    if ((lo == PO_N && ro == PO_S) || (lo == PO_S && ro == PO_N))
        add_str_color(wtarget, ysize-cp->y-1, cp->x, CBLUE, CDEFAULT, false, YAXIS_TB);
    else if (lo == PO_N && ro == PO_E)
        add_str_color(wtarget, ysize-cp->y-1, cp->x, CBLUE, CDEFAULT, false, YAXIS_TR);
    else if (lo == PO_S && ro == PO_E)
        add_str_color(wtarget, ysize-cp->y-1, cp->x, CBLUE, CDEFAULT, false, YAXIS_BR);
    else if (lo == PO_W && ro == PO_E)
        add_str_color(wtarget, ysize-cp->y-1, cp->x, CBLUE, CDEFAULT, false, YAXIS_LR);
    else if (lo == PO_W && ro == PO_N)
        add_str_color(wtarget, ysize-cp->y-1, cp->x, CBLUE, CDEFAULT, false, YAXIS_LT);
    else if (lo == PO_W && ro == PO_S)
        add_str_color(wtarget, ysize-cp->y-1, cp->x, CBLUE, CDEFAULT, false, YAXIS_LB);
    else
        add_str_color(wtarget, ysize-cp->y-1, cp->x, CBLUE, CDEFAULT, false, "*");
}

InterpolateXY* interpolate(Line* l, WINDOW* wtarget, InterpolateXY* prev, InterpolateXY* cur)
{
    /* Interpolate points in x and y direction */
    ipoint_xinterpolate(prev, prev->x, prev->y, cur->x, cur->y);

    InterpolateXY* xp = prev;

    // yinterpolate inbetween x points
    while (xp->next) {
        xp = ipoint_yinterpolate(xp, xp->x, xp->y, xp->next->x, xp->next->y);
        xp = xp->next;
    }
    return xp;
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

    InterpolateXY* head = NULL;
    InterpolateXY* prev = NULL;
    InterpolateXY* cur = NULL;

    while (g != NULL) {
        if (! g->is_empty) {

            // map data point from data range to terminal rows range
            int32_t iy  = map(g->y,  a->dmin, a->dmax, 0, ysize-1) + yoffset;

            if (!prev) {
                prev = ipoint_init(NULL, NULL, ix, iy);
                head = prev;
            }
            else {
                cur = ipoint_init(prev, NULL, ix, iy);
                prev = interpolate(l, wtarget, prev, cur);
            }
        }
        g = g->next;
        ix++;
    }

    InterpolateXY* p = head;
    while (p != NULL) {
        yaxis_draw_pipeline_chr(p, wtarget, l);
        p = p->next;
    }
    ipoint_destroy(head);
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
                add_str_color(win, ysize-y-1, ix, CGREEN, CDEFAULT, false, YAXIS_OHLC_BODY);
            else
                add_str_color(win, ysize-y-1, ix, CGREEN, CDEFAULT, false, YAXIS_OHLC_WICK);
        }
    // RED
    }
    else {
        for (int y=ilow ; y<=ihigh ; y++) {
            if (y < 0 || y > getmaxy(win)-1)
                continue;
            if (y <= iopen && y >= iclose)
                add_str_color(win, ysize-y-1, ix, CRED, CDEFAULT, false, YAXIS_OHLC_BODY);
            else
                add_str_color(win, ysize-y-1, ix, CRED, CDEFAULT, false, YAXIS_OHLC_WICK);
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
    /* create ticker with specific amount of decimals and copy to string */
    char swhole[50]   = {'\0'};
    char sfrac[50]    = {'\0'};
    char spadding[50] = {'\0'};

    sprintf(swhole, "%d.", abs(ticker));
    
    // calculate amount of zeros at start of string
    int zpad = ((int)nwhole - (int)strlen(swhole)+1 > 0) ? (int)nwhole - (int)strlen(swhole) +1 : 0;
    memset(spadding, '0', zpad);
    spadding[zpad] = '\0';

    sprintf(sfrac, "%.20f", ticker - abs(ticker));

    sprintf(buf, "%s%s%s", spadding, swhole, sfrac+2);
    buf[ntotal] = '\0';

    //debug("%d %d >%s< - %s - %s = >%s<\n", (int)nwhole - (int)strlen(swhole), zpad, spadding, swhole, sfrac+2, buf);
}

void printll(InterpolateXY* p)
{
    debug("----------------------------------------------\n");
    while (p != NULL) {
        if (p->prev && p->next)
            debug("%d,%d\t<-\t%d,%d\t->\t%d,%d\t=\t%s\n", p->prev->x, p->prev->y, p->x, p->y, p->next->x, p->next->y, YAXIS_LR);
        if (!p->prev && p->next)
            debug("NULL\t<-\t%d,%d\t->\t%d,%d\t=\t%s\n", p->x, p->y, p->next->x, p->next->y, YAXIS_LR);
        if (p->prev && !p->next)
            debug("%d,%d\t<-\t%d,%d\t->\tNULL\t=\t%s\n", p->prev->x, p->prev->y, p->x, p->y, YAXIS_LR);
        p = p->next;
    }
}
