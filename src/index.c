#include "index.h"

Index* index_create(int32_t grow_amount)
{
    Index* index = (Index*)malloc(sizeof(Index));
    index->bins = (Bin**)malloc(grow_amount*sizeof(Bin*));


    index->phead = NULL;
    index->spread = 0;

    index->grow_amount = grow_amount;
    index->imax = grow_amount;
    index->last_data = 0;

    index->dmin = 0;
    index->dmax = 0;
    return index;
}

int8_t index_build(Index* index, int32_t dmin, uint32_t spread, int8_t amount_lines)
{
    /* (re)build index, create bins
     * */

    index->dmin = dmin;
    index->spread = spread;

    // init bins
    Bin** b = index->bins;
    for (int i=0 ; i<index->imax ; i++,b++) {
        *b = (Bin*)malloc(sizeof(Bin));
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
    /* Insert point into index, create Bin if necessary
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

    // keep track of last non-empty bin.
    if (i > index->last_data)
        index->last_data = i;

    // at this point, we know bin exists
    Bin* b = index->bins[i];

    Line* l = b->lines[lineid];
    line_add_point(l, p);

    b->is_empty = false;

    return 1;
}

Group** index_get_grouped_bak(Index* index, uint8_t lineid, uint32_t gsize, uint32_t amount)
{
    /* Create groups, get data from start of index */
    Group** groups = (Group**)malloc(amount*sizeof(Group*));
    Bin** b = index->bins;

    if (amount*gsize > index->imax) {
        printf("Out of bound, can't get groups!\n");
        return NULL;
    }

    int gstart; // index in bins where group starts
    int gindex; // group number

    for (gstart=0, gindex=0 ; gindex<amount ; gstart+=gsize, gindex++) {
        Group* g = (Group*)malloc(sizeof(Group));
        g->is_empty = true;

        for (int i=0 ; i<gsize ; i++, b++) {
            
            if ((*b)->is_empty) {
                continue;
            }

            Line* l = (*b)->lines[lineid];

            if (i == 0) {
                g->is_empty = false;
                g->open = l->open;
                g->high = l->high;
                g->low = l->low;
                g->close = l->close;
                continue;
            }
            if (i == gsize-1)
                g->close = l->low;

            if (l->high > g->high)
                g->high = l->high;

            if (l->low < g->low)
                g->low = l->low;
        }
        *(groups+gindex) = g;
    }
    return groups;
}

Group** index_get_grouped(Index* index, uint8_t lineid, uint32_t gsize, uint32_t amount)
{
    /* Create groups, get data from last data */
    Group** groups = (Group**)malloc(amount*sizeof(Group*));
    Bin** b = index->bins;


    // calculate at which bin the last group starts
    int32_t last_group_i  = index->last_data - (index->last_data % gsize);
    int32_t first_group_i = last_group_i - (amount * gsize);

    // when there is an unfinished group at the end, make sure the amount is correct
    if (index->last_data > index->last_data - (index->last_data % gsize))
        first_group_i += gsize;

    int32_t gstart = first_group_i;

    for (int32_t gindex=0 ; gindex<amount ; gstart+=gsize, gindex++) {

        Group* g = (Group*)malloc(sizeof(Group));
        g->is_empty = true;
        //printf("%d - %d\n", gstart, gstart+gsize);

        if (gstart < 0) {
            *(groups+gindex) = g;
            continue;
        }

        for (int i=0 ; i<gsize ; i++) {
            Bin* bptr = *(b+gstart+i);
            
            if (bptr->is_empty) {
                printf("empty\n");
                continue;
            }

            Line* l = bptr->lines[lineid];

            if (i == 0) {
                g->is_empty = false;
                g->open = l->open;
                g->high = l->high;
                g->low = l->low;
                g->close = l->close;
                continue;
            }
            if (i == gsize-1)
                g->close = l->low;

            if (l->high > g->high)
                g->high = l->high;

            if (l->low < g->low)
                g->low = l->low;
        }
        *(groups+gindex) = g;
    }
    return groups;
}

void groups_print(Group** groups, uint32_t amount)
{
    printf("%5s %5s %5s %5s %5s\n", "INDEX", "OPEN", "HIGH", "LOW", "CLOSE");

    for (int i=0 ; i<amount ; i++) {
        Group* g = *(groups+i);

        if (g->is_empty) {
            printf("%5d %5s %5s %5s %5s\n", i, "empty", "empty", "empty", "empty");
            continue;
        }
        printf("%5d %5d %5d %5d %5d\n", i, g->open, g->high, g->low, g->close);
    }
}

void index_print(Index* index)
{
    for (int i=0 ; i<index->imax ; i++) {
        Bin* b = index->bins[i];
        Line* l = b->lines[0];

        if (b->is_empty)
            continue;

        printf("%d [%d-%d] %d => %d %d %d %d\n", i,
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
