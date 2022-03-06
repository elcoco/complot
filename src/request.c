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

int32_t do_req(char* url, char* buf)
{
    CURL *curl;
    CURLcode status;
    Response res = {0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(!curl) {
        curl_global_cleanup();
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    status = curl_easy_perform(curl);

    if(status != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(status));
        curl_global_cleanup();
        return -1;
    }

    curl_global_cleanup();
    strcpy(buf, res.response);
    free(res.response);
    return 0;
}


