#include "index.h"

Index* index_create(int32_t grow_amount)
{
    Index* index = (Index*)malloc(sizeof(Index));
    index->buckets = (Bucket**)malloc(grow_amount*sizeof(Bucket*));


    index->phead = NULL;
    index->spread = 0;

    index->grow_amount = grow_amount;
    index->imax = grow_amount;

    index->dmin = 0;
    index->dmax = 0;
    return index;
}

int8_t index_build(Index* index, int32_t dmin, uint32_t spread, int8_t amount_lines)
{
    /* (re)build index, create buckets
     * */

    index->dmin = dmin;
    index->spread = spread;

    // init buckets
    Bucket** b = index->buckets;
    for (int i=0 ; i<index->imax ; i++,b++) {
        *b = (Bucket*)malloc(sizeof(Bucket));
        (*b)->wstart = i*index->spread;
        (*b)->wend = (*b)->wstart + index->spread;

        (*b)->open = (float*)malloc(sizeof(float));
        (*b)->close = (float*)malloc(sizeof(float));
        (*b)->high = (float*)malloc(sizeof(float));
        (*b)->low = (float*)malloc(sizeof(float));
    }

    return 1;
}

int32_t index_map_to_index(Index* index, int32_t x)
{
    /* Map data X value to an array index */
    return (x - index->dmin) / index->spread;
}

int8_t index_insert(Index* index, uint8_t lineid, Point* point)
{
    /* Insert point into index, create Bucket if necessary
     */

    // map data to array index
    int32_t i = index_map_to_index(index, point->x);

    if (point->x > index->dmax)
        index->dmax = point->x;

    if (i > index->imax) {
        printf("Out of bounds, grow to right!: %d > %d \n", i, index->imax);
        return -1;
    } else if (i < 0) {
        printf("Out of bounds, grow to left!: %d < 0 \n", i);
        return -1;
    }

    // at this point, we know bucket exists
    Bucket* b = index->buckets[i];
    printf("min/max %d, %d\n", b->wstart, b->wend);


    return 1;
}

Point* point_create(int32_t x, int32_t open, int32_t high, int32_t low, int32_t close)
{
    Point* p = (Point*)malloc(sizeof(Point));
    p->x = x;
    p->open = open;
    p->high = high;
    p->low = low;
    p->close = close;
    return p;
}

int8_t bucket_add_point(Bucket* b, int8_t lineid, Point* p)
{
}
