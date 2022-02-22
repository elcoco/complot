#include "plot.h"


Plot* plot_init(WINDOW* parent, uint32_t ysize)
{

    Plot* pl = malloc(sizeof(Plot));

    pl->parent = parent;
    pl->win = subwin(pl->parent, ysize, getmaxx(pl->parent), 0, 0);

    pl->xsize = getmaxx(pl->parent);
    pl->ysize = ysize;

    pl->lyaxis = axis_init(pl->win, AXIS_LEFT);
    pl->ryaxis = axis_init(pl->win, AXIS_RIGHT);

    int graph_xsize = pl->xsize - (pl->lyaxis->xsize+pl->ryaxis->xsize);
    pl->graph = graph_init(pl->win, pl->lyaxis->xsize, graph_xsize);
    return pl;
}

void plot_destroy(Plot* pl)
{
    axis_destroy(pl->lyaxis);
    axis_destroy(pl->ryaxis);
    free(pl->graph);
    free(pl);
}

Graph* graph_init(WINDOW* parent, uint32_t xstart, uint32_t xsize)
{
    /* Area where the plot is being drawn */
    Graph* gr = malloc(sizeof(Graph));
    gr->parent = parent;

    gr->xsize = xsize;
    gr->ysize = getmaxy(gr->parent);

    gr->win = subwin(gr->parent, gr->ysize, gr->xsize, 0, xstart);
    return gr;
}


void plot_draw(Plot* pl, Groups* groups, State* s)
{
    // Find the window width of the axis so we can calculate the graph width
    axis_set_window_width(pl->lyaxis);
    axis_set_window_width(pl->ryaxis);

    int new_grxsize = pl->xsize - (pl->lyaxis->xsize + pl->ryaxis->xsize);

    delwin(pl->graph->win);
    int graph_xsize = pl->xsize - (pl->lyaxis->xsize+pl->ryaxis->xsize);
    pl->graph->win = subwin(pl->win, pl->ysize, graph_xsize, 0, pl->lyaxis->xsize);

    clear_win(pl->graph->win);

    axis_draw(pl->lyaxis, pl->graph->win, groups, s);
    axis_draw(pl->ryaxis, pl->graph->win, groups, s);

    //fill_win(pl->lyaxis->win, pl->lyaxis->ysize, pl->lyaxis->xsize, 'L');
    //fill_win(pl->graph->win, pl->graph->ysize, pl->graph->xsize, 'M');
    //fill_win(pl->ryaxis->win, pl->ryaxis->ysize, pl->ryaxis->xsize, 'R');

    refresh();
    wrefresh(pl->win);
    wrefresh(pl->lyaxis->win);
    wrefresh(pl->graph->win);
    wrefresh(pl->ryaxis->win);

}
