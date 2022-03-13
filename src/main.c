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


#define SLEEP_MS 100*1000

// thread lock
pthread_mutex_t lock;

int sigint_caught = 0;

void on_sigint(int signum)
{
    sigint_caught = 1;
}

bool check_user_input(void* arg)
{
    // s struct is passed as an argument to a callback, cast it to the proper type
    State* s = arg;
    PlotWin* pw;
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
                break;
            case 'L':
                s->gsize++;
                break;
            case 'K':
                s->cur_pw = (s->cur_pw == 0) ? s->pws_length-1 : s->cur_pw-1;
                debug("cur pw up: %d\n", s->cur_pw);
                break;
            case 'J':
                s->cur_pw = (s->cur_pw == s->pws_length-1) ? 0 : s->cur_pw+1;
                debug("cur pw down: %d\n", s->cur_pw);
                break;
            case 's': // autorange
                pw = s->pws[s->cur_pw];
                pw->plot->show_status = !pw->plot->show_status;
                plot_resize(pw->plot);
                break;
            case 'R': // autorange
                s->set_autorange = !s->set_autorange;
                break;
            case 'r': // reset
                s->panx = 0;
                s->pany = 0;
                break;
            case KEY_RESIZE:
                s->is_resized = true;
                break;
            case ' ':
                s->is_paused = !s->is_paused;
                break;
            case '0' ... '9':
                pw = s->pws[s->cur_pw];
                if (pw->lines[c-'0'])
                    pw->lines[c-'0']->is_enabled = !pw->lines[c-'0']->is_enabled;
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

void loop(State* s, Index* index)
{
    while (!s->is_stopped && !sigint_caught) {

        // triggered by KEY_RESIZE
        if (s->is_resized) {
            s->is_resized = false;
            if (pw_update_all(s->pws, s->pws_length, &lock))
                break;
        }

        // update on new data
        if (! s->is_paused && index_has_new_data(index)) {
            if (pw_update_all(s->pws, s->pws_length, &lock))
                break;
        }

        // sleep but update on user input
        if (non_blocking_sleep(SLEEP_MS, &check_user_input, s)) {
            if (pw_update_all(s->pws, s->pws_length, &lock))
                break;
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

    // init lock
    if (pthread_mutex_init(&lock, NULL) != 0)
        die("\nMutex init failed\n");

    //if (!parse_args(&s, argc, argv))
    //    return 1;

    init_ui();

    // index holds all data normalized into bins
    Index* index;
    if ((index = index_init(MAX_LINES)) == NULL)
        return 0;

    // state shared between plot windows
    // this records things like panning and autoranging etc...
    State* s = state_init();

    WINDOW* sw0 = newwin(0, 0, 0, 0);
    WINDOW* sw1 = newwin(0, 0, 0, 0);
    WINDOW* sw2 = newwin(0, 0, 0, 0);

    PlotWin* pw0 = pw_init(sw0, index, s, "BTCBUSD", &lock);
    PlotWin* pw1 = pw_init(sw1, index, s, "XMRBUSD", &lock);
    PlotWin* pw2 = pw_init(sw2, index, s, "ADABUSD", &lock);

    state_add_pw(s, pw0);
    state_add_pw(s, pw1);
    state_add_pw(s, pw2);

    loop(s, index);

    s->is_stopped = true;

    pthread_join(pw0->threadid, NULL);
    pthread_join(pw1->threadid, NULL);
    pthread_join(pw2->threadid, NULL);

    plot_destroy(pw0->plot);
    plot_destroy(pw1->plot);
    plot_destroy(pw2->plot);

    index_destroy(index);

    cleanup_ui();
    return 0;
}
