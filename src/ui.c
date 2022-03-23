#include "ui.h"

WINDOW *ui_window = NULL;

// keep track of chars and colors of cells on screen
// so we can draw characters on top of other characters
static Matrix* matrix;

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

    matrix = ui_matrix_init(COLS, LINES);
    //debug("cell: %s, %d, %d\n", c->chr, c->fgcol, c->bgcol);

    return 1;
}

void ui_cleanup()
{
    printf("\033[?1003l\n"); // Disable mouse movement events, as l = low
    delwin(ui_window);
    endwin();
}

Matrix* ui_matrix_init(int32_t xsize, int32_t ysize)
{
    Matrix* m = malloc(sizeof(Matrix));
    m->xsize = xsize;
    m->ysize = ysize;
    m->is_erased = false;

    m->cells = malloc((ysize*xsize) * sizeof(Cell*));
    Cell** pcell = m->cells;

    for (int32_t i=0 ; i<(ysize*xsize) ; i++, pcell++) {
        *pcell = malloc(sizeof(Cell));
        (*pcell)->chr[0] = '\0';
        (*pcell)->fgcol = -1;
        (*pcell)->bgcol = -1;
    }
    // end with null for easy itering
    //*(++pcell) = NULL;
    return m;
}

void ui_matrix_destroy(Matrix* m)
{
    for (int32_t i=0 ; i<(m->ysize*m->xsize) ; i++) {
        free(m->cells[i]);
    }
    free(m->cells);
    free(m);
}

Cell* ui_matrix_get(Matrix* m, int32_t y, int32_t x)
{
    return *(m->cells + ((y*m->xsize) + x));
}

void ui_matrix_set(Matrix* m, int32_t y, int32_t x, char* chr, int32_t fgcol, int32_t bgcol)
{
    Cell* cell = ui_matrix_get(m, y, x);
    strcpy(cell->chr, chr);
    cell->fgcol = fgcol;
    cell->bgcol = bgcol;
}

int add_chr(WINDOW* win, int32_t y, int32_t x, int32_t fgcol, int32_t bgcol, char c)
{
    set_color(win, fgcol, bgcol);
    mvwaddch(win, y, x, c);
    unset_color(win, fgcol, bgcol);
    return 0;
}

int add_str(WINDOW* win, int32_t y, int32_t x, int32_t fgcol, int32_t bgcol, char* fmt, ...)
{
    assert(x>=0);
    assert(y>=0);

    va_list ptr;
    va_start(ptr, fmt);

    set_color(win, fgcol, bgcol);
    mvwprintw(win, y, x, fmt, ptr);
    unset_color(win, fgcol, bgcol);
    va_end(ptr);


    return 0;
}

void get_color(char buf[10], int32_t color)
{
    switch (color) {
        case -1:
            strcpy(buf, "default");
            break;
        case COLOR_RED:
            strcpy(buf, "red");
            break;
        case COLOR_GREEN:
            strcpy(buf, "green");
            break;
        case COLOR_YELLOW:
            strcpy(buf, "yellow");
            break;
        case COLOR_BLUE:
            strcpy(buf, "blue");
            break;
        case COLOR_MAGENTA:
            strcpy(buf, "magenta");
            break;
        case COLOR_CYAN:
            strcpy(buf, "cyan");
            break;
        case COLOR_WHITE:
            strcpy(buf, "white");
            break;
        case COLOR_BLACK:
            strcpy(buf, "black");
            break;
        default:
            strcpy(buf, "error");
            break;

    }
}

void get_color_non_standard(char buf[10], int32_t color)
{
    switch (color) {
        case CDEFAULT:
            strcpy(buf, "default");
            break;
        case CRED:
            strcpy(buf, "red");
            break;
        case CGREEN:
            strcpy(buf, "green");
            break;
        case CYELLOW:
            strcpy(buf, "yellow");
            break;
        case CBLUE:
            strcpy(buf, "blue");
            break;
        case CMAGENTA:
            strcpy(buf, "magenta");
            break;
        case CCYAN:
            strcpy(buf, "cyan");
            break;
        case CWHITE:
            strcpy(buf, "white");
            break;
        case CBLACK:
            strcpy(buf, "black");
            break;
        default:
            strcpy(buf, "error");
            break;
    }
}

int color_translate(int color)
{
    switch (color) {
        case -1:
            return CDEFAULT;
        case COLOR_RED:
            return CRED;
        case COLOR_GREEN:
            return CGREEN;
        case COLOR_YELLOW:
            return CYELLOW;
        case COLOR_BLUE:
            return CBLUE;
        case COLOR_MAGENTA:
            return CMAGENTA;
        case COLOR_CYAN:
            return CCYAN;
        case COLOR_WHITE:
            return CWHITE;
        case COLOR_BLACK:
            return CBLACK;
        default:
            debug(">>>>>>>>. COLOR ERROR\n");
            return -1;

    }
}

