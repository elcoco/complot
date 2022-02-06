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


enum lines {
    LINE1,
    LINE2
} lines;

int main(int argc, char **argkv)
{

    Index* index = index_create(INDEX_GROW_AMOUNT);
    index_build(index, INDEX_START_KEY, INDEX_SPREAD);

    for (int8_t i=0 ; i<50 ; i++) {
        Point* p = point_create(i+100, 20, 30, 10, 0);
        index_insert(index, LINE1, p);
        printf("Creating point: %3d\n", i);
    }

    return 0;
}
