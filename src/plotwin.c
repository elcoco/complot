#include "plotwin.h"


PlotWin* pw_init(WINDOW* win, State* state, char* symbol, pthread_mutex_t* lock)
{
    /* Create a full new plot window, and start threads */
    PlotWin* pw = malloc(sizeof(PlotWin));

    if ((pw->index = index_init(MAX_LINES)) == NULL)
        return NULL;

    pw->plot = plot_init(win);

    pw->lines[0] = line_line_init("Volume");
    pw->lines[0]->color = CBLUE;
    pw->lines[1] = line_ohlc_init(symbol);

    yaxis_add_line(pw->plot->lyaxis, pw->lines[0]);
    yaxis_add_line(pw->plot->ryaxis, pw->lines[1]);

    pw->state = state;

    // Request arguments struct is passed to request thread
    pw->request = malloc(sizeof(Request));
    pw->request->index           = pw->index;
    pw->request->lines           = pw->lines;
    pw->request->lock            = lock;
    pw->request->is_stopped      = false;
    pw->request->OHLCinterval    = BINANCE_5_MINUTES;
    pw->request->limit           = 1000;
    pw->request->timeout         = 60 * 1000 * 1000;
    strcpy(pw->request->symbol, symbol);

    // amount of bins in a group
    pw->gsize = DEFAULT_GROUP_SIZE;
    pw->is_resized = false;
    pw->is_paused = false;
    pw->panx = 0;
    pw->pany = 0;
    pw->is_pan_changed = false;
    pw->set_autorange = true;

    pthread_create(&(pw->threadid), NULL, binance_read_thread, pw->request);
    return pw;
}

void pw_destroy(PlotWin* pw)
{
    plot_destroy(pw->plot);
    index_destroy(pw->index);
    free(pw->request);
    free(pw);
}

char* pw_select_interval(PlotWin* pw, const char** intervals)
{
    /* Select new interval for candlesticks.
     * Recreate index and restart thread with new interval. */

    char* result = menu_show(intervals, 17, 7);
    if (strlen(result) <= 0)
        return NULL;


    pw->request->is_stopped = true;
    pthread_join(pw->threadid, NULL);
    pw->request->is_stopped = false;
    for (int i=0 ; intervals[i] != NULL ; i++) {
        if (strcmp(result, intervals[i]) == 0)
            pw->request->OHLCinterval = i;
    }

    index_destroy(pw->index);
    if ((pw->index = index_init(MAX_LINES)) == NULL)
        return 0;

    pw->request->index = pw->index;
    pthread_create(&(pw->threadid), NULL, binance_read_thread, pw->request);

    return result;
}

int8_t pw_update_all(PlotWin** pws, uint32_t length, pthread_mutex_t* lock, bool force)
{
    for (int i=0 ; i<length ; i++) {
        if (i == pws[i]->state->cur_pw) {
            pws[i]->plot->lyaxis->bgcol = CBLACK;
            pws[i]->plot->ryaxis->bgcol = CBLACK;
        } else {
            pws[i]->plot->lyaxis->bgcol = CDEFAULT;
            pws[i]->plot->ryaxis->bgcol = CDEFAULT;
        }
        if (pw_update(pws[i], lock, force) < 0)
            return -1;
    }
    return 0;
}

int8_t pw_update(PlotWin* pw, pthread_mutex_t* lock, bool force)
{
    // check if thread is still alive
    if (pw->request->is_stopped) {
        ui_show_error(pw->plot->win, "Failed to get data for symbol %s!", pw->request->symbol);
        //debug("ERROR: thread is stopped, die now please!\n");
        //state_remove_pw(pw->state, pw);
        //return -1;
    }
        
    if (!force) {
        if (!index_has_new_data(pw->index))
            return 0;

        // check if data or exit early
        if (pw->index->npoints == 0)
            return 0;

        // exit if plot is paused
        if (pw->is_paused)
            return 0;
    }

    Plot* pl = pw->plot;
    Line* l0 = pw->lines[0];
    Line* l1 = pw->lines[1];
    Index* index = pw->index;

    pthread_mutex_lock(lock);

    Groups* groups;
    if ((groups = index_get_grouped(index, pw->gsize, pl->xsize, pw->panx, pw->pany)) == NULL) {
        pthread_mutex_unlock(lock);
        return 0;
    }

    pl->lyaxis->autorange = pw->set_autorange;
    pl->ryaxis->autorange = pw->set_autorange;
    pl->lyaxis->is_empty = true;
    pl->ryaxis->is_empty = true;

    // set y offset
    pl->pany = pw->pany;

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

    status_set(pl->status, "autorange", "%d",    pw->set_autorange);
    status_set(pl->status, "paused",    "%d",    pw->is_paused);
    status_set(pl->status, "points",    "%d",    index->npoints);
    status_set(pl->status, "gsize",     "%d",    pw->gsize);
    status_set(pl->status, "spread",    "%.3f",  index->spread);
    status_set(pl->status, "panxy",     "%d/%d", pw->panx, pw->pany);
    status_set(pl->status, "interval",  "%s",    binance_interval_map[pw->request->OHLCinterval]);

    PlotStatus plstatus;
    if ((plstatus = plot_draw(pl)) < PLSUCCESS)
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
    s->is_stopped = false;

    s->do_create_pw = false;

    s->is_resized = false;

    s->pws = NULL;
    s->pws_length = 0;
    s->cur_pw = 0;

    return s;
}

void state_destroy(State* s)
{
    for (int i=0 ; i<s->pws_length ; i++) {
        state_remove_pw(s, s->pws[i]);
    }

    free(s->pws);
    free(s);
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
        ui_refresh(p->plot->win);
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
    ui_refresh(NULL);
    state_resize_pws(s->pws, s->pws_length);
    return 0;
}

int8_t state_remove_pw(State* s, PlotWin* pw)
{
    /* Remove PlotWin from array, resize array and destroy struct */
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

    ui_refresh(NULL);

    if (s->pws_length)
        state_resize_pws(s->pws, s->pws_length);

    return 0;
}
