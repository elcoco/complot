#ifndef MENU_H
#define MENU_H

#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <menu.h>

#include "ui.h"

void menu_select_symbol();
char* menu_show(char** options, uint32_t noptions, uint32_t maxy, uint32_t maxx);

#endif
