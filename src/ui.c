#include "ui.h"

WINDOW *ui_window = NULL;

int init_ui()
{
    // https://stackoverflow.com/questions/61347351/ncurses-stdin-redirection
    // NOTE we have to reopen stdin, piping data into application doesn't play well with ncurses
    //if (!freopen("/dev/tty", "r", stdin)) {
    //    perror("/dev/tty");
    //    exit(1);
    //}
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
    return 1;
}

void cleanup_ui()
{
    printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
    delwin(ui_window);
    endwin();
    //refresh();
}

int add_chr(WINDOW* win, uint32_t y, uint32_t x, uint32_t color, char c)
{
    wattrset(win, COLOR_PAIR(color));
    mvwaddch(win, y, x, c);
    wattroff(win, COLOR_PAIR(color));
    return 0;
}

int add_str(WINDOW* win, uint32_t y, uint32_t x, uint32_t color, char* fmt, ...)
{
    va_list ptr;
    va_start(ptr, fmt);

    wattrset(win, COLOR_PAIR(color));
    mvwprintw(win, y, x, fmt, ptr);
    wattroff(win, COLOR_PAIR(color));
    va_end(ptr);

    return 0;
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

        usleep(UI_CHECK_INTERVAL);
    }
    return false;
}

void clear_win(WINDOW* win)
{
    for (uint32_t x=0; x<getmaxx(win); x++) {
        for (uint32_t y=0; y<getmaxy(win); y++) {
            mvwaddch(win, y, x, ' ');
        }
    }
}

