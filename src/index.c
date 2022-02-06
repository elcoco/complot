#include "index.h"

Index* index_create() {
    Index* index = (Index*)malloc(sizeof(Index));
    return index;
}

Point* point_create(int32_t x, int32_t open, int32_t high, int32_t low, int32_t close) {
    Point* p = (Point*)malloc(sizeof(Point));
    p->x = x;
    p->open = open;
    p->high = high;
    p->low = low;
    p->close = close;
    return p;
}
