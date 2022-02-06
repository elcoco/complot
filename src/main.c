#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "index.h"
#include "utils.h"

#define XWIN 10
#define INDEX_GROW_AMOUNT 1000
#define INDEX_START_KEY 50
#define INDEX_SPREAD 5
#define AMOUNT_LINES 2


enum lines {
    LINE1,
    LINE2
} lines;

int main(int argc, char **argkv)
{

    Index* index = index_create(INDEX_GROW_AMOUNT);
    index_build(index, INDEX_START_KEY, INDEX_SPREAD, AMOUNT_LINES);

    for (int32_t i=100 ; i<150 ; i++) {

        uint32_t open  = get_rand(0, 2000);
        uint32_t high  = get_rand(0, 2000);
        uint32_t low   = get_rand(0, 2000);
        uint32_t close = get_rand(0, 2000);
        Point* p = point_create(i, open, high, low, close);

        index_insert(index, LINE1, p);
        //printf("Creating point: %3d\n", i);

        printf("data:index: %d -> %d\n", p->x, index_map_to_index(index, p->x));
    }
    index_print(index);

    return 0;
}
