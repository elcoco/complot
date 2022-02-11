#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h> // read
#include <locale.h>     // for utf8 in curses
#include <signal.h>     // catch SIGTERM

#include "index.h"
#include "utils.h"
#include "matrix.h"
#include "ui.h"

#define INDEX_GROW_AMOUNT 1000
#define INDEX_SPREAD 8
#define NLINES 2

#define GROUP_SIZE 2
#define GROUPS 70 // = xsize
#define YSIZE 60

#define SLEEP_MS 1000000

#define DEFAULT_PAN_STEPS 5
#define DEFAULT_PAN_BIG_STEPS 20

// TODO extend index when out of bounds
// DONE get groups x amount of groups from last known data
// DONE create points linked list
// TODO create CSV file reader
// DONE add xwin dimension to group
// TODO draw candlesticks
// TODO return groups as linked list
// TODO keep track of data min and max. we need this info when drawing candlesticks
// DONE segfault when GROUP_SIZE = 1 or 2
// TODO now index data limits are used, but group data limits allows for auto scale

#define BUF_SIZE 1000
#define DELIM_STR ","
#define STDIN 0

uint32_t counter = 0;

enum lines {
    LINE1,
    LINE2
} lines;

int sigint_caught = 0;


void on_sigint(int signum)
{
    sigint_caught = 1;
}
bool fast_forward(char** c, char* search_lst, char* expected_lst, char* ignore_lst, char* buf)
{
    /* fast forward until a char from search_lst is found
     * Save all chars in buf until a char from search_lst is found
     * Only save in buf when a char is found in expected_lst
     *
     * If buf == NULL,          don't save chars
     * If expected_lst == NULL, allow all characters
     */

    // save skipped chars that are on expected_lst in buffer
    char* ptr = buf;

    // don't return these chars with buffer
    ignore_lst = (ignore_lst) ? ignore_lst : "";

    // exit at EOL
    if (strlen(*c) == 0)
        return false;

    while (!strchr(search_lst, **c)) {

        if (buf != NULL) {
            if (!strchr(ignore_lst, **c) && expected_lst == NULL)
                *ptr++ = **c;
            else if (!strchr(ignore_lst, **c) && strchr(expected_lst, **c))
                *ptr++ = **c;
            else
                return false;
        }
        (*c)++;
    }

    // terminate string
    if (ptr != NULL)
        *ptr = '\0';

    return true;
}

void die(char* msg)
{
    printf("ERROR: %s\n", msg);
    exit(0);
}

//int read_stdin3(char* fmt)
int read_stdin(Index* index, uint8_t idt, uint8_t iopen, uint8_t ihigh, uint8_t ilow, uint8_t iclose)
{
    char buf[BUF_SIZE] = {'\0'};
    char tmpbuf[BUF_SIZE] = {'\0'};

    // x incrementer, must be a converted datetime but for now this will do
    uint32_t ix = 0;

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (ix == 0) {
            ix++;
            continue;
        }

        if (buf[strlen(buf)-1] == '\n') {
            char* spos = buf;
            int c = 0;
            double open, high, low, close;

            while (fast_forward(&spos, ",\n", NULL, NULL, tmpbuf)) {

                // Reset local, using UTF-8 makes atof() wait for ',' instead of '.'
                // as a float delimiter
                // So to reset the previous: setlocale(LC_ALL, "");
                setlocale(LC_NUMERIC,"C");

                // NOTE atof failes on unicode!!!!
                if (c == iopen)
                    open = atof(tmpbuf);
                else if (c == ihigh)
                    high = atof(tmpbuf);
                else if (c == ilow)
                    low = atof(tmpbuf);
                else if (c == iclose)
                    close = atof(tmpbuf);

                spos++;
                c++;
                tmpbuf[0] = '\0';
            }

            Point* p = point_create(ix, open, high, low, close);
            if (index_insert(index, LINE1, p) < 0)
                return -1;

        } else {
            printf("too long!!! %d: %s\n", strlen(buf), buf);
        }

        ix++;
    }
}

void set_defaults(State* s)
{
    s->is_paused = false;
    s->is_stopped = false;
    s->panx = 0;
    s->pany = 0;
    s->is_pan_changed = false;
}

bool check_user_input(void* arg)
{
    // s struct is passed as an argument to a callback, cast it to the proper type
    State* s = arg;
    MEVENT event;       // curses mouse event
                        //
    /* check for user input, return 1 if there was input */
    int c = getch();

    counter++;

    if (c != ERR) {
        mvprintw(3,1, "got input!!! %d<<", c);

        switch (c) {
            case 'q':
                s->is_stopped = true;
                mvprintw(2,1, "stopped!!! %d", s->is_stopped);
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
                s->panx-=DEFAULT_PAN_BIG_STEPS;
                s->is_pan_changed = true;
                break;
            case 'L':
                s->panx+=DEFAULT_PAN_BIG_STEPS;
                s->is_pan_changed = true;
                break;
            case 'K':
                s->pany-=DEFAULT_PAN_BIG_STEPS;
                s->is_pan_changed = true;
                break;
            case 'J':
                s->pany+=DEFAULT_PAN_BIG_STEPS;
                s->is_pan_changed = true;
                break;
            case ' ':
                s->is_paused = !s->is_paused;
                break;
            case KEY_MOUSE:
                if (getmouse(&event) == OK) {
                    if(event.bstate & BUTTON1_CLICKED) {
                        // This works for left-click
                        s->clicked_x = event.x;
                        s->clicked_y = event.y;
                    }
                }
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

void loop(State* s, Index* index)
{

    while (!s->is_stopped && !sigint_caught) {
        Groups* groups = index_get_grouped(index, LINE1, GROUP_SIZE, COLS, s->panx, s->pany);
        //vp_clear_cells(vp);
        ViewPort* vp = vp_init(COLS, LINES); // x, y
        vp_draw_candlesticks(vp, groups);
        show_matrix(vp);

        non_blocking_sleep(SLEEP_MS, &check_user_input, s);
    }

}

int main(int argc, char **argv)
{
    // for UTF8 in curses
    setlocale(LC_ALL, "");

    //// catch sigint (CTRL-C)
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = on_sigint;
    sigaction(SIGINT, &action, NULL);

    // program state
    State s;
    set_defaults(&s);

    Index* index = index_create(INDEX_GROW_AMOUNT, INDEX_SPREAD, NLINES);

    read_stdin(index, 0,2,3,4,5);

    init_ui();                  // setup curses ui

    //Groups* groups = index_get_grouped(index, LINE1, GROUP_SIZE, COLS);


    //index_print(index);
    //groups_print(groups->group);
    //printf("INDEX dimensions: %f %f\n", index->dmin, index->dmax);
    //printf("GROUP dimensions: %f %f\n", groups->dmin, groups->dmax);

    //vp_draw_candlesticks(vp, groups);

    //vp_print(vp);

    loop(&s, index);

    cleanup_ui();
    return 0;
}
