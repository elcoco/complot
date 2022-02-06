#ifndef INDEX_H
#define INDEX_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct Group Group;
typedef struct Bucket Bucket;
typedef struct Index Index;
typedef struct Point Point;
typedef struct Line Line;

/* Point holds data for a one datapoint */
struct Point {
    int32_t x;
    int32_t open;
    int32_t high;
    int32_t low;
    int32_t close;
};

/* Container to put points in that can be stored inside the Group struct.
 * This enables for quick filtering of points  */
struct Line {
    char* name;

    // head of Points linked list
    Point** phead;
};


/* Group represents a column on the screen, with a width of one character.
 * A column has an xstart and xend, these are data/datetime boundaries.
 * A group contains the y values for all lines within this x range.
 *
 */
struct Group {
    // window start and end X values for this group
    int32_t wstart;
    int32_t wend;

    // neighbours of group linked list
    Group* prev;
    Group* next;

    // holds an array of lines
    // Line is a container that groups points per lines
    Line* lines;
};



/* Bucket is points container that is used by Index to be able to evenly space groups of points
 * It abstracts loads of datapoints
 * This enable Index to do very fast slicing of data to return groups of a certain length.
 */
struct Bucket {
    // window start and end X values for this group
    int32_t wstart;
    int32_t wend;

    // neighbours of group linked list
    Group* prev;
    Group* next;

    // holds an array of lines
    // Line is a container that groups points per lines
    Line* lines;
};

/* Index holds columns */
struct Index {
    // grow/extend index when we're trying to add an out of bound value
    int32_t grow_amount;

    // space between index x values
    int32_t spread;

    // the offset from index array index to data index
    int32_t start_key;

    // This is the actual index array
    // It holds grouped datapoints. Array indices are evenly spaced.
    // This enables fast slicing to quickly group datapoints.
    // indices can be mapped with: 
    // data_index = istart + (i * spread)
    // array_index = key - ((key-start_key) % spread)
    Bucket** buckets;

    // keep track of all points so we can re-index when spread changes
    // head of the Point linked list
    Point** phead;
};


// build an index of groups with a x xindow
// In the groups are sets of datapoints that fall within this window
// the sets are grouped by line name
Index* index_create(int32_t grow_amount);
int8_t index_build(Index* index, int32_t start_key, uint32_t spread);

// inserts point in appropriate line in group, creates new if data falls out of current index range
// line_id is the array index for line, is mapped by ENUM
int8_t index_insert(Index* index, uint8_t line_id, Point* point);

// get set of points that belong to line from group
Group* group_create();
int8_t group_destroy(Group* group);
int8_t group_get_line(Group* group, char* name);

Point* point_create(int32_t x, int32_t open, int32_t high, int32_t low, int32_t close);
int8_t point_destroy(Point* point);

#endif
