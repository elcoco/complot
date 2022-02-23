#include "plot.h"


Plot* plot_init(WINDOW* parent, uint32_t ysize)
{

    Plot* pl = malloc(sizeof(Plot));

    pl->parent = parent;
    pl->win = subwin(pl->parent, ysize, getmaxx(pl->parent), 0, 0);

    pl->xsize = getmaxx(pl->parent);
    pl->ysize = ysize;

    pl->xaxis = xaxis_init(pl->win);

    int yaxis_ysize = pl->ysize - pl->xaxis->ysize;
    pl->lyaxis = axis_init(pl->win, AXIS_LEFT, yaxis_ysize);
    pl->ryaxis = axis_init(pl->win, AXIS_RIGHT, yaxis_ysize);

    int graph_xsize = pl->xsize - (pl->lyaxis->xsize+pl->ryaxis->xsize);
    int graph_ysize = pl->ysize - pl->xaxis->ysize;
    pl->graph = graph_init(pl->win, graph_ysize, graph_xsize, pl->lyaxis->xsize);

    return pl;
}

void plot_destroy(Plot* pl)
{
    axis_destroy(pl->lyaxis);
    axis_destroy(pl->ryaxis);
    xaxis_destroy(pl->xaxis);
    free(pl->graph);
    free(pl);
}

Graph* graph_init(WINDOW* parent, uint32_t ysize, uint32_t xsize, uint32_t xstart)
{
    /* Area where the plot is being drawn */
    Graph* gr = malloc(sizeof(Graph));
    gr->parent = parent;

    gr->xsize = xsize;
    gr->ysize = ysize;
    //gr->ysize = getmaxy(gr->parent);

    gr->win = subwin(gr->parent, gr->ysize, gr->xsize, 0, xstart);
    return gr;
}


void graph_resize(Graph* gr, Plot* pl)
{
    // yaxis changes width to data so we need to resize graph window accordingly
    pl->graph->xsize = pl->xsize - (pl->lyaxis->xsize + pl->ryaxis->xsize); 
    pl->graph->ysize = pl->ysize - pl->xaxis->ysize;
    delwin(pl->graph->win);
    pl->graph->win = subwin(pl->win, pl->graph->ysize, pl->graph->xsize, 0, pl->lyaxis->xsize);
}

int8_t plot_resize(Plot* pl, uint32_t ysize)
{
    /* resize all windows */
    if (ysize > LINES)
        return -1; 

    pl->xsize = getmaxx(pl->parent);
    pl->ysize = ysize;
    wresize(pl->win, pl->ysize, pl->xsize);

    // resize xaxis
    pl->xaxis->xsize = pl->xsize;
    delwin(pl->xaxis->win);
    pl->xaxis->win = subwin(pl->win, pl->xaxis->ysize, pl->xaxis->xsize, pl->ysize-pl->xaxis->ysize, 0);

    // trigger yaxis window resize/recreate
    int yaxis_ysize = pl->ysize - pl->xaxis->ysize;
    pl->lyaxis->ysize = yaxis_ysize;
    pl->ryaxis->ysize = yaxis_ysize;

    pl->lyaxis->xsize = 0;
    pl->ryaxis->xsize = 0;
    axis_set_window_width(pl->lyaxis);
    axis_set_window_width(pl->ryaxis);

    graph_resize(pl->graph, pl);
    return 0;
}

void plot_draw(Plot* pl, Groups* groups, State* s)
{
    // check if window is too small
    if (getmaxx(pl->win) < MIN_WINDOW_XSIZE)
        return;
    if (getmaxy(pl->win) < MIN_WINDOW_YSIZE)
        return;

    // Handle terminal resize
    if (pl->xsize != getmaxx(pl->win) || pl->ysize != getmaxy(pl->win)) {
        if (plot_resize(pl, pl->ysize) < 0)
            return;
    }

    // Check if terminal is too narrow to fit windows
    uint32_t min_xsize = pl->lyaxis->xsize + pl->ryaxis->xsize + 15;
    if (min_xsize > getmaxx(pl->win)) {
        return;
    }

    // Find the window width of the yaxis so we can calculate the graph width
    if (axis_set_window_width(pl->lyaxis) >0 || axis_set_window_width(pl->ryaxis) >0) {
        graph_resize(pl->graph, pl);
    }

    clear_win(pl->graph->win);
    clear_win(pl->xaxis->win);

    //fill_win(pl->xaxis->win, 'X');
    axis_draw(pl->lyaxis, pl->graph->win, groups, s);
    axis_draw(pl->ryaxis, pl->graph->win, groups, s);

    Group* g = fast_forward_groups(groups->group, pl->xsize-pl->graph->xsize);
    xaxis_draw(pl->xaxis, g, pl->lyaxis->xsize);

    touchwin(pl->win);
    refresh();
    wrefresh(pl->win);
    wrefresh(pl->lyaxis->win);
    wrefresh(pl->graph->win);
    wrefresh(pl->ryaxis->win);
    wrefresh(pl->xaxis->win);

}
