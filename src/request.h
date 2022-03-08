#ifndef REQUEST_H
#define REQUEST_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#include "json.h"
#include "index.h"
#include "utils.h"

typedef struct Response {
    char* response;
    size_t size;
} Response;

// enum values can be mapped to strings using binance_interval_map
typedef enum BinanceInterval {
    BINANCE_1_MINUTES,
    BINANCE_3_MINUTES,
    BINANCE_5_MINUTES,
    BINANCE_15_MINUTES,
    BINANCE_30_MINUTES,
    BINANCE_1_HOURS,
    BINANCE_2_HOURS,
    BINANCE_4_HOURS,
    BINANCE_6_HOURS,
    BINANCE_8_HOURS,
    BINANCE_12_HOURS,
    BINANCE_1_DAYS,
    BINANCE_3_DAYS,
    BINANCE_1_WEEK,
    BINANCE_1_MONTH
} BinanceInterval;

typedef struct RequestArgs {
    Index* index;
    LineID* lineid;
    pthread_mutex_t* lock;
    bool is_stopped;
    char symbol[50];
    uint32_t timeout;
    BinanceInterval OHLCinterval;
    uint32_t limit;
} RequestArgs;

static char binance_interval_map[][4] = { "1m", "3m", "5m", "15m", "30m", "1h", "2h", "4h", "6h", "8h", "12h", "1d", "3d", "1w", "1M" };
static char binance_url_fmt[] = "https://api.binance.com/api/v3/klines?symbol=%s&interval=%s&limit=%d";

char* do_req(char* url);

void* binance_read_thread(void* args);
JSONObject* do_binance_req(char* symbol, BinanceInterval interval, int64_t tstart, uint32_t limit);

#endif
