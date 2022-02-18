#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h> // read
#include <locale.h>     // for utf8 in curses
#include <signal.h>     // catch SIGTERM

#include "index.h"
#include "utils.h"
#include "plot.h"
#include "ui.h"
#include "read_thread.h"

#include "json/json_parser.h"
#include "json/perr.h"

#define NLINES 2

#define SLEEP_MS 1000000

#define DEFAULT_GROUP_SIZE 1
#define DEFAULT_PAN_STEPS 3
#define DEFAULT_PAN_BIG_STEPS 5

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
// TODO on reindex first point is added to linked list again
// TODO create update function in index
// DONE rename component to plot oid
// TODO write better makefile


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
    s->set_autorange = false;
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

void update(State* s, Index* index)
{
    // TODO check if data or exit early
    Groups* groups;
    Plot* pl = pl_init(COLS, LINES);


    //if (s->fit_all)
    //    s->gsize = ceil(index->isize / pl->pxsize);
        
    if ((groups = index_get_grouped(index, LINE1, s->gsize, pl->xsize, s->panx, s->pany)) == NULL) {
        set_status(1, "error");
        return;
    }

    if (s->dmin < 0 || s->set_autorange) {
        s->dmin = groups->gmin;
        s->dmax = groups->gmax;
    }

    pl_draw(pl, index, groups, s);

    show_plot(pl);
    set_status(0, "paused: %d | panx: %d | pany: %d | points: %d | gsize: %d | spread: %f", s->is_paused, s->panx, s->pany, index->npoints, s->gsize, index->spread);
    //set_status(1, "ry: %d[%d.%d]", pl->ryaxis_size, pl->ryaxis_nwhole, pl->ryaxis_nfrac);
    groups_destroy(groups);
}

void loop(State* s, Index* index)
{
    // do initial update
    update(s, index);

    while (!s->is_stopped && !sigint_caught) {

        // update on user input
        if (non_blocking_sleep(SLEEP_MS, &check_user_input, s)) {
            update(s, index);
        }

        if (! s->is_paused) {
            if (index_has_data()) {
                set_status(1, "update!");
            }
            // TODO do reading from stdin here and update screen
            //read_stdin(5, index, 0,2,3,4,5);
            //update(s, index);
            //usleep(1000);
        }
    }
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

    // program state
    State s;
    set_defaults(&s);

    // holds all data and normalizes into bins
    Index* index = index_create(NLINES);

    read_stdin(index, 0,2,3,4,5);

    //int ngroups = 88;
    //Groups* groups;
    //groups = index_get_grouped(index, LINE1, 8, ngroups, 0, 0);
    //groups_print(groups->group);
    //printf("frac:  %d\n", find_nfrac(123.1233000));
    //printf("whole: %d\n", find_nwhole(123.12345));
    //printf("digits: %d\n", count_digits(1234.12));
    //return 0;

    init_ui();                  // setup curses ui

    loop(&s, index);

    cleanup_ui();
    return 0;
}
