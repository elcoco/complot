#include "index.h"

Index* index_create(int32_t grow_amount, int32_t spread, uint8_t nlines)
{
    Index* index = (Index*)malloc(sizeof(Index));
    index->bins = (Bin**)malloc(grow_amount*sizeof(Bin*));

    index->phead = NULL;

    // data distance inbetween aray indices
    index->spread = spread;

    index->grow_amount = grow_amount;
    index->imax = grow_amount;
    index->nlines = nlines;

    index->last_data = 0;

    index->dmin = 0;
    index->dmax = 0;

    index->is_initialized = false;
    return index;
}

int8_t index_build(Index* index, int32_t dmin)
{
    /* (re)build index, create bins
     * */
    printf("Building index.... dmin=%d\n", dmin);

    index->dmin = dmin;
    index->is_initialized = true;

    // init bins
    Bin** b = index->bins;
    for (int i=0 ; i<index->imax ; i++,b++) {

        *b = (Bin*)malloc(sizeof(Bin));

        (*b)->wstart = (i*index->spread) + dmin;
        (*b)->wend = (*b)->wstart + index->spread -1;

        (*b)->is_empty = true;


        // init line container
        (*b)->lines = (Line**)malloc(index->nlines*sizeof(Line*));

        // init line conainers
        Line** l = (*b)->lines;
        for (int i=0 ; i<index->nlines ; i++,l++) {
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

int32_t index_map_to_x(Index* index, int32_t i)
{
    /* Map index to x value */
    return (i * index->spread) + index->dmin;
}

int8_t index_insert(Index* index, uint8_t lineid, Point* p)
{
    /* Insert point into index, create Bin if necessary
     */
    // first insert determines the index start
    if (! index->is_initialized)
        index_build(index, p->x);

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

    // keep track of last non-empty bin.
    if (i > index->last_data)
        index->last_data = i;

    // at this point, we know bin exists
    Bin* b  = index->bins[i];
    Line* l = b->lines[lineid];

    line_add_point(l, p);

    b->is_empty = false;

    return 1;
}

Group** index_get_grouped(Index* index, uint8_t lineid, uint32_t gsize, uint32_t amount)
{
    /* Create groups, get data from last data */
    Group** groups = (Group**)malloc(amount*sizeof(Group*));
    Bin** bins = index->bins;

    // calculate at which bin the last group starts
    int32_t last_group_i  = index->last_data - (index->last_data % gsize);
    int32_t first_group_i = last_group_i - (amount * gsize);


    // when there is an unfinished group at the end, make sure the amount is correct
    //if (index->last_data > index->last_data - (index->last_data % gsize)) {
    //    printf(">>>>>>>> correcting\n");
    //    first_group_i += gsize;
    //}
    first_group_i += gsize;

    printf("first group i=%d\n", first_group_i);
    printf("last group  i=%d, start: %d\n", last_group_i, bins[last_group_i]->wstart);

    int32_t gstart = first_group_i;

    for (int32_t gindex=0 ; gindex<amount ; gstart+=gsize, gindex++) {
        printf("gstart=%d\n", gstart);

        Group* g = (Group*)malloc(sizeof(Group));
        // prefill group dimensions
        g->wstart = index_map_to_x(index, gstart);
        g->wend   = index_map_to_x(index, gstart + gsize);
        g->is_empty = true;

        // if group index is below 0 this means that there is no data for this group
        if (gstart < 0) {
            groups[gindex] = g;
            continue;
        }

        for (int i=0 ; i<gsize ; i++) {
            Bin* b = bins[gstart+i];
            
            //if (b->is_empty) {
            //    continue;
            //}

            Line* l = b->lines[lineid];

            if (l->is_empty) {
                printf("line is empty!\n");
                continue;
            }
            if (g->is_empty) {
                g->is_empty = false;
                g->open = l->open;
                g->high = l->high;
                g->low = l->low;
                g->close = l->close;
                continue;

            } else {
                g->close = l->close;
            }

            if (l->high > g->high)
                g->high = l->high;

            if (l->low < g->low)
                g->low = l->low;
        }
        groups[gindex] = g;
    }
    return groups;
}

void groups_print(Group** groups, uint32_t amount)
{
    printf("\n%5s %6s %5s %5s %5s %5s %5s\n", "INDEX", "WSTART", "WEND",  "OPEN", "HIGH", "LOW", "CLOSE");

    for (int i=0 ; i<amount ; i++) {
        Group* g = *(groups+i);

        if (g->is_empty) {
            printf("%5d %6s %5s %5s %5s %5s %5s\n", i, "empty", "empty", "empty", "empty", "empty", "empty");
            continue;
        }
        printf("%5d %6d %5d %5d %5d %5d %5d\n", i, g->wstart, g->wend, g->open, g->high, g->low, g->close);
    }
}

void index_print(Index* index)
{
    for (int i=0 ; i<index->imax ; i++) {
        Bin* b = index->bins[i];
        Line* l = b->lines[0];

        if (b->is_empty)
            continue;

        printf("BIN %3d: %d:%d %d => %d %d %d %d\n", i,
                                              b->wstart,
                                              b->wend,
                                              l->npoints,
                                              l->open,
                                              l->high,
                                              l->low,
                                              l->close);
    }
}

Group* group_create()
{
    Group* g = (Group*)malloc(sizeof(Group));
    g->is_empty = true;
    return g;
}

Point* point_create(int32_t x, int32_t open, int32_t high, int32_t low, int32_t close)
{
    printf("NEW POINT: %3d = %d %d %d %d\n", x, open, high, low, close);
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

        l->xmin = p->x;
        l->xmax = p->x;
        return 1;
    }
    if (p->x < l->xmin) {
        l->open = p->open;
        l->xmin = p->x;
    }
    if (p->x > l->xmax) {
        l->close = p->close;
        l->xmax = p->x;
    }
    if (p->high > l->high)
        l->high = p->high;

    if (p->low < l->low)
        l->low = p->low;

    return 1;

}
