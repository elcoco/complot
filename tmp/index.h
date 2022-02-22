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
typedef struct LineBin LineBin;

/* Point holds data for one datapoint. */
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

/* A Group is a slice of the bins array from the Index.
 * It represents a column on the display. */
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

    // group id, is used to let x tickers follow candles
    uint32_t id;
};

/* Container returned from index_get_grouped().
 * Contains Group linked list and data limit information. */
struct Groups {
    // linked list of groups
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
};

/* Container to put points in that is stored in Bin struct.
 * This enables for quick filtering of points per line. */
struct LineBin {
    // holds averages of points in line
    double open;
    double high;
    double low;
    double close;

    bool is_empty;

    // X limits so we can keep track of open and close values when new points are added.
    // The actual window values are stored in Bin.
    double xmin;
    double xmax;

    // TODO maybe remove?
    // amount of datapoints in line
    int32_t npoints;
};

/* Bin is points container that is used by Index to be able to evenly space groups of points.
 * This enable Index to do very fast slicing of data to return groups of a certain length.  */
struct Bin {
    // window start and end X values for this bin
    double wstart;
    double wend;

    // Holds the line containers that hold data per line
    // Array is indexed by lineid
    LineBin** lines;

    bool is_empty;
};

/* Index holds all data.
 * build an index of groups with a x xindow.
 * In the groups are sets of datapoints that fall within this window. */
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

// the sets are grouped by line name
Index*  index_init(uint8_t nlines);
void    index_destroy(Index* index);
int8_t  index_build(Index* index);
int32_t index_map_to_index(Index* index, double x);
double  index_map_to_x(Index* index, int32_t i);
int32_t index_get_gstart(Index* index, uint32_t gsize, uint32_t amount);
void    index_print(Index* index);
bool    index_has_new_data(Index* index);
Point*  index_get_last_point(Index* index, uint32_t lineid);

// Reindex is done when the index spread changes, All bins are regenerated with a different window.
void    index_reindex(Index* index);

// inserts point in appropriate line in group, creates new if data falls out of current index range
// line_id is the array index for line, is mapped by ENUM
int8_t index_insert(Index* index, Point* point);

// get last amount of grouped bins with size gsize
Groups* index_get_grouped(Index* index, uint32_t lineid, uint32_t gsize, uint32_t amount, int32_t x_offset, int32_t y_offset);

void   points_print(Point* p);
Point* point_create(Index* index, uint32_t lineid, double x, double open, double high, double low, double close);
void   point_append(Point* p, Point** tail);
void   point_print(Point* p);

int8_t line_add_point(LineBin* l, Point* p);

Group* group_create(Index* index, int32_t gstart, uint32_t gsize, Group** gtail);
void   group_append(Group* g, Group** tail);

Groups* groups_init(Index* index, uint32_t lineid);
void    groups_print(Group* g);
void    groups_destroy(Groups* groups);
void    groups_update_limits(Groups* groups, Group* g);

Bin* bin_create(Index* index, uint32_t i);
void bin_destroy(Bin* b, Index* index);
#endif
