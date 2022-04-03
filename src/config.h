#ifndef CONFIG_H
#define CONFIG_H

#define INDEX_DEFAULT_GROW_AMOUNT 2000

#define LINE_LINE_ICON        "█"
#define LINE_OHLC_ICON        "┿"
//#define LINE_DEFAULT_LINE_CHR "X"
#define LINE_DEFAULT_LINE_CHR "█"

#define PLOT_LARROW "◀"
#define PLOT_RARROW "▶"

#define PLOT_MIN_WINDOW_XSIZE 10
#define PLOT_MIN_WINDOW_YSIZE 10

#define LEGEND_MAX_SIZE 200

#define XAXIS_TICK_SPACING 15
#define XAXIS_GRID_CHR "│"
#define XAXIS_GRID_COLOR CDEFAULT

//#define YAXIS_LDATA_LINE_CHR "-"
#define YAXIS_LDATA_LINE_CHR "─"
//#define YAXIS_OHLC_BODY "O"
//#define YAXIS_OHLC_WICK "|"
#define YAXIS_OHLC_BODY "█"
#define YAXIS_OHLC_WICK "┃"

#define YAXIS_BR "╭"
#define YAXIS_TB "│"
#define YAXIS_TR "╰"
#define YAXIS_LB "╮"
#define YAXIS_LT "╯"
#define YAXIS_LR "─"

#define DEFAULT_GROUP_SIZE 1
#define DEFAULT_PAN_STEPS 3
#define DEFAULT_PAN_BIG_STEPS 5

// L means wide character
#define UI_BLOCK L"█"

// TODO this should really be dynamic
#define MAX_LINES 20

#define GRAPH_MIN_SIZE 5

#define LOG_PATH "./complot.log"

#define MAX_LOG_LINE_LENGTH 100
#define MAX_LOG_LINE_NO_LENGTH 10

#endif
