#include "request.h"

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

char* do_req(char* url)
{
    CURL *curl;
    CURLcode status;
    Response res = {0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(!curl) {
        curl_global_cleanup();
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    status = curl_easy_perform(curl);

    if(status != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(status));
        curl_global_cleanup();
        return NULL;
    }

    curl_global_cleanup();
    return res.response;
}

JSONObject* do_binance_req(char* symbol, BinanceInterval interval, int64_t tstart, uint32_t limit)
{
    /* limit is: default=500, max=1000
     * tstart is last close time unixtime integer
     *   if tstart = -1, use default
     * interval is BinanceInterval enum that will be mapped to a string */
    char interval_str[10] = {'\0'};
    strcpy(interval_str, binance_interval_map[interval]);

    char url[500] = {'\0'};
    sprintf(url, binance_url_fmt, symbol, interval_str, limit);
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
    return *((bool*)stopped);
}

void* binance_read_thread(void* thread_args)
{
    // TODO needs non blocking sleep
    // Cast thread arguments to correct type
    PlotWin* args = thread_args;
    int64_t tstart = -1;

    while (!args->is_stopped) {

        JSONObject* rn = do_binance_req(args->symbol, *(args->OHLCinterval), tstart, args->limit);

        if (rn == NULL) {
            debug("Failed to get data from binance\n");
            if (non_blocking_sleep(args->timeout, &check_exit_callback, &(args->is_stopped)))
                return NULL;
            continue;
        }

        //json_print(rn, 0);

        debug("Got %d OHCL datapoints\n", rn->length);

        double dt_open, dt_close, open, high, low, close, volume;

        for (int i=0 ; i<rn->length ; i++) {
            JSONObject* dp = rn->children[i];

            if (dp->length != 12) {
                debug("Received invalid datapoint\n");
                dp = dp->next;
                continue;
            }

            dt_open  = json_get_number(dp->children[0]);
            dt_close = json_get_number(dp->children[6]);
            open     = atof(json_get_string(dp->children[1]));
            high     = atof(json_get_string(dp->children[2]));
            low      = atof(json_get_string(dp->children[3]));
            close    = atof(json_get_string(dp->children[4]));
            volume   = atof(json_get_string(dp->children[5]));

            pthread_mutex_lock(args->lock);
            //debug("%d: %f, %f, %f, %f, %f\n", i, dt_open, open, high, low, close);
            //
            Line* l_vol = args->lines[0];
            Line* l_ohlc = args->lines[1];

            point_create_point(args->index, l_vol->lineid, dt_open, volume);
            point_create_cspoint(args->index, l_ohlc->lineid, dt_open, open, high, low, close);

            pthread_mutex_unlock(args->lock);

            if (i == rn->length-1)
                tstart = (int64_t)dt_close;
        }

        // do non blocking sleep
        if (non_blocking_sleep(args->timeout, &check_exit_callback, &(args->is_stopped)))
            return NULL;

    }
    return NULL;


}


