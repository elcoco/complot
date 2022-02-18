#ifndef READ_THREAD_H
#define READ_THREAD_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "index.h"
#include <locale.h>     // for utf8 in curses

#define BUF_SIZE 1000


enum lines {
    LINE1,
    LINE2
};

typedef struct Args Args;
struct Args {
    Index* index;
    uint8_t idt;
    uint8_t iopen;
    uint8_t ihigh;
    uint8_t ilow;
    uint8_t iclose;
    pthread_mutex_t* lock;
    bool is_stopped;
};

//void* read_stdin_thread(void* args);
void* read_file_thread(void* args);
bool fast_forward(char** c, char* search_lst, char* expected_lst, char* ignore_lst, char* buf);

#endif
