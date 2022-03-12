#include "plot.h"


Plot* plot_init(WINDOW* win)
{
    /* passed parent window decides the dimensions of the plot we're going to render in it */

    Plot* pl = malloc(sizeof(Plot));

    pl->win = win;
    pl->xsize = getmaxx(pl->win);
    pl->ysize = getmaxy(pl->win);

    // create all struct that represent features in the plot
    pl->xaxis   = xaxis_init();
    pl->lyaxis  = yaxis_init(pl->win, AXIS_LEFT);
    pl->ryaxis  = yaxis_init(pl->win, AXIS_RIGHT);
    pl->graph   = graph_init();
    pl->llegend = legend_init(pl->lyaxis);
    pl->rlegend = legend_init(pl->ryaxis);
    pl->status  = status_init();

    // set all sizes and create all windows
    plot_resize(pl);

    return pl;
}

void plot_destroy(Plot* pl)
{
    yaxis_destroy(pl->lyaxis);
    yaxis_destroy(pl->ryaxis);
    xaxis_destroy(pl->xaxis);
    legend_destroy(pl->llegend);
    legend_destroy(pl->rlegend);
    free(pl->graph);
    status_destroy(pl->status);
    free(pl);
}

Graph* graph_init()
{
    /* Area where the plot is being drawn */
    Graph* gr = malloc(sizeof(Graph));
    gr->win = NULL;
    return gr;
}

int8_t plot_resize(Plot* pl)
{
    /* resize/create all windows to fit parent window
     * Since maintaining proper window (re)sizes is difficult,
     * i centralized all of it to make things a bit more managable */

    pl->xsize = getmaxx(pl->win);
    pl->ysize = getmaxy(pl->win);


    // resize statusbar
    pl->status->xsize = pl->xsize;
    delwin(pl->status->win);
    pl->status->win = derwin(pl->win, pl->status->ysize, pl->status->xsize, pl->ysize-1, 0);


    // resize xaxis
    pl->xaxis->xsize = pl->xsize;
    delwin(pl->xaxis->win);
    uint32_t xaxis_ypos = pl->ysize - pl->xaxis->ysize - pl->status->ysize;
    pl->xaxis->win = derwin(pl->win, pl->xaxis->ysize, pl->xaxis->xsize, xaxis_ypos, 0);

    // trigger yaxis window resize/recreate
    int yaxis_ysize = pl->ysize - pl->xaxis->ysize - pl->llegend->ysize - pl->status->ysize;

    pl->lyaxis->xsize = 0;
    pl->lyaxis->ysize = yaxis_ysize;
    if (yaxis_set_window_width(pl->lyaxis) < 0) {
        debug("Failed to set window width of lyaxis\n");
        return -1;
    }

    pl->ryaxis->xsize = 0;
    pl->ryaxis->ysize = yaxis_ysize;
    if (yaxis_set_window_width(pl->ryaxis) < 0) {
        debug("Failed to set window width of ryaxis\n");
        return -1;
    }

    // resize left and right legend
    pl->llegend->xsize = floor(pl->xsize/2);
    pl->llegend->win = derwin(pl->win, pl->llegend->ysize, pl->llegend->xsize, 0, 0);
    pl->rlegend->xsize = floor(pl->xsize/2);
    pl->rlegend->win = derwin(pl->win, pl->rlegend->ysize, pl->rlegend->xsize, 0, pl->xsize-pl->rlegend->xsize);

    // yaxis changes width to data so we need to resize graph window accordingly
    pl->graph->xsize = pl->xsize - (pl->lyaxis->xsize + pl->ryaxis->xsize); 
    pl->graph->ysize = pl->ysize - pl->xaxis->ysize - pl->llegend->ysize - pl->status->ysize;
    delwin(pl->graph->win);
    pl->graph->win = derwin(pl->win, pl->graph->ysize, pl->graph->xsize, pl->llegend->ysize, pl->lyaxis->xsize);

    assert(pl->win);
    assert(pl->lyaxis->win && "Failed to create lyaxis window");
    assert(pl->ryaxis->win && "Failed to create ryaxis window");
    assert(pl->xaxis->win  && "Failed to create xaxis window");
    assert(pl->status->win && "Failed to create status window");
    assert(pl->graph->win  && "Failed to create graph window");

    return 0;
}

void plot_show_error(WINDOW* win, char* msg)
{
    clear_win(win);
    add_str(win, 0, 0, CRED, msg);
    wrefresh(win);
}

PlotError plot_draw(Plot* pl, State* s)
{
    if(pl->lyaxis->is_empty || pl->ryaxis->is_empty) {
        plot_show_error(pl->win, "NO DATA!");
        return PLOT_ERR_NO_DATA;
    }

    // Check if terminal is too narrow to fit windows
    uint32_t min_xsize = pl->lyaxis->xsize + pl->ryaxis->xsize + GRAPH_MIN_SIZE;
    if (min_xsize > getmaxx(pl->win))  {
        plot_show_error(pl->win, "WINDOW TOO SMOLL!");
        return PLOT_ERR_TOO_SMALL;
    }

    // Handle terminal resize
    if (pl->xsize != getmaxx(pl->win) || pl->ysize != getmaxy(pl->win)) {
        if (plot_resize(pl) < 0)
            return PLOT_ERR_RESIZE_FAILED;
    }
    
    // Find the window width of the yaxis so we can calculate the graph width
    int8_t res;
    if ((res = yaxis_set_window_width(pl->lyaxis)) < 0) {
        return PLOT_ERR_RESIZE_FAILED;
    } else if (res > 0) {
        if (plot_resize(pl) < 0)
            return PLOT_ERR_RESIZE_FAILED;
    }

    if ((res = yaxis_set_window_width(pl->ryaxis)) < 0) {
        return PLOT_ERR_RESIZE_FAILED;
    } else if (res > 0) {
        if (plot_resize(pl) < 0)
            return PLOT_ERR_RESIZE_FAILED;
    }

    clear_win(pl->graph->win);
    clear_win(pl->xaxis->win);
    clear_win(pl->llegend->win);
    clear_win(pl->rlegend->win);
    clear_win(pl->status->win);

    yaxis_draw(pl->lyaxis, pl->graph->win, s);
    yaxis_draw(pl->ryaxis, pl->graph->win, s);

    // find data to display on x axis in Yaxis
    GroupContainer* gc;
    if ((gc = yaxis_get_gc(pl->lyaxis)) != NULL)
        xaxis_draw(pl->xaxis, gc->group, pl->lyaxis->xsize, pl->graph->xsize);
    else if ((gc = yaxis_get_gc(pl->ryaxis)) != NULL)
        xaxis_draw(pl->xaxis, gc->group, pl->lyaxis->xsize, pl->graph->xsize);
    else
        debug("No data found\n");

    legend_draw(pl->llegend);
    legend_draw(pl->rlegend);

    status_draw(pl->status);

    touchwin(pl->win);
    refresh();
    wrefresh(pl->win);
    return PLOT_SUCCESS;
}
