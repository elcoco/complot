#ifndef INDEX_GROUPS_H
#define INDEX_GROUPS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include "index.h"

typedef struct Group Group;
typedef struct Groups Groups;
typedef struct GroupContainer GroupContainer;

// forward declare
typedef struct Point Point;
typedef struct LineContainer LineContainer;
typedef struct OHLCContainer OHLCContainer;
typedef struct Index Index;
typedef struct LineID LineID;

/* Represents data for a line in the plot */
struct GroupContainer {
    Group* group;

    // data limits for all data in index
    double dmin;
    double dmax;

    // data limits for this group
    double gmin;
    double gmax;

    // indicate if groups has any data in it or just empty groups
    bool is_empty; 

    // last datapoint for this lineid
    Point* plast;

    LineID* lineid; 
};

/* A Group is a slice of the bins array for one line from the Index.
 * It represents a column on the display. */
struct Group {
    double wstart;
    double wend;

    double y;
    double open;
    double high;
    double low;
    double close;
    bool is_empty;

    // iter groups using linked list
    Group* next;
    Group* prev;

    // group id, is used to let x tickers follow candles
    uint32_t id;
};

/* Container returned from index_get_grouped(). */
struct Groups {
    // data limits for all data in index
    double dmin;
    double dmax;

    // data limits for this group
    double gmin;
    double gmax;

    // indicate if groups has any data in it or just empty groups
    bool is_empty; 
                   
    LineID* lineid; 
    uint32_t nlines;

    // groups indexed by lineid
    GroupContainer** lines;
};

Group* group_init(Index* index, int32_t gstart, uint32_t gsize, Group** gtail);
void   group_append(Group* g, Group** tail);
Group* group_ohlc_update(Group* g, OHLCContainer* lb);
Group* group_line_update(Group* g, LineContainer* lb);

Groups* groups_init(Index* index);
void    groups_print(Groups* groups);
void    groups_destroy(Groups* groups);
void    groups_update_limits(Groups* groups, Group* g);

// get last amount of grouped bins with size gsize
Groups* index_get_grouped(Index* index, uint32_t gsize, uint32_t amount, int32_t x_offset, int32_t y_offset);

#endif
