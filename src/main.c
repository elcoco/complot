#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h> // read
#include <locale.h>     // for utf8 in curses
#include <signal.h>     // catch SIGTERM
#include <pthread.h>
#include <curses.h>

#include "index.h"
#include "utils.h"
#include "plot.h"
#include "ui.h"
#include "read_thread.h"

#define NLINES 5

#define SLEEP_MS 100*1000

#define DEFAULT_GROUP_SIZE 1
#define DEFAULT_PAN_STEPS 3
#define DEFAULT_PAN_BIG_STEPS 5


// thread lock
pthread_mutex_t lock;

// DONE extend index when out of bounds
// DONE get groups x amount of groups from last known data
// DONE create points linked list
// DONE create CSV file reader
// DONE add xwin dimension to group
// DONE draw candlesticks
// DONE return groups as linked list
// DONE keep track of data min and max. we need this info when drawing candlesticks
// DONE segfault when GROUP_SIZE = 1 or 2
// DONE now index data limits are used, but group data limits allows for auto scale
// DONE dataleak for Points, Viewport and groups
// DONE option disable autoscale
//
// TODO find a way to decide on the precision of tickers on axis
// DONE x tickers should follow data not columns
// DONE reindex on second datapoint and calculate spread dynamically
// DONE on reindex first point is added to linked list again
// DONE create update function in index
// DONE rename component to plot oid
// DONE write better makefile
// DONE x should also be double
// DONE plot should have axis and line structs to organize what should be drawn where
// DONE rename pl (plot) to p and p (point) to pnt
// DONE rename pl_ function names to plot_
// DONE don't recreate plot on every iteration, not pretty
// DONE draw legend from axis
// TODO when not using autoscale, it should not change axis scale
// DONE nasty bug in index_get_grouped where we're trying to access a non existing group
// DONE auto resize plot
// DONE status window
// DONE x axis window/struct
// DONE rename axis to yaxis (a -> ya)
// TODO create non candlestick lines
// TODO create pop up log window
// TODO write a bit of documentation about the way the indexer works before i forget all of it
// TODO test multiple lines on multiple axis
// TODO request many lines from index so we can process them at once in the draw function


int sigint_caught = 0;
LineID lineid0 = {.lineid=0, .ltype=LTYPE_LINE};
LineID lineid1 = {.lineid=1, .ltype=LTYPE_OHLC};


void on_sigint(int signum)
{
    sigint_caught = 1;
}

void set_defaults(State* s)
{
    s->is_paused = false;
    s->is_stopped = false;
    s->panx = 0;
    s->pany = 0;
    s->is_pan_changed = false;
    s->set_autorange = true;
    s->fit_all = false;

    s->dmin = -1;
    s->dmax = -1;

    // amount of bins in a group
    s->gsize = DEFAULT_GROUP_SIZE;
    s->is_resized = false;
}

bool check_user_input(void* arg)
{
    // s struct is passed as an argument to a callback, cast it to the proper type
    State* s = arg;
    //MEVENT event;       // curses mouse event
                        //
    /* check for user input, return 1 if there was input */
    int c = getch();

    if (c != ERR) {
        switch (c) {
            case 'q':
                s->is_stopped = true;
                break;
            case 'h':
                s->panx-=DEFAULT_PAN_STEPS;
                s->is_pan_changed = true;
                s->is_paused = true;
                break;
            case 'l':
                s->panx+=DEFAULT_PAN_STEPS;
                s->is_pan_changed = true;
                s->is_paused = true;
                break;
            case 'k':
                s->pany-=DEFAULT_PAN_STEPS;
                s->is_pan_changed = true;
                s->set_autorange = false;
                break;
            case 'j':
                s->pany+=DEFAULT_PAN_STEPS;
                s->is_pan_changed = true;
                s->set_autorange = false;
                break;
            case 'H':
                if (s->gsize > 1)
                    s->gsize--;
                //s->panx-=DEFAULT_PAN_BIG_STEPS;
                //s->is_pan_changed = true;
                break;
            case 'L':
                //s->panx+=DEFAULT_PAN_BIG_STEPS;
                //s->is_pan_changed = true;
                s->gsize++;
                break;
            case 'K':
                s->pany-=DEFAULT_PAN_BIG_STEPS;
                s->is_pan_changed = true;
                break;
            case 'J':
                s->pany+=DEFAULT_PAN_BIG_STEPS;
                s->is_pan_changed = true;
                break;
            case 'R': // autorange
                s->set_autorange = !s->set_autorange;
                s->dmin = -1;
                s->dmax = -1;
                break;
            case 'r': // reset
                s->panx = 0;
                s->pany = 0;
                s->dmin = -1;
                s->dmax = -1;
                break;
            case 'a': // fit all
                s->panx = 0;
                s->pany = 0;
                s->set_autorange = !s->fit_all;
                s->fit_all = !s->fit_all;

                if (!s->fit_all) {
                    s->gsize = DEFAULT_GROUP_SIZE;
                    s->dmin = -1;
                    s->dmax = -1;
                }
                break;
            case KEY_RESIZE:
                s->is_resized = true;
                break;
            case ' ':
                s->is_paused = !s->is_paused;
                break;
            //case KEY_MOUSE:
            //    if (getmouse(&event) == OK) {
            //        if(event.bstate & BUTTON1_CLICKED) {
            //            // This works for left-click
            //            s->clicked_x = event.x;
            //            s->clicked_y = event.y;
            //        }
            //    }
            //    break;
            default:
                return false;
        }

        // flush chars
        while (c != ERR)
            c = getch();

        return true;
    }
    return false;
}

