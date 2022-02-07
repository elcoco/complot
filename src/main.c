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
#define GROUPS 5

// TODO extend index when out of bounds
// DONE get groups x amount of groups from last known data
// TODO create points linked list
// TODO create CSV file reader
// TODO add xwin dimension to group

enum lines {
    LINE1,
    LINE2
} lines;

int main(int argc, char **argkv)
{

    Index* index = index_create(INDEX_GROW_AMOUNT, INDEX_SPREAD, NLINES);

    for (int32_t i=100 ; i<142 ; i++) {

        uint32_t open  = get_rand(0, 2000);
        uint32_t high  = get_rand(0, 2000);
        uint32_t low   = get_rand(0, 2000);
        uint32_t close = get_rand(0, 2000);
        Point* p = point_create(i, open, high, low, close);

        index_insert(index, LINE1, p);
    }

    index_print(index);

    Group** groups = index_get_grouped(index, LINE1, GROUP_SIZE, GROUPS);
    groups_print(groups, GROUPS);



    return 0;
}
