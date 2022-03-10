#include "plotwin.h"


PlotWin* pw_init(WINDOW* win, Index* index, char* symbol, pthread_mutex_t* lock)
{
    /* Create a full new plot window, and start threads */
    PlotWin* pw = malloc(sizeof(PlotWin));
    pw->index = index;

    pw->plot = plot_init(win);

    pw->lines[0] = line_line_init("Volume");
    pw->lines[0]->color = CBLUE;
    pw->lines[1] = line_ohlc_init(symbol);

    yaxis_add_line(pw->plot->lyaxis, pw->lines[0]);
    yaxis_add_line(pw->plot->ryaxis, pw->lines[1]);

    pw->state = state_init();

    pw->lock         = lock;
    pw->is_stopped   = false;
    pw->OHLCinterval = malloc(sizeof(BinanceInterval));
    *(pw->OHLCinterval) = BINANCE_5_MINUTES;
    pw->limit        = 500;
    pw->timeout      = 60 * 1000 * 1000;
    strcpy(pw->symbol, symbol);

    pthread_create(&(pw->threadid), NULL, binance_read_thread, pw);
    return pw;
}

