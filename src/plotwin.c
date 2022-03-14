#include "plotwin.h"


PlotWin* pw_init(WINDOW* win, Index* index, State* state, char* symbol, pthread_mutex_t* lock)
{
    /* Create a full new plot window, and start threads */
    PlotWin* pw = malloc(sizeof(PlotWin));
    pw->index = index;

    pw->plot = plot_init(win);

    pw->lines[0] = line_line_init("Volume");
    pw->lines[0]->color = CBLUE;
    pw->lines[1] = line_ohlc_init(symbol);

    yaxis_add_line(pw->plot->lyaxis, pw->lines[0]);
    yaxis_add_line(pw->plot->ryaxis, pw->lines[1]);

    pw->state = state;

    // Request arguments struct is passed to request thread
    pw->request = malloc(sizeof(Request));
    pw->request->index           = index;
    pw->request->lines           = pw->lines;
    pw->request->lock            = lock;
    pw->request->is_stopped      = false;
    //pw->request->is_stopped      = &(state->is_stopped);
    pw->request->OHLCinterval    = BINANCE_1_MINUTES;
    pw->request->limit           = 500;
    pw->request->timeout         = 60 * 1000 * 1000;
    strcpy(pw->request->symbol, symbol);

    pthread_create(&(pw->threadid), NULL, binance_read_thread, pw->request);
    return pw;
}

void pw_destroy(PlotWin* pw)
{
    plot_destroy(pw->plot);
    free(pw->request);
    free(pw);
}

int8_t pw_update_all(PlotWin** pws, uint32_t length, pthread_mutex_t* lock)
{
    for (int i=0 ; i<length ; i++) {
        if (i == pws[i]->state->cur_pw) {
            pws[i]->plot->lyaxis->bgcol = CBLACK;
            pws[i]->plot->ryaxis->bgcol = CBLACK;
        } else {
            pws[i]->plot->lyaxis->bgcol = CDEFAULT;
            pws[i]->plot->ryaxis->bgcol = CDEFAULT;
        }
        if (pw_update(pws[i], lock) < 0)
            return -1;
    }
    return 0;
}

int8_t pw_update(PlotWin* pw, pthread_mutex_t* lock)
{
    // check if data or exit early
    if (pw->index->npoints == 0)
        return 0;
        
    Plot* pl = pw->plot;
    Line* l0 = pw->lines[0];
    Line* l1 = pw->lines[1];
    Index* index = pw->index;
    State* s = pw->state;

    pthread_mutex_lock(lock);

    Groups* groups;
    if ((groups = index_get_grouped(index, s->gsize, pl->xsize, s->panx, s->pany)) == NULL) {
        pthread_mutex_unlock(lock);
        return 0;
    }

    pl->lyaxis->autorange = s->set_autorange;
    pl->ryaxis->autorange = s->set_autorange;
    pl->lyaxis->is_empty = true;
    pl->ryaxis->is_empty = true;

    if (line_set_data(l0, groups->lines[l0->lineid->id]) < 0) {
        pthread_mutex_unlock(lock);
        groups_destroy(groups);
        debug("Failed to set data for line 0\n");
        return -1;
    }
    if (line_set_data(l1, groups->lines[l1->lineid->id]) < 0) {
        pthread_mutex_unlock(lock);
        groups_destroy(groups);
        debug("Failed to set data for line 1\n");
        return -1;
    }

    status_set(pl->status, "autorange", "%d",    s->set_autorange);
    status_set(pl->status, "paused",    "%d",    s->is_paused);
    status_set(pl->status, "points",    "%d",    index->npoints);
    status_set(pl->status, "gsize",     "%d",    s->gsize);
    status_set(pl->status, "spread",    "%.3f",  index->spread);
    status_set(pl->status, "panxy",     "%d/%d", s->panx, s->pany);

    PlotStatus plstatus;
    if ((plstatus = plot_draw(pl, s)) < PLSUCCESS)
        plot_print_error(plstatus);

    // cleanup
    groups_destroy(groups);
    pthread_mutex_unlock(lock);
    return 0;
}

State* state_init()
{
    /* Shared state struct holds data about pan state, autorange etc... */
    State* s = malloc(sizeof(State));
    s->is_paused = false;
    s->is_stopped = false;
    s->panx = 0;
    s->pany = 0;
    s->is_pan_changed = false;
    s->set_autorange = true;

    s->do_create_pw = false;

    // amount of bins in a group
    s->gsize = DEFAULT_GROUP_SIZE;
    s->is_resized = false;

    s->pws = NULL;
    s->pws_length = 0;
    s->cur_pw = 0;

    return s;
}

int8_t state_resize_pws(PlotWin** pws, uint32_t length)
{
    // resize windows to fit as rows in terminal
    uint32_t wheight = LINES/length;

    for (uint32_t i=0 ; i<length ; i++) {
        PlotWin* p = pws[i];

        if (i == length-1)
            wresize(p->plot->win, LINES-(i*wheight), COLS);
        else
            wresize(p->plot->win, wheight, COLS);

        mvwin(p->plot->win, i*wheight, 0);
        plot_resize(p->plot);
        wrefresh(p->plot->win);
    }
    return 0;
}

int8_t state_add_pw(State* s, PlotWin* pw)
{
    /* Add a PlotWin struct to the state->pws array */
    s->pws_length++;
    s->pws = realloc(s->pws, s->pws_length * sizeof(PlotWin*));
    s->pws[s->pws_length-1] = pw;

    s->cur_pw = s->pws_length-1;;
    refresh();
    state_resize_pws(s->pws, s->pws_length);
    return 0;
}

int8_t state_remove_pw(State* s, PlotWin* pw)
{
    if (s->pws_length == 1) {
        debug("Not removing last window\n");
        return -1;
    }
    /* Remove PlotWin from array */
    PlotWin** pws = malloc(s->pws_length * sizeof(PlotWin*));
    PlotWin** ppws = pws;
    for (uint32_t i=0 ; i<s->pws_length ; i++) {
        if (s->pws[i] != pw)
            *ppws++ = s->pws[i];
    }
    // stopping thread
    pw->request->is_stopped = true;
    pthread_join(pw->threadid, NULL);

    pw_destroy(pw);

    free(s->pws);

    s->pws = pws;
    s->pws_length--;

    s->cur_pw = (s->cur_pw == 0) ? 0 : s->cur_pw-1;

    refresh();

    if (s->pws_length)
        state_resize_pws(s->pws, s->pws_length);

    return 0;
}
