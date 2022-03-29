
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
#include "curses_menu.h"

// TODO create searchable log window
// TODO create more awesome line drawing algorithm

#define SLEEP_MS 100*1000

// thread lock
pthread_mutex_t lock;

int sigint_caught = 0;

void on_sigint(int signum)
{
    sigint_caught = 1;
}

void print_usage()
{
    printf("COMPLOT :: Crypto candlestick plots in your terminal!\n");
    //printf("\nMandatory arguments:\n");
    printf("\nOptional arguments:\n");
    printf("    -s SYMBOL     Create plot for this symbol\n");
    printf("    -h            Help!\n");
}

bool parse_args(State* s, int argc, char** argv)
{
    int option;

    while((option = getopt(argc, argv, "s:h")) != -1){ //get option from the getopt() method
        switch (option) {
            case 's':
                debug("Creating plot for symbol: %s\n", optarg);
                WINDOW* win = newwin(0, 0, 0, 0);
                PlotWin* pw = pw_init(win, s, optarg, &lock);
                state_add_pw(s, pw);
                pw_update_all(s->pws, s->pws_length, &lock, true);
                break;
            case ':': 
                printf("option needs a value\n"); 
                return false;
            case 'h': 
                print_usage();
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
    if (s->pws_length == 0) {
        printf("No symbol selected\n");
        return false;
    }
    return true;
}

bool check_user_input(void* arg)
{
    // s struct is passed as an argument to a callback, cast it to the proper type
    State* s = arg;

    // current selected PlotWin
    PlotWin* pw = s->pws[s->cur_pw];
                        //
    /* check for user input, return 1 if there was input */
    int c = getch();

    if (c != ERR) {
        switch (c) {
            case 'q':
                s->is_stopped = true;
                break;
            case 'h':
                pw->panx-=DEFAULT_PAN_STEPS;
                pw->is_pan_changed = true;
                //pw->is_paused = true;
                break;
            case 'l':
                pw->panx+=DEFAULT_PAN_STEPS;
                pw->is_pan_changed = true;
                //pw->is_paused = true;
                break;
            case 'k':
                pw->pany-=DEFAULT_PAN_STEPS;
                pw->is_pan_changed = true;
                pw->set_autorange = false;
                break;
            case 'j':
                pw->pany+=DEFAULT_PAN_STEPS;
                pw->is_pan_changed = true;
                pw->set_autorange = false;
                break;
            case 'H':
                if (pw->gsize > 1)
                    pw->gsize--;
                break;
            case 'L':
                pw->gsize++;
                break;
            case 'K':
                s->cur_pw = (s->cur_pw == 0) ? s->pws_length-1 : s->cur_pw-1;
                debug("cur pw up: %d\n", s->cur_pw);
                break;
            case 'J':
                s->cur_pw = (s->cur_pw == s->pws_length-1) ? 0 : s->cur_pw+1;
                debug("cur pw down: %d\n", s->cur_pw);
                break;
            case 'x':
                menu_select_symbol();
                break;
            case 'i':
                pw_select_interval(pw, (const char**)binance_interval_map);
                break;
            case 's': // autorange
                pw->plot->show_status = !pw->plot->show_status;
                plot_resize(pw->plot);
                break;
            case 'd':
                if (s->pws_length == 1) {
                    debug("Not removing last window\n");
                    break;
                }
                // delete plot window
                state_remove_pw(s, pw);
                break;
            case 'n':
                s->do_create_pw = true;
                break;
            case 'R': // autorange
                pw->set_autorange = !pw->set_autorange;
                break;
            case 'r': // reset
                pw->panx = 0;
                pw->pany = 0;
                break;
            case KEY_RESIZE:
                s->is_resized = true;
                break;
            case ' ':
                pw->is_paused = !pw->is_paused;
                break;
            case '0' ... '9':
                if (pw->lines[c-'0'])
                    pw->lines[c-'0']->is_enabled = !pw->lines[c-'0']->is_enabled;
                break;
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


void loop(State* s)
{
    /* Do plot updating and handle events from check_user_input */
    while (!s->is_stopped && !sigint_caught && s->pws_length > 0) {
        if (s->do_create_pw) {
            s->do_create_pw = false;

            char* symbol = menu_select_symbol();
            if (symbol == NULL)
                continue;

            WINDOW* win = newwin(0, 0, 0, 0);
            PlotWin* pw = pw_init(win, s, symbol, &lock);
            state_add_pw(s, pw);
            if (pw_update_all(s->pws, s->pws_length, &lock, true))
                break;
            free(symbol);
        }

        // triggered by KEY_RESIZE
        if (s->is_resized) {
            s->is_resized = false;
            state_resize_pws(s->pws, s->pws_length);
            if (pw_update_all(s->pws, s->pws_length, &lock, true))
                break;
        }

        // update on new data and !paused state
        if (pw_update_all(s->pws, s->pws_length, &lock, false))
            break;

        // sleep but update on user input
        if (non_blocking_sleep(SLEEP_MS, &check_user_input, s)) {
            if (pw_update_all(s->pws, s->pws_length, &lock, true))
                break;
        }
    }
}

void run_test()
{
    debug("\nStart test -----------------\n");
    add_str_color(stdscr, 10, 10, CRED, CBLACK, "12345678910");
    add_str_color(stdscr, 10, 10, CBLUE, CDEFAULT, "123456789");
    add_str_color(stdscr, 10, 10, CMAGENTA, CGREEN, "12345678");
    add_str_color(stdscr, 10, 10, CWHITE, CGREEN, "123456");
    add_str_color(stdscr, 10, 10, CBLACK, CGREEN, "12345");
    add_str_color(stdscr, 10, 10, CCYAN, CGREEN, "1234");
    add_str_color(stdscr, 10, 10, CGREEN, CGREEN, "123");
    add_str_color(stdscr, 10, 10, CYELLOW, CGREEN, "12");
    refresh();

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

    ui_init();

    //run_test();
    //usleep(1000000000);
    //goto cleanup;
    

    // state shared between plot windows
    // this records things like panning and autoranging etc...
    State* s = state_init();

    if (parse_args(s, argc, argv))
        loop(s);


    state_destroy(s);

    ui_cleanup();
    return 0;
}
