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

    pl->show_status = false;
    pl->show_legend = true;
    pl->show_xaxis = true;

    pl->pany = 0;

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

PlotStatus plot_resize(Plot* pl)
{
    /* resize/create all windows to fit parent window
     * Since maintaining proper window (re)sizes is hard,
     * i centralized all of it to make things a bit more managable */
    // set error here
    PlotStatus plstatus;

    pl->xsize = getmaxx(pl->win);
    pl->ysize = getmaxy(pl->win);

    uint32_t status_ysize = (pl->show_status) ? pl->status->ysize : 0;
    uint32_t legend_ysize = (pl->show_legend) ? pl->llegend->ysize : 0;
    uint32_t xaxis_ysize = (pl->show_xaxis) ? pl->xaxis->ysize : 0;

    // resize statusbar
    if (pl->show_status) {
        pl->status->xsize = pl->xsize;
        delwin(pl->status->win);
        pl->status->win = derwin(pl->win, status_ysize, pl->status->xsize, pl->ysize-1, 0);
        assert(pl->status->win && "Failed to create status window");
    }

    // resize xaxis
    if (pl->show_xaxis) {
        pl->xaxis->xsize = pl->xsize;
        delwin(pl->xaxis->win);
        uint32_t xaxis_ypos = pl->ysize - xaxis_ysize - status_ysize;
        pl->xaxis->win = derwin(pl->win, xaxis_ysize, pl->xaxis->xsize, xaxis_ypos, 0);
        assert(pl->xaxis->win  && "Failed to create xaxis window");
    }

    // trigger yaxis window resize/recreate
    int yaxis_ysize = pl->ysize - xaxis_ysize - legend_ysize - status_ysize;

    pl->lyaxis->xsize = 0;
    pl->lyaxis->ysize = yaxis_ysize;
    if ((plstatus = yaxis_set_window_width(pl->lyaxis, legend_ysize)) < PLSUCCESS)
        return plstatus;

    pl->ryaxis->xsize = 0;
    pl->ryaxis->ysize = yaxis_ysize;
    if ((plstatus = yaxis_set_window_width(pl->ryaxis, legend_ysize)) < PLSUCCESS)
        return plstatus;

    // resize left and right legend
    if (pl->show_legend) {
        pl->llegend->xsize = floor(pl->xsize/2);
        pl->llegend->win = derwin(pl->win, legend_ysize, pl->llegend->xsize, 0, 0);
        pl->rlegend->xsize = floor(pl->xsize/2);
        pl->rlegend->win = derwin(pl->win, legend_ysize, pl->rlegend->xsize, 0, pl->xsize-pl->rlegend->xsize);
    }

    // yaxis changes width to data so we need to resize graph window accordingly
    pl->graph->xsize = pl->xsize - (pl->lyaxis->xsize + pl->ryaxis->xsize); 
    pl->graph->ysize = pl->ysize - xaxis_ysize - legend_ysize - status_ysize;
    delwin(pl->graph->win);
    pl->graph->win = derwin(pl->win, pl->graph->ysize, pl->graph->xsize, legend_ysize, pl->lyaxis->xsize);

    assert(pl->win);
    assert(pl->lyaxis->win && "Failed to create lyaxis window");
    assert(pl->ryaxis->win && "Failed to create ryaxis window");
    assert(pl->graph->win  && "Failed to create graph window");

    return PLSUCCESS;
}

PlotStatus plot_draw(Plot* pl)
{
    // set return status here
    PlotStatus plstatus;

    if(pl->lyaxis->is_empty || pl->ryaxis->is_empty) {
        ui_show_error(pl->win, "NO DATA!");
        return PLERR_NO_DATA;
    }

    // Handle terminal resize
    if (pl->xsize != getmaxx(pl->win) || pl->ysize != getmaxy(pl->win)) {
        debug("!!!!!!!!!!!!!!!!!!1 resize selecta\n");
        if ((plstatus = plot_resize(pl)) < PLSUCCESS)
            return plstatus;
    }
    
    // Check if terminal is too narrow to fit windows
    uint32_t min_xsize = pl->lyaxis->xsize + pl->ryaxis->xsize + GRAPH_MIN_SIZE;
    if (min_xsize > getmaxx(pl->win))  {
        ui_show_error(pl->win, "WINDOW TOO SMOLL!");
        debug("min size: %d = %d + %d\n", min_xsize, pl->lyaxis->xsize, pl->ryaxis->xsize);
        return PLERR_WINDOW_TOO_SMALL;
    }

    // Find the window width of the yaxis so we can calculate the graph width
    // TODO very ugly solution to the problem, must find out if yaxis changed width
    //      without calculating legend_ysize
    uint32_t legend_ysize = (pl->show_legend) ? pl->llegend->ysize : 0;

    if ((plstatus = yaxis_set_window_width(pl->lyaxis, legend_ysize)) < PLSUCCESS) {
        return plstatus;
    }
    else if (plstatus == PLSTATUS_YAXIS_CHANGED) {
        if ((plstatus = plot_resize(pl)) < PLSUCCESS)
            return plstatus;
    }

    if ((plstatus = yaxis_set_window_width(pl->ryaxis, legend_ysize)) < PLSUCCESS) {
        return plstatus;
    }
    else if (plstatus == PLSTATUS_YAXIS_CHANGED) {
        if ((plstatus = plot_resize(pl)) < PLSUCCESS)
            return plstatus;
    }

    ui_erase(pl->graph->win);
    yaxis_draw(pl->lyaxis, pl->graph->win, pl->pany);
    yaxis_draw(pl->ryaxis, pl->graph->win, pl->pany);

    // find data to display on x axis in Yaxis
    if (pl->show_xaxis)
        ui_erase(pl->xaxis->win);

    GroupContainer* gc;
    if (pl->show_xaxis && (gc = yaxis_get_gc(pl->lyaxis)) != NULL)
        xaxis_draw(pl->xaxis, gc->group, pl->lyaxis->xsize, pl->graph->xsize);
    else if (pl->show_xaxis && (gc = yaxis_get_gc(pl->ryaxis)) != NULL)
        xaxis_draw(pl->xaxis, gc->group, pl->lyaxis->xsize, pl->graph->xsize);

    if (pl->show_legend) {
        ui_erase(pl->llegend->win);
        ui_erase(pl->rlegend->win);
        legend_draw(pl->llegend);
        legend_draw(pl->rlegend);
    }

    if (pl->show_status) {
        ui_erase(pl->status->win);
        status_draw(pl->status);
    }

    touchwin(pl->win);
    ui_refresh(NULL);
    ui_refresh(pl->win);
    return PLSUCCESS;
}

void plot_print_error(PlotStatus plstatus)
{
    /* Display error message that is returned from functions */
    switch (plstatus) {
        case PLERR_NO_DATA:
            debug("ERROR: No data\n");
            break;
        case PLERR_WINDOW_TOO_SMALL:
            debug("ERROR: Window too small\n");
            break;
        case PLERR_RESIZE_FAILED:
            debug("ERROR: Failed to resize window\n");
            break;
        // we don't want no compiler warnings!
        case PLSUCCESS:
        case PLSTATUS_YAXIS_UNCHANGED:
        case PLSTATUS_YAXIS_CHANGED:
    }
}
