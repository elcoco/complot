#ifndef LINE_H
#define LINE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "assert.h"

#include "plot.h"
#include "index.h"
#include "yaxis.h"
#include "config.h"

typedef struct Line Line;

// forward declare from index.h
typedef struct Group Group;
typedef struct Groups Groups;
typedef struct GroupContainer GroupContainer;
typedef struct Yaxis Yaxis;
typedef struct LineID LineID;
typedef struct Index Index;

/* Represents a line. Lineid is the index for lineid in the indexer */
struct Line {
    char* name;
    bool  is_enabled;

    // unicode character takes at most 4 bytes
    char icon[5];
    char chr[5];

    // contains data for this line
    GroupContainer* gc;

    Line* next;
    Yaxis* axis;

    uint32_t color;
    LineID* lineid;
};

Line* line_init(char* name);
Line* line_line_init(char* name);
Line* line_ohlc_init(char* name);

void  line_destroy(Line* l);
void  line_print_lines(Line* l);
int8_t line_set_data(Line* l, GroupContainer* groups);

#endif
