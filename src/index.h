#ifndef INDEX_H
#define INDEX_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include "utils.h"
#include "index_groups.h"
#include "config.h"


typedef struct Bin Bin;
typedef struct Index Index;
typedef struct Point Point;
typedef struct LineContainer LineContainer;
typedef struct OHLCContainer OHLCContainer;
typedef struct LineID LineID;

// forward declare
typedef struct Group Group;
typedef struct Groups Groups;

typedef enum LType LType;
enum LType {
    LTYPE_OHLC,
    LTYPE_LINE
};

/* LineID holds info about a line, it helps the indexer to find the right data when groups for a line are requested */
struct LineID{
    uint32_t id;
    LType ltype;
    //Line* next;
};

/* Point holds data for one datapoint. */
struct Point {
    double x;
    double y;
    double open;
    double high;
    double low;
    double close;
    Point* prev;
    Point* next;
    LineID* lineid;
};

/* Container to put normal line points in that is stored in Bin or Group struct.
 * This enables for quick filtering of points per line. */
struct LineContainer {
    // holds averages of points in line
    double y;
    bool is_empty;

    // amount of datapoints in line
    int32_t npoints;
};

/* Container to put candlestick points in that is stored in Bin or Group struct.
 * This enables for quick filtering of points per line. */
struct OHLCContainer {
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
    // Type can be a LineContainer or an OHLCContainer
    void** lcontainers;

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

    // keep track of the linetypes in index
    uint8_t nlines;
    LineID** lineids;

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
Point* index_get_last_point(Index* index, LineID* lineid);

// Reindex is done when the index spread changes, All bins are regenerated with a different window.
void    index_reindex(Index* index);

// inserts point in appropriate line in group, creates new if data falls out of current index range
// line_id is the array index for line, is mapped by ENUM
int8_t index_insert(Index* index, Point* point);

void   points_print(Point* p);
Point* point_create_cspoint(Index* index, LineID* lineid, double x, double open, double high, double low, double close);
Point* point_create_point(Index* index, LineID* lineid, double x, double y);
void   point_append(Point* p, Point** tail);
void   point_print(Point* p);

int8_t line_add_point(Bin* b, Point* p);

Bin* bin_create(Index* index, uint32_t i);
void bin_destroy(Bin* b, Index* index);
#endif
