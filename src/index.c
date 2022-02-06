#include "index.h"

Index* index_create(int32_t grow_amount)
{
    Index* index = (Index*)malloc(sizeof(Index));
    index->buckets = (Bucket**)malloc(grow_amount*sizeof(Bucket*));
    index->phead = NULL;
    index->start_key = 0;
    index->spread = 0;
    return index;
}

int8_t index_build(Index* index, int32_t start_key, uint32_t spread)
{
    /* (re)build index, create groups */
    index->start_key = start_key;
    index->spread = spread;

    //Point* p = *(index->phead);
    //while (p != NULL) {
    //    // do something
    //}
    return 1;
}

int8_t index_insert(Index* index, uint8_t line_id, Point* point)
{
    /* Insert point into index, create Group if necessary */
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
