#include "curses_menu.h"


ITEM** menu_filter_items(char** options, uint32_t length, char* inp)
{
    ITEM** items = malloc(100*sizeof(ITEM*));
    items[0] = NULL;
    int i, j;

    for (i=0, j=1 ; i<length ; i++) {
        if (strstr(options[i], inp) || strlen(inp) == 0) {

            //items = realloc(items, j+1 * sizeof(ITEM*));
            items[j-1] = new_item(options[i], "");
            items[j] = NULL;

            j++;
            debug("MATCH: %s\n", options[i]);
        }
    }
    debug("END\n");
    return items;
}

void menu_select_symbol()
{
    char* options[] = {
        "bever",
        "disko",
        "banaan",
        "aardappel",
        "blub",
        "disko2"
    };
    char* result = menu_show(options, 6, 20, 20);
    if (strlen(result))
        debug("return: %s\n", result);
    free(result);
}

void destroy_items(ITEM** items)
{
    for (int i=0 ; items[i] != NULL ; i++)
        free_item(items[i]);
    free(items);
}


char* menu_show(char** options, uint32_t noptions, uint32_t maxy, uint32_t maxx)
{
    MENU* menu;
    WINDOW* win;
    char inp[100] = {'\0'};
    char* result = strdup("");
    ITEM** items = menu_filter_items(options, noptions, "");
    int ch;

    uint32_t width = (COLS >= maxx) ? maxx : COLS;
    uint32_t height = (LINES >= maxy) ? maxy : LINES;
    uint32_t xpos = (COLS/2) - (width/2);
    uint32_t ypos = (LINES/2) - (height/2);

    menu = new_menu(menu_filter_items(options, noptions, ""));
    win = newwin(height, width, ypos, xpos);
    box(win, 0, 0);
    set_menu_sub(menu, derwin(win, height-2, width-2, 1, 1));
    set_menu_mark(menu, " ");
    post_menu(menu);
    keypad(win, TRUE);   // Enables keypad mode also necessary for mouse clicks

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

        unpost_menu(menu);
        destroy_items(items);
        items = menu_filter_items(options, noptions, inp);
        if (set_menu_items(menu, items) != E_OK) {
            clear_win(win);
            box(win, 0, 0);
        } else {
            post_menu(menu);
        }
        add_str(win, getmaxy(win)-2, 1, CWHITE, CDEFAULT, inp);
        wrefresh(win);
    }

    cleanup:
    unpost_menu(menu);
    free_menu(menu);
    endwin();
    refresh();
    return result;
}
