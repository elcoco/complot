#ifndef PLOTWIN_H
#define PLOTWIN_H

#include "request.h"
#include "index.h"
#include "plot.h"
#include "line.h"
#include "request.h"

typedef struct PlotWin PlotWin;

// forward declare
typedef enum BinanceInterval BinanceInterval;
typedef struct Plot Plot;
typedef struct Line Line;
typedef struct Request Request;


typedef struct State {
    bool is_paused;
    bool is_stopped;
    bool is_pan_changed;
    bool is_resized;

    int clicked_x;
    int clicked_y;

    int panx;
    int pany;

    bool set_autorange;

    int32_t gsize;

    // pointer array to all plot wins and current selected PlotWin
    PlotWin** pws;
    PlotWin*  cur_pw;
    uint32_t pw_length;
} State;

typedef struct PlotWin {
    Plot*  plot;
    Line*  lines[MAX_LINES];
    State* state;
    Index* index;

    // these lines are shown in plot
    // TODO this can easily be replaced with the is_enabled attribute in Line struct
    //bool* lines_enabled;
    //bool line_changed;

    pthread_t threadid;
    Request* request;
} PlotWin;


PlotWin* pw_init(WINDOW* win, Index* index, State* state, char* symbol, pthread_mutex_t* lock);
int8_t pw_update_all(PlotWin** pws, uint32_t length, pthread_mutex_t* lock);
int8_t pw_update(PlotWin* pw, pthread_mutex_t* lock);

State* state_init();

#endif
