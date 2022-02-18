#ifndef INDEX_H
#define INDEX_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>


#define INDEX_DEFAULT_GROW_AMOUNT 2000
#define INDEX_DEFAULT_SPREAD 5

typedef struct Group Group;
typedef struct Groups Groups;
typedef struct Bin Bin;
typedef struct Index Index;
typedef struct Point Point;
typedef struct Line Line;

/* Point holds data for a one datapoint */
struct Point {
    double x;
    double open;
    double high;
    double low;
    double close;
    Point* prev;
    Point* next;
    uint32_t lineid;
};

/* A Group is a slice of the bins array in Index, for one line.
 * It represents a column on the display
 * Groups should be cached
 */
struct Group {
    //Bin** bins;
    double wstart;
    double wend;

    double open;
    double high;
    double low;
    double close;
    bool is_empty;

    // iter groups using linked list
    Group* next;
    Group* prev;

    // group number, is used to draw xaxis tickers beneath candles
    uint32_t id;
};

/* container returned from index_get_grouped() */
struct Groups {
    Group* group;
    Group* gtail;

    // data limits for all data in index
    double dmin;
    double dmax;

    // data limits for this group
    double gmin;
    double gmax;

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
    double xmin;
    double xmax;

    // amount of datapoints in line
    int32_t npoints;
};

/* Bin is points container that is used by Index to be able to evenly space groups of points
 * It abstracts loads of datapoints
 * This enable Index to do very fast slicing of data to return groups of a certain length.
 */
struct Bin {
    // window start and end X values for this bin
    double wstart;
    double wend;

    // holds the line containers that hold the averages for this bin per line
    Line** lines;

    bool is_empty;
};

/* Index holds columns */
struct Index {
    // grow/extend bins array when we're trying to add an out of bound value
    int32_t grow_amount;

    // space between index x values
    double spread;

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

    // amount of points in index
    uint32_t npoints;

    // index of last non-empty bin, this is used to quickly find
    // end of data when creating groups
    int32_t ilast;

    // is true after first index build
    bool is_initialized;

    // needs to be checked using: index_has_new_data()
    bool has_new_data;

    // lowest x value, used as offset to map x values to array indices
    double xmin;
};

// build an index of groups with a x xindow
// In the groups are sets of datapoints that fall within this window
// the sets are grouped by line name
Index*  index_init(uint8_t nlines);
void    index_destroy(Index* index);
int8_t  index_build(Index* index);
int32_t index_map_to_index(Index* index, double x);
double  index_map_to_x(Index* index, int32_t i);
int32_t index_get_gstart(Index* index, uint32_t gsize, uint32_t amount);
void    index_reindex(Index* index);
void    index_print(Index* index);
bool    index_has_new_data(Index* index);

// inserts point in appropriate line in group, creates new if data falls out of current index range
// line_id is the array index for line, is mapped by ENUM
int8_t index_insert(Index* index, Point* point);

// get last amount of grouped bins with size gsize
Groups* index_get_grouped(Index* index, uint8_t lineid, uint32_t gsize, uint32_t amount, int32_t x_offset, int32_t y_offset);

void   points_print(Point* p);
Point* point_create(Index* index, uint32_t lineid, double x, double open, double high, double low, double close);
void   point_append(Point* p, Point** tail);

int8_t line_add_point(Line* l, Point* p);

Group* group_create(Index* index, int32_t gstart, uint32_t gsize);
void   groups_print(Group* g);
void   groups_destroy(Groups* groups);
void   group_append(Group* g, Group** tail);

Bin* bin_create(Index* index, uint32_t i);
void bin_destroy(Bin* b, Index* index);
#endif
