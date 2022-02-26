#include "plot.h"


Plot* plot_init(WINDOW* parent)
{
    /* passed parent window decides the dimensions of the plot we're going to render in it */

    Plot* pl = malloc(sizeof(Plot));

    pl->parent = parent;
    pl->xsize = getmaxx(pl->parent);
    pl->ysize = getmaxy(pl->parent);

    pl->win = subwin(pl->parent, pl->ysize, pl->xsize, 0, 0);

    // create all struct that represent features in the plot
    pl->xaxis   = xaxis_init();
    pl->lyaxis  = yaxis_init(pl->win, AXIS_LEFT);
    pl->ryaxis  = yaxis_init(pl->win, AXIS_RIGHT);
    pl->graph   = graph_init();
    pl->llegend = legend_init(pl->lyaxis);
    pl->rlegend = legend_init(pl->ryaxis);
    pl->status  = status_init();

    // set all sizes and (re)create all windows
    plot_resize(pl);

    return pl;
}

Legend* legend_init(Yaxis* ya)
{
    Legend* le = malloc(sizeof(Legend));
    le->ysize = 1;
    le->yaxis = ya;
    return le;
}

void legend_draw(Legend* le)
{
    /* draw left or right legend */
    // TODO is writing outside of window area but is dificult to fix because UTF8 chars are wider than ascii
    Line* l = le->yaxis->line;
    uint32_t xpos = 0;

    while (l != NULL) {
        char buf[LEGEND_MAX_SIZE] = {'\0'};
        sprintf(buf, "%s = %s", l->icon, l->name);

        if (l->next != NULL)
            strcat(buf, ", ");

        add_str(le->win, 0, xpos, l->color, buf);

        // can't do strlen on buf because the icon is a multi byte unicode character
        xpos += strlen(l->name) + 6;
        l = l->next;
    }
}

void legend_destroy(Legend* le)
{
    free(le);
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

    // resize main window
    pl->xsize = getmaxx(pl->parent);
    pl->ysize = getmaxy(pl->parent);
    wresize(pl->win, pl->ysize, pl->xsize);

    // resize statusbar
    pl->status->xsize = pl->xsize;
    delwin(pl->status->win);
    pl->status->win = subwin(pl->win, pl->status->ysize, pl->status->xsize, pl->ysize-1, 0);

    // resize xaxis
    pl->xaxis->xsize = pl->xsize;
    delwin(pl->xaxis->win);
    uint32_t xaxis_ypos = pl->ysize - pl->xaxis->ysize - pl->status->ysize;
    pl->xaxis->win = subwin(pl->win, pl->xaxis->ysize, pl->xaxis->xsize, xaxis_ypos, 0);

    // trigger yaxis window resize/recreate
    int yaxis_ysize = pl->ysize - pl->xaxis->ysize - pl->llegend->ysize - pl->status->ysize;

    pl->lyaxis->xsize = 0;
    pl->lyaxis->ysize = yaxis_ysize;
    yaxis_set_window_width(pl->lyaxis);

    pl->ryaxis->xsize = 0;
    pl->ryaxis->ysize = yaxis_ysize;
    yaxis_set_window_width(pl->ryaxis);

    // resize left and right legend
    pl->llegend->xsize = floor(pl->xsize/2);
    pl->llegend->win = subwin(pl->win, pl->llegend->ysize, pl->llegend->xsize, 0, 0);
    pl->rlegend->xsize = floor(pl->xsize/2);
    pl->rlegend->win = subwin(pl->win, pl->rlegend->ysize, pl->rlegend->xsize, 0, pl->xsize-pl->rlegend->xsize);

    //graph_resize(pl->graph, pl);
    // yaxis changes width to data so we need to resize graph window accordingly
    pl->graph->xsize = pl->xsize - (pl->lyaxis->xsize + pl->ryaxis->xsize); 
    pl->graph->ysize = pl->ysize - pl->xaxis->ysize - pl->llegend->ysize - pl->status->ysize;
    delwin(pl->graph->win);
    pl->graph->win = subwin(pl->win, pl->graph->ysize, pl->graph->xsize, pl->llegend->ysize, pl->lyaxis->xsize);

    return 0;
}

void plot_draw(Plot* pl, Groups* groups, State* s)
{
    // check if window is too small
    if (getmaxx(pl->win) < MIN_WINDOW_XSIZE || getmaxy(pl->win) < MIN_WINDOW_YSIZE)
        return;

    // Handle terminal resize
    if (pl->xsize != getmaxx(pl->win) || pl->ysize != getmaxy(pl->win)) {
        if (plot_resize(pl) < 0)
            return;
    }

    // Check if terminal is too narrow to fit windows
    uint32_t min_xsize = pl->lyaxis->xsize + pl->ryaxis->xsize + 15;
    if (min_xsize > getmaxx(pl->win)) 
        return;

    // Find the window width of the yaxis so we can calculate the graph width
    if (yaxis_set_window_width(pl->lyaxis) >0 || yaxis_set_window_width(pl->ryaxis) >0)
        plot_resize(pl);

    clear_win(pl->graph->win);
    clear_win(pl->xaxis->win);
    clear_win(pl->llegend->win);
    clear_win(pl->rlegend->win);
    clear_win(pl->status->win);

    //fill_win(pl->xaxis->win, 'X');
    yaxis_draw(pl->lyaxis, pl->graph->win, groups, s);
    yaxis_draw(pl->ryaxis, pl->graph->win, groups, s);

    Group* g = fast_forward_groups(groups->group, pl->xsize-pl->graph->xsize);
    xaxis_draw(pl->xaxis, g, pl->lyaxis->xsize);

    legend_draw(pl->llegend);
    legend_draw(pl->rlegend);

    status_draw(pl->status);

    touchwin(pl->win);
    refresh();
    wrefresh(pl->win);
}
