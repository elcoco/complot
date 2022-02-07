#include "index.h"

Index* index_create(int32_t grow_amount, int32_t spread, uint8_t nlines)
{
    Index* index = (Index*)malloc(sizeof(Index));
    index->bins = (Bin**)malloc(grow_amount*sizeof(Bin*));

    index->phead = (Point**)malloc(sizeof(Point*));
    index->ptail = (Point**)malloc(sizeof(Point*));
    *(index->phead) = NULL;
    *(index->ptail) = NULL;

    // data distance inbetween aray indices
    index->spread = spread;

    index->grow_amount = grow_amount;
    index->isize = grow_amount;
    index->nlines = nlines;

    index->ilast = 0;

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
    for (int i=0 ; i<index->isize ; i++,b++) {

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

void points_print(Point* p)
{
    uint32_t i = 0;

    while (p != NULL) {
        printf("[%d] %d\n", i, p->x);
        p = p->next;
        i++;
    }
}

void point_append(Point* p, Point** tail)
{
    /* Append point in linked list */
    Point* prev = *tail;
    *tail = p;
    prev->next = p;
}

void group_append(Group* g, Group** tail)
{
    /* Append point in linked list */
    Group* prev = *tail;
    *tail = g;
    prev->next = g;
}

int8_t index_insert(Index* index, uint8_t lineid, Point* p)
{
    /* Insert point into index, create Bin if necessary
     */
    // first insert determines the index start
    if (! index->is_initialized)
        index_build(index, p->x);

    // connect point in linked list
    if (*(index->phead) == NULL) {
        *(index->phead) = p;
        *(index->ptail) = p;
    } else {
        point_append(p, index->ptail);
    }

    // map data to array index
    int32_t i = index_map_to_index(index, p->x);

    // check if index is too small for data
    if (i > index->isize-1) {
        printf("Out of bounds, grow to right!: %d > %d \n", i, index->isize-1);
        return -1;
    } else if (i < 0) {
        printf("Out of bounds, grow to left!: %d < 0 \n", i);
        return -1;
    }

    // set data limit if bigger X
    if (p->x > index->dmax)
        index->dmax = p->x;

    // keep track of last non-empty bin.
    if (i > index->ilast)
        index->ilast = i;

    // at this point, we know bin exists
    Bin* b  = index->bins[i];
    b->is_empty = false;

    Line* l = b->lines[lineid];
    line_add_point(l, p);

    return 1;
}

Group* group_create(Index* index, int32_t gstart, uint32_t gsize)
{
    Group* g = (Group*)malloc(sizeof(Group));
    g->wstart = index_map_to_x(index, gstart);
    g->wend   = index_map_to_x(index, gstart + gsize-1);
    g->is_empty = true;
    return g;
}

int32_t index_get_gstart(Index* index, uint32_t gsize, uint32_t amount)
{
    /* calculate bin index of first group */
    int32_t last_group_i  = index->ilast - (index->ilast % gsize);
    int32_t first_group_i = last_group_i - (amount * gsize);
    first_group_i += gsize;
    return first_group_i;
}

Group* index_get_grouped(Index* index, uint8_t lineid, uint32_t gsize, uint32_t amount)
{
    /* Create groups, get data from last data */
    Bin** bins = index->bins;

    // calculate at which bin index the first group starts
    int32_t gstart = index_get_gstart(index, gsize, amount);

    // setup linked list
    Group** ghead = (Group**)malloc(sizeof(Group*));
    Group** gtail = (Group**)malloc(sizeof(Group*));
    *gtail = NULL;
    *ghead = NULL;

    for (int32_t gindex=0 ; gindex<amount ; gstart+=gsize, gindex++) {

        Group* g = group_create(index, gsize, amount);

        // connect linked list
        if (*ghead == NULL) {
            *ghead = g;
            *gtail = g;
        } else {
            group_append(g, gtail);
        }

        // if group index is below 0 this means that there is no data for this group
        if (gstart < 0)
            continue;

        for (int i=0 ; i<gsize ; i++) {
            // trying to access a bin outside index boundaries, should not be reachable
            // because index should auto extend to fit data
            if (gstart+i > index->isize-1)
                return NULL;

            Bin* b = bins[gstart+i];
            Line* l = b->lines[lineid];

            if (l->is_empty)
                continue;

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
    }
    free(gtail);
    return *ghead;
}

void groups_print(Group* g)
{
    int32_t c = 0;
    printf("\n%5s %6s %5s %5s %5s %5s %5s\n", "INDEX", "WSTART", "WEND",  "OPEN", "HIGH", "LOW", "CLOSE");

    while (g != NULL) {
        if (g->is_empty) {
            printf("%5d %6s %5s %5s %5s %5s %5s\n", c, "empty", "empty", "empty", "empty", "empty", "empty");
            continue;
        }
        printf("%5d %6d %5d %5d %5d %5d %5d\n", c, g->wstart, g->wend, g->open, g->high, g->low, g->close);
        g = g->next;
        c++;
    }
}

void index_print(Index* index)
{
    for (int i=0 ; i<index->isize ; i++) {
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
