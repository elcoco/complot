#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "index.h"

enum lines {
    LINE1
} lines;

int main(int argc, char **argkv) {

    Index* index = index_create();

    // Point* point_create(int32_t x, int32_t open, int32_t high, int32_t low, int32_t close);
    Point* p = point_create(100, 20, 30, 10, 0);




    return 0;
}
