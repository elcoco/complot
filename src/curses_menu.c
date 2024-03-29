#include "curses_menu.h"


ITEM** menu_filter_items(const char** options, char* inp)
{
    /* Filter array of strings based on input string, return menu ITEM* array */
    // NOTE!!!! don't put newlines at end of strings, curses will complain!!!!

    int32_t ngrow = 300;
    int32_t itemsiz = ngrow;
    ITEM** items = malloc(itemsiz*sizeof(ITEM*));

    const char** poption = options;

    int32_t i = 0;
    while (*poption != NULL) {
        char* loption = strdup(*poption);
        char* linp = strdup(inp);

        if (strstr(str_to_lower(loption), str_to_lower(linp)) || strlen(inp) == 0) {
        
            if (i >= itemsiz-1) {
                itemsiz += ngrow;
                items = realloc(items, itemsiz*sizeof(ITEM*));
            }

            if ((items[i++] = new_item(*poption, "")) == NULL) {
                //debug("Failed to create new menu item\n");
            }
        }
        free(loption);
        free(linp);
        poption++;
    }

    // end array with NULL
    if (i >= itemsiz-1)
        items = realloc(items, itemsiz*sizeof(ITEM*));

    items[i] = NULL;

    return items;
}

char* menu_select_symbol()
{
    char** symbols = do_binance_symbols_req();
    if (symbols == NULL)
        return NULL;

    char* result = menu_show((const char**)symbols, LINES-6, 15);
    if (strlen(result) <= 0)
        return NULL;

    // TODO free symbols

    return result;
}

void destroy_items(ITEM** items)
{
    /* free item destroys the pointers to the strings.
     * curses doesn't store the actual strings! */
    for (int i=0 ; items[i] != NULL ; i++)
        free_item(items[i]);
    // TODO this causes segfault
    //free(items);
}


char* menu_show(const char** options, uint32_t maxy, uint32_t maxx)
{
    MENU* menu;
    WINDOW* win;
    char inp[MENU_MAX_INPUT_SIZE] = {'\0'};
    char* result = strdup("");
    ITEM** items = menu_filter_items(options, "");
    int ch;

    uint32_t width = (COLS >= maxx) ? maxx : COLS;
    uint32_t height = (LINES >= maxy) ? maxy : LINES;
    uint32_t xpos = (COLS/2) - (width/2);
    uint32_t ypos = (LINES/2) - (height/2);

    curs_set(1);

    win = newwin(height, width, ypos, xpos);
    keypad(win, TRUE);   // Enables keypad mode also necessary for mouse clicks
    box(win, 0, 0);

    menu = new_menu(items);
    set_menu_format(menu, height-3, 1);

    set_menu_sub(menu, derwin(win, height-2, width-2, 1, 1));
    set_menu_mark(menu, " ");
    post_menu(menu);

    add_str(win, getmaxy(win)-2, 2, CWHITE, CDEFAULT, ":");

    refresh();
    wrefresh(win);
                         //
    while((ch = wgetch(win)) != 27 ) {
        switch(ch) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                continue;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                continue;
            case KEY_LEFT:
                menu_driver(menu, REQ_FIRST_ITEM);
                continue;
            case KEY_RIGHT:
                menu_driver(menu, REQ_LAST_ITEM);
                continue;
            case KEY_BACKSPACE:
                if (strlen(inp) > 0)
                    inp[strlen(inp)-1] = '\0';
                break;
            case '\n':
                // curses does weird stuff with selected item so check if
                // there are actually items in the menu
                if (items[0] != NULL) {
                    free(result);
                    result = strdup(item_name(current_item(menu)));
                }
                goto cleanup;
            case 32 ... 126:        // printable ascii
                char tmp[2] = {'\0'};
                sprintf(tmp, "%c", ch);
                strcat(inp, tmp);
                break;
            default:
                continue;
        }

        // draw filtered items in menu
        unpost_menu(menu);
        destroy_items(items);
        items = menu_filter_items(options, inp);

        if (set_menu_items(menu, items) != E_OK) {
            werase(win);
            box(win, 0, 0);
        } else {
            post_menu(menu);
        }
            
        // clear line and redraw border
        wmove(win, height-2, 1);
        wclrtoeol(win);
        add_str(win, getmaxy(win)-2, 2, CWHITE, CDEFAULT, ":%s", inp);
        box(win, 0, 0);
        wrefresh(win);
    }

    cleanup:
    unpost_menu(menu);
    free_menu(menu);

    // first free menu and THEN items or else heap corruption
    destroy_items(items);
    free(items);

    curs_set(0);

    endwin();
    return result;
}
