#ifndef PLOTWIN_H
#define PLOTWIN_H

#include "request.h"
#include "index.h"
#include "plot.h"
#include "line.h"
#include "request.h"
#include "curses_menu.h"

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

    bool do_create_pw;

    int clicked_x;
    int clicked_y;

    int panx;
    int pany;

    bool set_autorange;

    int32_t gsize;

    // pointer array to all plot wins and current selected PlotWin
    PlotWin** pws;

    // index to current PlotWin
    uint32_t  cur_pw;

    // pws array length
    uint32_t pws_length;
} State;

typedef struct PlotWin {
    Plot*  plot;
    Line*  lines[MAX_LINES];
    State* state;
    Index* index;

    pthread_t threadid;
    Request* request;
} PlotWin;


PlotWin* pw_init(WINDOW* win, State* state, char* symbol, pthread_mutex_t* lock);
void     pw_destroy(PlotWin* pw);
int8_t   pw_update_all(PlotWin** pws, uint32_t length, pthread_mutex_t* lock, bool force);
int8_t   pw_update(PlotWin* pw, pthread_mutex_t* lock, bool force);
char* pw_select_interval(PlotWin* pw, const char** intervals);

State* state_init();
int8_t state_add_pw(State* s, PlotWin* pw);
int8_t state_remove_pw(State* s, PlotWin* pw);
int8_t state_resize_pws(PlotWin** pws, uint32_t length);

#endif
