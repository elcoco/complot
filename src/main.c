#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h> // read
#include <locale.h>     // for utf8 in curses
#include <signal.h>     // catch SIGTERM
#include <pthread.h>

#include "index.h"
#include "utils.h"
#include "plot.h"
#include "ui.h"
#include "read_thread.h"

#include "json/json_parser.h"
#include "json/perr.h"

#define NLINES 2

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
// TODO write better makefile
// DONE x should also be double
// TODO plot should have axis and line structs to organize what should be drawn where
// TODO rename pl (plot) to p and p (point) to pnt
// DONE rename pl_ function names to plot_
// TODO don't recreate plot on every iteration, not pretty


int sigint_caught = 0;


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
                break;
            case 'l':
                s->panx+=DEFAULT_PAN_STEPS;
                s->is_pan_changed = true;
                break;
            case 'k':
                s->pany-=DEFAULT_PAN_STEPS;
                s->is_pan_changed = true;
                break;
            case 'j':
                s->pany+=DEFAULT_PAN_STEPS;
                s->is_pan_changed = true;
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

void update(State* s, Index* index, Plot* pl)
{
    // check if data or exit early
    if (index->npoints == 0) {
        set_status(0, "No data...");
        refresh();
        return;
    }
        
    Groups* groups;
    pthread_mutex_lock(&lock);

    //if (s->fit_all)
    //    s->gsize = ceil(index->isize / pl->pxsize);

    // TODO we get more groups than we need so we need to skip a bunch later
    //      this is super annoying and should be fixed.
    //      The reason this is necessary is because when plot dimensions
    //      are decided there is no information about y axis width.
    //      Therefore we take the same amount of groups as there are columns
    //      in the terminal.
    if ((groups = index_get_grouped(index, l1->lineid, s->gsize, pl->xsize, s->panx, s->pany)) == NULL) {
        set_status(1, "error");
        refresh();
        plot_destroy(pl);
        pthread_mutex_unlock(&lock);
        return;
    }

    axis_set_data(pl->raxis, groups);

    if (s->dmin < 0 || s->set_autorange) {
        s->dmin = groups->gmin;
        s->dmax = groups->gmax;
    }
    pl->raxis->dmin = s->dmin;
    pl->raxis->dmax = s->dmax;

    plot_draw(pl, groups, s);

    ui_show_plot(pl);
    set_status(0, "paused: %d | panx: %d | pany: %d | points: %d | gsize: %d | spread: %f", s->is_paused, s->panx, s->pany, index->npoints, s->gsize, index->spread);

    // cleanup
    groups_destroy(groups);
    pthread_mutex_unlock(&lock);
}

void loop(State* s, Index* index)
{
    Line* l1 = line_init("First line");
    Plot* pl = plot_init(COLS, LINES);
    axis_add_line(pl->raxis, l1);

    while (!s->is_stopped && !sigint_caught) {

        if (! s->is_paused) {
            if (index_has_new_data(index))
                update(s, index);
        }
        // update on user input
        if (non_blocking_sleep(SLEEP_MS, &check_user_input, s)) {
            update(s, index);
        }

        //return;

    }
    line_destroy(l1);
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
    Args args = {.index=index, .lock=&lock, .is_stopped=false, .idt=0, .iopen=1, .ihigh=2, .ilow=3, .iclose=4};
    //Args args = {.index=index, .lock=&lock, .is_stopped=false, .idt=0, .iopen=2, .ihigh=3, .ilow=4, .iclose=5};
    pthread_t threadid;
    pthread_create(&threadid, NULL, read_file_thread, &args);

    //usleep(1000000);
    //points_print(*(index->phead));
    //Plot* pl = plot_init(COLS, LINES);
    //Groups* groups = index_get_grouped(index, LINE1, 1, 50, 0, 0);
    //groups_print(groups->group);
    //points_print(*(index->phead));


    init_ui();

    loop(&s, index);

    args.is_stopped = true;
    pthread_join(threadid, NULL);
    index_destroy(index);
    cleanup_ui();
    return 0;
}
