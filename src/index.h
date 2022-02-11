#ifndef INDEX_H
#define INDEX_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>


typedef struct Group Group;
typedef struct Groups Groups;
typedef struct Bin Bin;
typedef struct Index Index;
typedef struct Point Point;
typedef struct Line Line;

/* Point holds data for a one datapoint */
struct Point {
    int32_t x;
    double open;
    double high;
    double low;
    double close;
    Point* prev;
    Point* next;
};

/* A Group is a slice of the bins array in Index, for one line.
 * It represents a column on the display
 * Groups should be cached
 */
struct Group {
    //Bin** bins;
    int32_t wstart;
    int32_t wend;

    double open;
    double high;
    double low;
    double close;
    bool is_empty;

    // iter groups using linked list
    Group* next;
};

/* container returned from index_get_grouped() */
struct Groups {
    Group* group;
    double dmin;
    double dmax;
    bool is_empty; // indicate if groups has any data in it or just empty groups
};

/* Container to put points in that can be stored inside the Group struct.
 * This enables for quick filtering of points  */
struct Line {
    // holds averages of points in line
    double open;
    double high;
    double low;
    double close;

    bool is_empty;

    // x limits so we can keep track of open and close values
    // The actual window values are stored in Bin
    int32_t xmin;
    int32_t xmax;
    //int32_t wstart;
    //int32_t wend;

    // amount of datapoints in line
    int32_t npoints;
};

/* Bin is points container that is used by Index to be able to evenly space groups of points
 * It abstracts loads of datapoints
 * This enable Index to do very fast slicing of data to return groups of a certain length.
 */
struct Bin {
    // window start and end X values for this bin
    int32_t wstart;
    int32_t wend;

    // holds the line containers that hold the averages for this bin per line
    Line** lines;

    bool is_empty;
};



/* Index holds columns */
struct Index {
    // grow/extend index when we're trying to add an out of bound value
    int32_t grow_amount;

    // space between index x values
    int32_t spread;

    // This is the actual index array
    // It holds grouped datapoints. Array indices are evenly spaced.
    // This enables fast slicing to quickly group datapoints.
    // indices can be mapped with: 
    // data_index = istart + (i * spread)
    // array_index = key - ((key-start_offset) % spread)
    Bin** bins;

    // keep track of all points so we can re-index when spread changes
    // head of the Point linked list
    Point** phead;
    Point** ptail;

    // current data limits represented by index
    double dmin;   // this offset is important for calculating array index
    double dmax;

    // current index size
    int32_t isize;

    // amount of lines this index represents
    uint8_t nlines;

    // index of last non-empty bin, this is used to quickly find
    // end of data when creating groups
    int32_t ilast;

    // is true after first index build
    bool is_initialized;

};

// build an index of groups with a x xindow
// In the groups are sets of datapoints that fall within this window
// the sets are grouped by line name
Index* index_create(int32_t grow_amount, int32_t spread, uint8_t nlines);
int8_t index_build(Index* index);
int32_t index_map_to_index(Index* index, int32_t x);
int32_t index_map_to_x(Index* index, int32_t i);
int32_t index_get_gstart(Index* index, uint32_t gsize, uint32_t amount);

// inserts point in appropriate line in group, creates new if data falls out of current index range
// line_id is the array index for line, is mapped by ENUM
int8_t index_insert(Index* index, uint8_t lineid, Point* point);
void index_print(Index* index);

// get last amount of grouped bins with size gsize
Groups* index_get_grouped(Index* index, uint8_t lineid, uint32_t gsize, uint32_t amount, int32_t x_offset, int32_t y_offset);

Point* point_create(int32_t x, double open, double high, double low, double close);
int8_t point_destroy(Point* point);

int8_t line_add_point(Line* l, Point* p);

Group* group_create(Index* index, int32_t gstart, uint32_t gsize);

void groups_print(Group* g);
void points_print(Point* p);

void point_append(Point* p, Point** tail);
void group_append(Group* g, Group** tail);
#endif
