#include <curses.h>
#include <stdbool.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include <string.h>

#define B_TL "┌"
#define B_TR "┐"
#define B_BL "└"
#define B_BR "┘"
#define B_H  '-'
#define B_V  '|'

WINDOW* pwin;
WINDOW* lawin;
WINDOW* rawin;
WINDOW* cswin;
WINDOW* swin;

void draw_border(WINDOW* w, unsigned int lines, unsigned int cols)
{
    mvwhline(w, 0, 1, B_H, cols-2);
    mvwhline(w, lines-1, 1, B_H, cols-2);
    mvwvline(w, 1, 0, B_V, lines-2);
    mvwvline(w, 1, cols-1, B_V, lines-2);

    mvwaddstr(w, 0, 0, B_TL);
    mvwaddstr(w, 0, cols-1, B_TR);
    mvwaddstr(w, lines-1, 0, B_BL);
    mvwaddstr(w, lines-1, cols-1, B_BR);
}

void fill_win(WINDOW* w, unsigned int lines, unsigned int cols, char c)
{
    for (int y=0 ; y<lines ; y++) {
        for (int x=0 ; x<cols ; x++) {
            mvwaddch(w, y, x, c);
        }
    }
}

int draw()
{
    int laxis_xsize = 10;
    int raxis_xsize = 10;
    int s_ysize = 3;

    // don't do anything if screen is too small to hold data
    if (COLS - laxis_xsize - raxis_xsize <= 10)
        return -1;

    // main plot window
    pwin = newwin(LINES-s_ysize, COLS, 0, 0);
    //fill_win(pwin, LINES-s_ysize, COLS, 'x');
    //draw_border(pwin, LINES-s_ysize, COLS);

    // left axis
    lawin = subwin(pwin, getmaxy(pwin), laxis_xsize, 0, 0);
    fill_win(lawin, getmaxy(pwin), laxis_xsize, 'O');
    //draw_border(lawin, getmaxy(pwin), laxis_xsize);
    //
    //lspacer = subwin(pwin, getmaxy(pwin), 1, 0, 0);

    // right axis
    rawin = subwin(pwin, getmaxy(pwin), raxis_xsize, 0, getmaxx(pwin)-raxis_xsize);
    fill_win(rawin, getmaxy(pwin), raxis_xsize, 'O');
    //draw_border(rawin, getmaxy(pwin), raxis_xsize);

    // candlestick area
    int cswin_xsize = getmaxx(pwin) - getmaxx(lawin) - getmaxx(rawin);
    cswin = subwin(pwin, getmaxy(pwin), cswin_xsize, 0, laxis_xsize);
    fill_win(cswin, getmaxy(pwin), cswin_xsize, 'P');
    draw_border(cswin, getmaxy(pwin), cswin_xsize);

    // status bar
    swin = newwin(s_ysize, COLS, LINES-s_ysize, 0);
    fill_win(swin, s_ysize, COLS, 'S');
    draw_border(swin, s_ysize, COLS);


    refresh();
    wrefresh(pwin);
    wrefresh(lawin);
    wrefresh(rawin);
    wrefresh(swin);
    return 0;
}

void handle_winch(int sig)
{
    // handle SIGWINCH window resize signal
    endwin();
    refresh();
    clear();
    draw();
}

int main()
{
    setlocale(LC_ALL, "");
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);

    initscr();
    curs_set(0);            // don't show cursor
    cbreak();               // don't wait for enter
    noecho();               // don't echo input to screen

    draw();
    
    while (1) {
        char c = getch();
        if (c == 'q')
            break;
        else if (c == KEY_RESIZE) {
            mvwaddstr(swin, 0, 0, "bevers zijn awesome");
            wrefresh(swin);
        }
    }
    
    delwin(pwin);
    delwin(lawin);
    delwin(rawin);
    delwin(swin);
    endwin();
}
 
