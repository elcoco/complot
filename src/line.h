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

typedef struct Line Line;

// forward declare from index.h
typedef struct Group Group;
typedef struct Groups Groups;

// forward declare from yaxis.h
typedef struct Yaxis Yaxis;

/* Represents a line. Lineid is the index for lineid in the indexer */
struct Line {
    uint32_t lineid;
    char* name;

    // contains data for this line
    Groups* groups;

    Line* next;
    Yaxis* axis;
};

Line* line_init(char* name);
void  line_destroy(Line* l);
void  line_print_lines(Line* l);
int8_t line_set_data(Line* l, Groups* groups);

#endif
