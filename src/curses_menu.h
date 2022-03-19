#ifndef MENU_H
#define MENU_H

#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <menu.h>

#include "ui.h"

char* menu_select_symbol();
char* menu_select_interval(char** intervals);
char* menu_show(const char** options, uint32_t maxy, uint32_t maxx);

#endif
