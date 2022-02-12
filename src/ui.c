#include "ui.h"

WINDOW *ui_window = NULL;

int init_ui()
{
    // https://stackoverflow.com/questions/61347351/ncurses-stdin-redirection
    // NOTE we have to reopen stdin, piping data into application doesn't play well with ncurses
    if (!freopen("/dev/tty", "r", stdin)) {
        perror("/dev/tty");
        exit(1);
    }
    ui_window = initscr();


    if (ui_window == NULL)
        return 0;

    init_colors();
    curs_set(0);            // don't show cursor
    cbreak();               // don't wait for enter
    noecho();               // don't echo input to screen
    nodelay(stdscr, TRUE);  // don't block
    keypad(stdscr, TRUE);   // Enables keypad mode also necessary for mouse clicks
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL); // Don't mask any mouse events
    printf("\033[?1003h\n"); // Makes the terminal report mouse movement events
}

void cleanup_ui()
{
    printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
    delwin(ui_window);
    endwin();
    refresh();
}

int get_color(int age)
{
    int col = (age / 10) +1;
    if (col > 8)
        col = 8;
    else if (col == 0)
        col = 1;
    return col;
}

void show_matrix(ViewPort* vp)
{
    /* Put data from viewport into curses matrix */
    Cell*** cells = vp->cells;

    for (int y=0 ; y<vp->ysize ; y++) {
        for (int x=0 ; x<vp->xsize ; x++) {
            Cell* c = cells[x][y];


            attrset(COLOR_PAIR(c->fgcol));

            // curses starts top-left
            mvaddstr(vp->ysize-1-y, x, c->chr);

            attroff(COLOR_PAIR(c->fgcol));
        }
    }
    refresh();
    return;
}

int set_status(uint32_t lineno, char* fmt, ...)
{
    char buf[1000];
    va_list ptr;
    int ret;

    va_start(ptr, fmt);
    ret = vsprintf(buf, fmt, ptr);
    va_end(ptr);

    move(LINES-(1+lineno), 0);
    clrtoeol();

    attrset(COLOR_PAIR(CRED));
    mvaddstr(LINES-(1+lineno), 0, buf);
    attroff(COLOR_PAIR(CRED));

    return(ret);
}

void init_colors()
{
    use_default_colors();

    if(has_colors()) {
        if(start_color() == OK) {
            init_pair(CRED,     COLOR_RED,     -1);
            init_pair(CGREEN,   COLOR_GREEN,   -1);
            init_pair(CYELLOW,  COLOR_YELLOW,  -1);
            init_pair(CBLUE,    COLOR_BLUE,    -1);
            init_pair(CMAGENTA, COLOR_MAGENTA, -1);
            init_pair(CCYAN,    COLOR_CYAN,    -1);
            init_pair(CWHITE,   COLOR_WHITE,   -1);
            init_pair(CBLACK,   COLOR_BLACK,   -1);

        } else {
            addstr("Cannot start colours\n");
            refresh();
        }
    } else {
        addstr("Not colour capable\n");
        refresh();
    }
} 

bool non_blocking_sleep(int interval, bool(*callback)(void* arg), void* arg)
{
    /* Do a non blocking sleep that checks for user input */
    struct timeval t_start, t_end;
    gettimeofday(&t_start, NULL);

    while (1) {
        gettimeofday(&t_end, NULL);
        if ((t_end.tv_sec*1000000 + t_end.tv_usec) - (t_start.tv_sec*1000000 + t_start.tv_usec) >= interval)
            break;

        if (callback(arg))
            return true;

        usleep(CHECK_INTERVAL);
    }
    return false;
}

