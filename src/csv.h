#ifndef CSV_H
#define CSV_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 1000
#define FIELD_SIZE 500

typedef struct Line Line;

struct Line {
    char f0[FIELD_SIZE];
    char f1[FIELD_SIZE];
    char f2[FIELD_SIZE];
    char f3[FIELD_SIZE];
    char f4[FIELD_SIZE];
    char f5[FIELD_SIZE];
    char f6[FIELD_SIZE];
    char f7[FIELD_SIZE];
    char f8[FIELD_SIZE];
    char f9[FIELD_SIZE];
};

Line** read_csv(char* path);

#endif
