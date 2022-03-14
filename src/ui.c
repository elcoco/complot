#include "ui.h"

WINDOW *ui_window = NULL;

int ui_init()
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

    ui_init_colors();
    curs_set(0);            // don't show cursor
    cbreak();               // don't wait for enter
    noecho();               // don't echo input to screen
    nodelay(stdscr, TRUE);  // don't block
    keypad(stdscr, TRUE);   // Enables keypad mode also necessary for mouse clicks
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL); // Don't mask any mouse events
    printf("\033[?1003h\n"); // Makes the terminal report mouse movement events
    return 1;
}

void ui_cleanup()
{
    printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
    delwin(ui_window);
    endwin();
    //refresh();
}

int add_chr(WINDOW* win, uint32_t y, uint32_t x, uint32_t fgcol, uint32_t bgcol, char c)
{
    set_color(win, fgcol, bgcol);
    mvwaddch(win, y, x, c);
    unset_color(win, fgcol, bgcol);
    return 0;
}

int add_str(WINDOW* win, uint32_t y, uint32_t x, uint32_t fgcol, uint32_t bgcol, char* fmt, ...)
{
    va_list ptr;
    va_start(ptr, fmt);

    set_color(win, fgcol, bgcol);
    mvwprintw(win, y, x, fmt, ptr);
    unset_color(win, fgcol, bgcol);
    va_end(ptr);

    return 0;
}

void ui_init_colors()
{
    use_default_colors();

    if(has_colors()) {
        if(start_color() == OK) {
            for (uint32_t bg=0 ; bg<ncolors ; bg++) {
                for (uint32_t fg=0 ; fg<ncolors ; fg++)
                    init_pair((fg+1)+(bg*ncolors), ccolors[fg], ccolors[bg]);
            }
        } else {
            addstr("Cannot start colours\n");
            refresh();
        }
    } else {
        addstr("Not colour capable\n");
        refresh();
    }
}

void set_color(WINDOW* win, uint32_t fgcolor, uint32_t bgcolor)
{
    wattrset(win, COLOR_PAIR(fgcolor + ((bgcolor-1)*ncolors)));
}

void unset_color(WINDOW* win, uint32_t fgcolor, uint32_t bgcolor)
{
    wattroff(win, COLOR_PAIR(fgcolor + (bgcolor*ncolors)));
}

void clear_win(WINDOW* win)
{
    for (uint32_t x=0; x<getmaxx(win); x++) {
        for (uint32_t y=0; y<getmaxy(win); y++) {
            mvwaddch(win, y, x, ' ');
        }
    }
}

void ui_show_error(WINDOW* win, char* msg)
{
    uint32_t xpos = (getmaxx(win) / 2) - (strlen(msg) / 2);
    uint32_t ypos = getmaxy(win) / 2;
    clear_win(win);
    add_str(win, ypos, xpos, CRED, CDEFAULT, msg);
    wrefresh(win);
}
