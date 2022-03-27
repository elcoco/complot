#include "request.h"

const char* binance_interval_map[] = { "1m", "3m", "5m", "15m", "30m", "1h", "2h", "4h", "6h", "8h", "12h", "1d", "3d", "1w", "1M", NULL };

size_t write_callback(char *data, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    Response* res = userdata;

    char* ptr = realloc(res->response, res->size + realsize +1);

    // check out of memory
    if (ptr == NULL)
        return 0;

    res->response = ptr;
    memcpy(&(res->response[res->size]), data, realsize);
    res->size += realsize;
    res->response[res->size] = '\0';
    return realsize;
}

char* do_req(const char* url)
{
    CURL *curl;
    CURLcode status;
    Response res = {.size=0, .response=NULL};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(!curl) {
        debug("Failed to init CURL\n");
        goto on_fail;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    status = curl_easy_perform(curl);

    if(status != CURLE_OK) {
        debug("CURL request failed: %s\n", curl_easy_strerror(status));
        goto on_fail;
    }

    curl_easy_cleanup(curl);
    //curl_global_cleanup();
    return res.response;

    on_fail:
        if (res.response)
            free(res.response);
        //curl_global_cleanup();
        curl_easy_cleanup(curl);
        return NULL;
}

char** do_binance_symbols_req()
{
    /* Request array of symbols, end array with NULL */
    int32_t ngrow = 300;
    int32_t ssiz = ngrow;
    char** symbols = malloc(ssiz*sizeof(char*));
    int32_t i = 0;

    char* data = do_req(binance_ticker_url_fmt);
    if (data == NULL)
        return NULL;

    JSONObject* rn = json_load(data);
    free(data);

    if (rn == NULL)
        return NULL;

    if (!rn->is_array)
        goto on_error;

    JSONObject* symbol_obj = rn->value;
    while (symbol_obj != NULL) {

        if (!symbol_obj->is_object)
            goto on_error;

        JSONObject* child = symbol_obj->value;
        while (child != NULL) {

            if (strcmp(child->key, "symbol") == 0) {
                if (i >= ssiz-1) {
                    ssiz += ngrow;
                    symbols = realloc(symbols, ssiz*sizeof(char*));
                }
                symbols[i++] = strdup(json_get_string(child));
            }
            child = child->next;
        }
        symbol_obj = symbol_obj->next;
    }

    // end array with NULL
    if (i >= ssiz-1)
        symbols = realloc(symbols, ssiz*sizeof(char*));

    symbols[i] = NULL;

    json_obj_destroy(rn);
    return symbols;

    on_error:
    json_obj_destroy(rn);
    return NULL;
}

JSONObject* do_binance_ohlc_req(char* symbol, BinanceInterval interval, int64_t tstart, uint32_t limit)
{
    /* limit is: default=500, max=1000
     * tstart is last close time unixtime integer
     *   if tstart = -1, use default
     * interval is BinanceInterval enum that will be mapped to a string */
    char interval_str[10] = {'\0'};
    strcpy(interval_str, binance_interval_map[interval]);

    char url[500] = {'\0'};
    sprintf(url, binance_ohlc_url_fmt, symbol, interval_str, limit);
    char tstart_str[100] = {'\0'};

    if (tstart >= 0) {
        sprintf(tstart_str, "&startTime=%ld", tstart);
        strcat(url, tstart_str);
    }

    char* data = do_req(url);
    if (data == NULL)
        return NULL;

    JSONObject* rn = json_load(data);
    
    free(data);
    return rn;
}

bool check_exit_callback(void* stopped)
{
    /* Used by non_blocking_sleep to signal the stopped state and trigger thread/program exit to main process */
    return *((bool*)stopped);
}

void* binance_read_thread(void* thread_args)
{
    // TODO needs non blocking sleep
    // Cast thread arguments to correct type
    Request* args = thread_args;
    //PlotWin* args = thread_args;
    int64_t tstart = -1;

    while (!args->is_stopped) {
        debug("GETTING DATA\n");

        assert(args->index);
        assert(args->symbol);
        JSONObject* rn = do_binance_ohlc_req(args->symbol, (args->OHLCinterval), tstart, args->limit);

        if (rn == NULL) {
            debug("Failed to get data from binance\n");
            if (non_blocking_sleep(args->timeout, &check_exit_callback, &(args->is_stopped)))
                return NULL;
            continue;
        }

        //json_print(rn, 0);

        if (rn->length > 0)
            debug("[%s] Received %d datapoints\n", args->symbol, rn->length);

        double dt_open, dt_close, open, high, low, close, volume;

        for (int i=0 ; i<rn->length ; i++) {
            JSONObject* dp = rn->children[i];

            if (dp->length != 12) {
                args->is_stopped = true;
                debug("Received invalid datapoint\n");
                //dp = dp->next;
                //continue;
                break;
            }

            if (!(dt_open  = json_get_number(dp->children[0])))
                continue;
            if (!(dt_close = json_get_number(dp->children[6])))
                continue;
            if (!(open     = atof(json_get_string(dp->children[1]))))
                continue;
            if (!(high     = atof(json_get_string(dp->children[2]))))
                continue;
            if (!(low      = atof(json_get_string(dp->children[3]))))
                continue;
            if (!(close    = atof(json_get_string(dp->children[4]))))
                continue;
            if (!(volume   = atof(json_get_string(dp->children[5]))))
                continue;

            pthread_mutex_lock(args->lock);
            //debug("%d: %f, %f, %f, %f, %f\n", i, dt_open, open, high, low, close);
            //
            Line* l_vol = args->lines[0];
            Line* l_ohlc = args->lines[1];

            if (i % 5 == 0)
                point_create_point(args->index, l_vol->lineid, dt_open, volume);
            point_create_cspoint(args->index, l_ohlc->lineid, dt_open, open, high, low, close);

            pthread_mutex_unlock(args->lock);

            if (i == rn->length-1)
                tstart = (int64_t)dt_close;
        }

        //json_print(rn, 0);
        json_obj_destroy(rn);

        // do non blocking sleep
        if (non_blocking_sleep(args->timeout, &check_exit_callback, &(args->is_stopped)))
            break;

    }
    debug("[%s] thread exit...\n", args->symbol);
    return NULL;
}
