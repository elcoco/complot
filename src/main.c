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
#include "json.h"
#include "request.h"
#include "plotwin.h"

#define NLINES 5

#define SLEEP_MS 100*1000


// thread lock
pthread_mutex_t lock;

int sigint_caught = 0;


PlotWin pw0;
PlotWin pw1;

void on_sigint(int signum)
{
    sigint_caught = 1;
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
            case '0' ... '9':
                s->lines_enabled[c-'0'] = !s->lines_enabled[c-'0'];
                s->line_changed = true;
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

void print_usage()
{
    printf("CGOL :: Game of life written in C!\n");
    //printf("\nMandatory arguments:\n");
    printf("\nOptional arguments:\n");
    printf("    -f SEED_FILE  Set seed file\n");
    printf("    -s SPEED_MS   Transition speed, Default=1000\n");
    printf("    -S            Set shading\n");
    printf("    -r            Set random seed\n");
    printf("    -w            Set wrapping edges\n");
}

bool parse_args(State* s, int argc, char** argv)
{
    int option;

    while((option = getopt(argc, argv, "s:")) != -1){ //get option from the getopt() method
        switch (option) {
            case 's':
                strcpy(s->symbol, optarg);
                break;
            case ':': 
                printf("option needs a value\n"); 
                return false;
            case '?': 
                print_usage();
                return false;
       }
    }
    if (argc == 1) {
        print_usage();
        return false;
    }
    return true;
}

int8_t update(PlotWin* pw)
{
    // check if data or exit early
    if (pw->index->npoints == 0)
        return 0;
        
    Plot* pl = pw->plot;
    Line* l0 = pw->lines[0];
    Line* l1 = pw->lines[1];
    Index* index = pw->index;
    State* s = pw->state;

    pthread_mutex_lock(&lock);

    // set the changed enabled state
    if (s->line_changed) {
        for (int i=0 ; i<index->nlines ; i++) {
            if (pw->lines[i] != NULL)
                pw->lines[i]->is_enabled = s->lines_enabled[i];
        }
        s->line_changed = false;
    }

    Groups* groups;
    if ((groups = index_get_grouped(index, s->gsize, pl->xsize, s->panx, s->pany)) == NULL) {
        pthread_mutex_unlock(&lock);
        return 0;
    }

    pl->lyaxis->autorange = s->set_autorange;
    pl->ryaxis->autorange = s->set_autorange;
    pl->lyaxis->is_empty = true;
    pl->ryaxis->is_empty = true;

    if (line_set_data(l0, groups->lines[l0->lineid->id]) < 0) {
        pthread_mutex_unlock(&lock);
        groups_destroy(groups);
        debug("Failed to set data for line 0\n");
        return -1;
    }
    if (line_set_data(l1, groups->lines[l1->lineid->id]) < 0) {
        pthread_mutex_unlock(&lock);
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

    plot_draw(pl, s);

    // cleanup
    groups_destroy(groups);
    pthread_mutex_unlock(&lock);
    return 0;
}

void loop(PlotWin** pws, uint32_t length)
{
    PlotWin* pw = *pws;
    bool is_stopped = false;

    while (!sigint_caught) {
        for (int i=0 ; i<length ; i++) {
            PlotWin* pw = pws[i];

            if (pw->state->is_stopped)
                return;

            // is also done in plot_draw but this is faster
            if (pw->state->is_resized) {
                pw->state->is_resized = false;
                if (plot_resize(pw->plot) < 0)
                    continue;
            }

            if (! pw->state->is_paused && index_has_new_data(pw->index)) {
                if (update(pw) < 0)
                    break;
            }
            // update on user input
            if (non_blocking_sleep(SLEEP_MS, &check_user_input, pw->state)) {
                if (update(pw) < 0)
                    break;
            }

        }
    }

    //while (!pw->state->is_stopped && !sigint_caught) {


    //    // is also done in plot_draw but this is faster
    //    if (pw->state->is_resized) {
    //        pw->state->is_resized = false;
    //        if (plot_resize(pw->plot) < 0)
    //            continue;
    //    }

    //    if (! pw->state->is_paused && index_has_new_data(pw->index)) {
    //        if (update(pw) < 0)
    //            break;
    //    }
    //    // update on user input
    //    if (non_blocking_sleep(SLEEP_MS, &check_user_input, pw->state)) {
    //        if (update(pw) < 0)
    //            break;
    //    }
    //}
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

    //if (!parse_args(&s, argc, argv))
    //    return 1;

    init_ui();

    // index holds all data normalized into bins
    Index* index;
    if ((index = index_init(NLINES)) == NULL)
        return 0;

    WINDOW* sw0 = newwin(LINES/2, 0, 0, 0);
    WINDOW* sw1 = newwin(0, 0, LINES/2, 0);
    //WINDOW* sw1 = newwin(getmaxy(stdscr)/2, getmaxx(stdscr), getmaxy(stdscr)/2, 0);

    fill_win(sw1, '#');
    //wrefresh(sw0);
    //usleep(10000000000);
    refresh();
    wrefresh(sw1);

    PlotWin* pw0 = pw_init(sw0, index, "BTCBUSD", &lock);
    PlotWin* pw1 = pw_init(sw1, index, "ADABUSD", &lock);
    PlotWin* pws[] = {pw0, pw1};

    loop(pws, 2);

    pw0->is_stopped = true;
    pw1->is_stopped = true;

    pthread_join(pw0->threadid, NULL);
    pthread_join(pw1->threadid, NULL);

    plot_destroy(pw0->plot);
    plot_destroy(pw1->plot);

    index_destroy(index);

    cleanup_ui();
    return 0;
}
