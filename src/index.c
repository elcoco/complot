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
        (*b)->wend = (*b)->wstart + index->spread -1;

        (*b)->is_empty = true;


        // init line container
        (*b)->lines = (Line**)malloc(amount_lines*sizeof(Line*));

        // init line conainers
        Line** l = (*b)->lines;
        for (int i=0 ; i<amount_lines ; i++,l++) {
            *l = (Line*)malloc(sizeof(Line));
            (*l)->is_empty = true;
        }
    }

    return 1;
}

int32_t index_map_to_index(Index* index, int32_t x)
{
    /* Map data X value to an array index */
    return (x - index->dmin) / index->spread;
}

int8_t index_insert(Index* index, uint8_t lineid, Point* p)
{
    /* Insert point into index, create Bucket if necessary
     */

    // map data to array index
    int32_t i = index_map_to_index(index, p->x);

    if (p->x > index->dmax)
        index->dmax = p->x;

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

    Line* l = b->lines[lineid];
    line_add_point(l, p);

    b->is_empty = false;

    return 1;
}

int8_t index_get_grouped(Index* index, uint32_t gsize, uint32_t amount)
{
    // TODO return grouped bins, that can be drawn to display
}

void index_print(Index* index)
{
    for (int i=0 ; i<index->imax ; i++) {
        Bucket* b = index->buckets[i];
        Line* l = b->lines[0];

        if (b->is_empty)
            continue;

        printf("BUCKET: %d\n", i);
        printf("xmin-xmax: %d - %d\n", b->wstart, b->wend);
        printf("open:           %d\n", l->open);
        printf("close:          %d\n", l->close);
        printf("high:           %d\n", l->high);
        printf("low:            %d\n", l->low);
        printf("npoints:        %d\n\n", l->npoints);
    }
}

Point* point_create(int32_t x, int32_t open, int32_t high, int32_t low, int32_t close)
{
    printf("%d %d %d %d %d\n", x, open, high, low, close);
    Point* p = (Point*)malloc(sizeof(Point));
    p->x = x;
    p->open = open;
    p->high = high;
    p->low = low;
    p->close = close;
    return p;
}

int8_t line_add_point(Line* l, Point* p)
{
    l->npoints++;

    if (l->is_empty) {
        l->is_empty = false;
        l->open = p->open;
        l->close = p->close;
        l->high = p->high;
        l->low = p->low;
        return 1;
    }

    if (p->open < l->xmin) {
        l->open = p->open;
        l->xmin = p->x;
    }
    if (p->close > l->xmax) {
        l->close = p->close;
        l->xmax = p->x;
    }
    if (p->high > l->high)
        l->high = p->high;

    if (p->low < l->low)
        l->low = p->low;


    l->close = p->close;

    return 1;

}
