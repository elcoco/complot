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

    // set ESCDELAY so we don't have to wait a seconds after pressing escape key
    setenv("ESCDELAY", "25", 1); 
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

int add_str_color(WINDOW* win, uint32_t y, uint32_t x, uint32_t fgcol, uint32_t bgcol, char* fmt, ...)
{

    /* Maintain current display representation with colors so we can replace background color */
    va_list ptr;
    va_start(ptr, fmt);

    char str[100] = {'\0'};
    vsprintf(str, fmt, ptr);

    // TODO if cell is occupied in matrix, replace background color with color in matrix
    add_str(win, y, x, fgcol, bgcol, fmt, ptr);

    va_end(ptr);

    // TODO save color in color matrix if string is a block
    if (strcmp(str, UI_BLOCK) == 0)
        debug("save in matrix\n");

    return 0;
}

void ui_refresh(WINDOW* win)
{
    if (win)
        wrefresh(win);
    else
        refresh();
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

void ui_erase(WINDOW* win)
{
    /* Erase window wrapper */

    // TODO reset color matrix here

    if (win)
        werase(win);
    else
        erase();
}

void ui_show_error(WINDOW* win, char* msg)
{
    uint32_t xpos = (getmaxx(win) / 2) - (strlen(msg) / 2);
    uint32_t ypos = getmaxy(win) / 2;
    ui_erase(win);
    add_str(win, ypos, xpos, CRED, CDEFAULT, msg);
    wrefresh(win);
}
