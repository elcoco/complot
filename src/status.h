#ifndef STATUS_H
#define STATUS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <curses.h>
#include <string.h>
#include "assert.h"

#include "ui.h"


typedef struct StatusItem StatusItem;
typedef struct StatusBar StatusBar;

struct StatusItem {
    char key[20];
    char value[200];
};

struct StatusBar {
    WINDOW* win;
    uint32_t xsize;
    uint32_t ysize;
    uint32_t length;    // kv array length
    StatusItem** kv;
    bool is_changed;
    uint32_t color;
};

StatusBar*  status_init();
void        status_destroy(StatusBar* sb);
StatusItem* status_set(StatusBar* sb, char* k, char* fmt, ...);
void        status_draw(StatusBar* sb);

#endif
