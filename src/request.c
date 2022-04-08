#include "request.h"


size_t write_callback(char *data, size_t size, size_t nmemb, void *userdata)
{
    /* Handle received data */
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
