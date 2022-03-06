#ifndef REQUEST_H
#define REQUEST_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

typedef struct Response {
    char* response;
    size_t size;
} Response;

int32_t do_req(char* url, char* buf);

#endif