void handle_sigwinch(int sig)
{
}

int8_t update(State* s, Index* index, Plot* pl, Line* l, LineID* lid)
{
    // check if data or exit early
    if (index->npoints == 0)
        return 0;
        
    pthread_mutex_lock(&lock);

    Groups* groups;
    if ((groups = index_get_grouped(index, lid, s->gsize, pl->xsize, s->panx, s->pany)) == NULL) {
        pthread_mutex_unlock(&lock);
        return 0;
    }

    pl->lyaxis->autorange = s->set_autorange;
    pl->ryaxis->autorange = s->set_autorange;

    pl->lyaxis->is_empty = true;
    pl->ryaxis->is_empty = true;

    if (line_set_data(l, groups) < 0) {
        pthread_mutex_unlock(&lock);
        groups_destroy(groups);
        return -1;
    }

    status_set(pl->status, "autorange", "%d",    s->set_autorange);
    status_set(pl->status, "paused",    "%d",    s->is_paused);
    status_set(pl->status, "points",    "%d",    index->npoints);
    status_set(pl->status, "gsize",     "%d",    s->gsize);
    status_set(pl->status, "spread",    "%.3f",  index->spread);
    status_set(pl->status, "panxy",     "%d/%d", s->panx, s->pany);

    plot_draw(pl, groups, s);

    // cleanup
    groups_destroy(groups);
    pthread_mutex_unlock(&lock);
    return 0;
}

void loop(State* s, Index* index)
{
    Plot* pl = plot_init(stdscr);
    Line* l0 = line_init("Right");
    Line* l1 = line_init("Left");
    yaxis_add_line(pl->ryaxis, l0);
    yaxis_add_line(pl->ryaxis, l1);


    while (!s->is_stopped && !sigint_caught) {

        // is also done in plot_draw but this is faster
        if (s->is_resized) {
            s->is_resized = false;
            if (plot_resize(pl) < 0)
                continue;
        }

        if (! s->is_paused && index_has_new_data(index)) {
            if (update(s, index, pl, l0, &lineid0) < 0)
                break;
            //if (update(s, index, pl, l1, &lineid1) < 0)
            //    break;
        }
        // update on user input
        if (non_blocking_sleep(SLEEP_MS, &check_user_input, s)) {
            if (update(s, index, pl, l0, &lineid0) < 0)
                break;
            //if (update(s, index, pl, l1, &lineid1) < 0)
            //    break;
        }
    }
    line_destroy(l0);
    plot_destroy(pl);
}

int main(int argc, char **argv)
{
    // for UTF8 in curses, messes with atof() see: read_stdin()
    setlocale(LC_ALL, "");

    //// catch sigint (CTRL-C)
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = on_sigint;
    sigaction(SIGINT, &action, NULL);

    // init lock
    if (pthread_mutex_init(&lock, NULL) != 0)
        die("\nMutex init failed\n");

    // program state
    State s;
    set_defaults(&s);

    // index holds all data normalized into bins
    Index* index;
    if ((index = index_init(NLINES)) == NULL)
        return 0;


    // start data aggregation thread
    Args largs = {.path="csv/XMRBTC_1m_distance.csv", .index=index, .lock=&lock, .lineid=&lineid0, .is_stopped=false, .idt=0, .iy=5};
    pthread_t lthreadid;
    pthread_create(&lthreadid, NULL, read_file_thread, &largs);

    //sleep(100000);

    //Args args = {.path="csv/btcusd.csv", .index=index, .lock=&lock, .lineid=0, .is_stopped=false, .idt=0, .iopen=1, .ihigh=2, .ilow=3, .iclose=4};
    //Args args = {.index=index, .lock=&lock, .is_stopped=false, .idt=0, .iopen=2, .ihigh=3, .ilow=4, .iclose=5};
    //
    //
    //Args ohlcargs = {.path="csv/XMRBTC_1m_distance.csv", .index=index, .lock=&lock, .lineid=&lineid1, .is_stopped=false, .idt=0, .iopen=2, .ihigh=3, .ilow=4, .iclose=5};
    //pthread_t ohlcthreadid;
    //pthread_create(&ohlcthreadid, NULL, cs_read_file_thread, &ohlcargs);

    //printf(">>>>>>>>>past\n");
    //sleep(1000000);
    //Groups* groups = index_get_grouped(index, &lineid1, s.gsize, 30, 0, 0);
    //groups_print(groups->group);
    //return 1;

    init_ui();
    loop(&s, index);

    largs.is_stopped = true;
    //ohlcargs.is_stopped = true;
    //pthread_join(lthreadid, NULL);
    //pthread_join(ohlcthreadid, NULL);

    index_destroy(index);
    cleanup_ui();
    return 0;
}
