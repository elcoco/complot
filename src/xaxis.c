#include "xaxis.h"

Xaxis* xaxis_init(WINDOW* parent)
{
    Xaxis* xa = malloc(sizeof(Xaxis));
    xa->parent = parent;
    xa->xsize = getmaxx(parent);
    xa->ysize = 2;
    xa->win = subwin(xa->parent, xa->ysize, xa->xsize, getmaxy(xa->parent)-xa->ysize, 0);
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

            add_str(xa->win, 1, ix, 3, dbuf);
            add_str(xa->win, 0, ix, 4, tbuf);
        }
        g = g->next;
        ix++;
    }
}
