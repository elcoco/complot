#include "xaxis.h"

Xaxis* xaxis_init()
{
    Xaxis* xa = malloc(sizeof(Xaxis));
    xa->ysize = 2;
    xa->win = NULL;
    xa->fgcol = CWHITE;
    xa->bgcol = CDEFAULT;
    return xa;
}

void xaxis_destroy(Xaxis* xa)
{
    free(xa);
}

void xaxis_draw(Xaxis* xa, Group* g, uint32_t xstart, uint32_t width)
{
    uint32_t ix = xstart;
    g = fast_forward_groups(g, getmaxx(xa->win) - width);

    while (g != NULL) {
        if (g->id % XAXIS_TICK_SPACING == 0) {
            if (ix >= xa->xsize)
                return;
            //pl->xsize - pl->graph->xsize

            char dbuf[15] = {'\0'};
            char tbuf[15] = {'\0'};

            ts_to_dt(g->wstart, "%Y-%m-%d", dbuf, sizeof(dbuf));
            ts_to_dt(g->wstart, "%H:%M:%S", tbuf, sizeof(tbuf));

            // cut off strings at end of window so we don't overflow
            if (ix + strlen(dbuf) >= getmaxx(xa->win))
                dbuf[getmaxx(xa->win) - ix] = '\0';

            if (ix + strlen(tbuf) >= getmaxx(xa->win))
                tbuf[getmaxx(xa->win) - ix] = '\0';

            add_str(xa->win, 1, ix, xa->fgcol, xa->bgcol, dbuf);
            add_str(xa->win, 0, ix, xa->fgcol, xa->bgcol, tbuf);

        }
        g = g->next;
        ix++;
    }
}
