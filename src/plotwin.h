#ifndef PLOTWIN_H
#define PLOTWIN_H

#include "request.h"
#include "index.h"
#include "plot.h"
#include "line.h"


// forward declare
typedef enum BinanceInterval BinanceInterval;

typedef struct PlotWin {
    Plot*  plot;
    Line*  lines[MAX_LINES];
    State* state;
    Index* index;
    pthread_t threadid;

    char symbol[MAX_SYMBOL_SIZE];
    bool is_stopped;
    BinanceInterval* OHLCinterval;
    uint32_t limit;
    pthread_mutex_t* lock;
    uint32_t timeout;
} PlotWin;

PlotWin* pw_init(WINDOW* win, Index* index, char* symbol, pthread_mutex_t* lock);

#endif
