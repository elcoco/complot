#include "xaxis.h"

Xaxis* xaxis_init()
{
    Xaxis* xa = malloc(sizeof(Xaxis));
    xa->ysize = 2;
    xa->win = NULL;
    return xa;
}

void xaxis_destroy(Xaxis* xa)
{
    free(xa);
}

void xaxis_draw(Xaxis* xa, Group* g, uint32_t xstart)
{
    int32_t ix = xstart;

    while (g != NULL) {
        if (g->id % XTICK_SPACING == 0 && !g->is_empty) {
            if (ix >= xa->xsize)
                return;

            char dbuf[15] = {'\0'};
            char tbuf[15] = {'\0'};

            ts_to_dt(g->wstart, "%Y-%m-%d", dbuf, sizeof(dbuf));
            ts_to_dt(g->wstart, "%H:%M:%S", tbuf, sizeof(tbuf));

            add_str(xa->win, 1, ix, CWHITE, dbuf);
            add_str(xa->win, 0, ix, CWHITE, tbuf);
        }
        g = g->next;
        ix++;
    }
}