int add_str_color(WINDOW* win, int32_t y, int32_t x, int32_t fgcol, int32_t bgcol, char* fmt, ...)
{
    // TODO keep matrices for every window that calls this function
    //      lookup matrix when function is called, clear on ui_erase
    // check: https://www.ibm.com/docs/en/aix/7.2?topic=library-manipulating-video-attributes
    assert(x>=0);
    assert(y>=0);

    /* Maintain current display representation with colors so we can replace background color */
    va_list ptr;
    va_start(ptr, fmt);
    char str[100] = {'\0'};
    vsprintf(str, fmt, ptr);
    va_end(ptr);

    //add_str(win, y, x, fgcol, bgcol, str);
    //return 0;






    //chtype* buf = calloc(6, sizeof(chtype));
    //mvwinchnstr(win, y, x, buf, 5);
    //wchar_t* buf = calloc(6, sizeof(chtype));

    //int res = mvin_wchnstr(win, y, x, buf, 5);


    //char chr = (char)(buf[0] & A_CHARTEXT);
    
    // get cell contents and extract attributes
    short prevfg, prevbg;
    char prevfgbuf[10] = {'\0'};
    char prevbgbuf[10] = {'\0'};
    char newfgbuf[10] = {'\0'};
    char newbgbuf[10] = {'\0'};

    int pair = PAIR_NUMBER((mvwinch(win, y, x) & A_ATTRIBUTES));
    pair_content(pair, &prevfg, &prevbg);

    int ctranslated = color_translate(prevfg);

    if (pair > 0  && prevfg != -1) {
    //if (pair > 0 && strcmp(str, UI_BLOCK) != 0) {

        get_color(prevfgbuf, prevfg);
        get_color(prevbgbuf, prevbg);
        get_color_non_standard(newfgbuf, fgcol);
        get_color_non_standard(newbgbuf, bgcol);
        //debug("[%dx%d] %s before Found color: %d, %s,%s\n", y, x, str, pair, fgbuf, bgbuf);


        //if (ctranslated == CDEFAULT)
        //    add_str(win, y, x, fgcol, bgcol, str);
        //else
            add_str(win, y, x, fgcol, ctranslated, str);
        
        //add_str(win, y, x, fgcol, ctranslated, str);
        //add_str(win, y, x, fgcol, ctranslated, str);
        if (fgcol == CMAGENTA) {
            char ctransbuf[10] = {'\0'};
            get_color_non_standard(ctransbuf, ctranslated);
            debug("translated prev fgcol: %d -> %d = %s\n", prevfg, ctranslated, prevfgbuf);

            //(ctranslated == CDEFAULT) ? debug("DEFAULT: %d\n", fg) : debug("ELSE\n");
            debug("write: %s %s,%s\n", str, newfgbuf, newbgbuf);
            debug("Color changed %d,%d->%d,%d - %s,%s->%s,%s\n\n", prevfg, prevbg, fgcol, prevfg, prevfgbuf, prevbgbuf, newfgbuf, ctransbuf);
        }
    }
        //add_str(win, y, x, fgcol, fg, str);
    else {
        add_str(win, y, x, fgcol, bgcol, str);


    }


    //mvwin_wchnstr(win, y, x, buf, 5);
    //free(buf);


    // TODO if cell is occupied in matrix, replace background color with color in matrix
    //Cell* c = ui_matrix_get(matrix, y, x);
    //if (strlen(c->chr) >0)
    //    add_str(win, y, x, fgcol, c->fgcol, fmt, ptr);
    //else
    //    add_str(win, y, x, fgcol, bgcol, fmt, ptr);



    // TODO save color in color matrix if string is a block
    //if (strcmp(str, UI_BLOCK) == 0)
    //    ui_matrix_set(matrix, y, x, str, fgcol, bgcol);

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

void ui_show_error(WINDOW* win, char* fmt, ...)
{
    va_list ptr;
    va_start(ptr, fmt);

    char msg[100] = {'\0'};
    vsprintf(msg, fmt, ptr);

    uint32_t xpos = (getmaxx(win) / 2) - (strlen(msg) / 2);
    uint32_t ypos = getmaxy(win) / 2;
    ui_erase(win);
    add_str(win, ypos, xpos, CRED, CDEFAULT, msg);
    wrefresh(win);

    va_end(ptr);
}
