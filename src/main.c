#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "index.h"
#include "utils.h"

#define INDEX_GROW_AMOUNT 1000
#define INDEX_SPREAD 2
#define NLINES 2

#define GROUP_SIZE 3
#define GROUPS 20

// TODO extend index when out of bounds
// DONE get groups x amount of groups from last known data
// DONE create points linked list
// TODO create CSV file reader
// DONE add xwin dimension to group
// TODO draw candlesticks
// TODO return groups as linked list

enum lines {
    LINE1,
    LINE2
} lines;

int main(int argc, char **argkv)
{

    Index* index = index_create(INDEX_GROW_AMOUNT, INDEX_SPREAD, NLINES);

    for (int32_t i=100 ; i<420 ; i++) {

        uint32_t open  = get_rand(0, 2000);
        uint32_t high  = get_rand(0, 2000);
        uint32_t low   = get_rand(0, 2000);
        uint32_t close = get_rand(0, 2000);
        Point* p = point_create(i, open, high, low, close);

        index_insert(index, LINE1, p);
    }

    index_print(index);

    Group* ghead = index_get_grouped(index, LINE1, GROUP_SIZE, GROUPS);
    groups_print(ghead);

    //points_print(*(index->phead));



    return 0;
}
