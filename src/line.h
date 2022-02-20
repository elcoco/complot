#ifndef LINE_H
#define LINE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "assert.h"

#include "plot.h"
#include "index.h"

typedef struct Line Line;
typedef struct Group Group;

/* Represents a line. Lineid is the index for lineid in the indexer */
struct Line {
    uint32_t lineid;
    char* name;

    // contains data for this line
    Group* group;

    Line* next;
};

Line* line_init(char* name);
void  line_destroy(Line* l);
void  line_print_lines(Line* l);

#endif
