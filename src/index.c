#include "index.h"


Index* index_init(uint8_t nlines)
{
    Index* index = malloc(sizeof(Index));

    index->phead = malloc(sizeof(Point*));
    index->ptail = malloc(sizeof(Point*));
    *(index->phead) = NULL;
    *(index->ptail) = NULL;

    // data distance inbetween aray indices
    index->spread = -1;
    index->npoints = 0;

    index->grow_amount = INDEX_DEFAULT_GROW_AMOUNT;
    index->isize = INDEX_DEFAULT_GROW_AMOUNT;
    index->nlines = nlines;

    index->bins = malloc(index->isize * sizeof(Bin*));
    debug("created index with %d * %d = %d bytes\n", index->isize, sizeof(Bin*), index->isize*sizeof(Bin*));

    // initialize all lineid structs with NULL pointers
    index->lineids = malloc(index->nlines * sizeof(LineID*));
    for (int i=0 ; i<index->nlines ; i++)
        index->lineids[i] = NULL;

    index->ilast = 0;
    index->has_new_data = false;
    index->is_initialized = false;

    return index;
}

void index_destroy(Index* index)
{
    if (index->is_initialized) {
        for (int i=0 ; i<index->isize ; i++)
            bin_destroy(*(index->bins+i), index);
    }

    // NOTE for some reason valgrind complains about lost bytes although they are freed here
    free(index->bins);
    debug("Destroyed index of %d * %d = %d bytes\n", index->isize, sizeof(Bin*), index->isize*sizeof(Bin*));

    free(index->lineids);

    Point* p = *(index->phead);
    Point* pprev;
    while (p != NULL) {
        pprev = p;
        p = p->next;
        free(pprev);
    }

    free(index->phead);
    free(index->ptail);
    free(index);
}

int8_t index_build(Index* index)
{
    /* (re)build index, create bins */
    index->is_initialized = true;

    Bin** b = index->bins;
    for (int i=0 ; i<index->isize ; i++,b++)
        *b = bin_create(index, i);
    return 1;
}

int8_t index_extend(Index* index)
{
    /* Grow bins array */
    //printf("Extending index\n");
    index->isize += index->grow_amount;

    if ((index->bins = realloc(index->bins, index->isize * sizeof(Bin*))) == NULL) {
        printf("Error while extending index\n");
        return -1;
    }

    for (int i=index->isize-index->grow_amount ; i<index->isize ; i++)
        *(index->bins+i) = bin_create(index, i);

    debug("Extended index to %d * %d = %d bytes\n", index->isize, sizeof(Bin*), index->isize*sizeof(Bin*));

    return 1;
}

Point* index_get_last_point(Index* index, LineID* lineid)
{
    /* Get last datapoint for a specific lineid */
    Point* p = *(index->ptail);
    while (p != NULL) {
        if (p->lineid->id == lineid->id)
            return p;
        p = p->prev;
    }
    return NULL;
}

int32_t index_map_to_index(Index* index, double x)
{
    /* Map data X value to an array index */
    return (x - index->xmin) / index->spread;
}

double index_map_to_x(Index* index, int32_t i)
{
    /* Map index to x value */
    return (i * index->spread) + index->xmin;
}

void index_set_data_limits(Index* index, Point* p)
{
    // find data limits and update index->dmin and index->dmax
    if (p->lineid->ltype == LTYPE_OHLC) {
        if (p->high > index->dmax)
            index->dmax = p->high;
        if (p->low < index->dmin)
            index->dmin = p->low;
    }
    else if (p->lineid->ltype == LTYPE_LINE) {
        if (p->y > index->dmax)
            index->dmax = p->y;
        if (p->y < index->dmin)
            index->dmin = p->y;
    }
}

void index_reindex(Index* index)
{
    /* After updating spread, recreate all bins from points linked list. */
    debug("Reindexing index with %d points, spread: %f\n", index->npoints, index->spread);

    if (index->is_initialized) {
        // destroy old bins
        for (int i=0 ; i<index->isize ; i++)
            bin_destroy(*(index->bins+i), index);
        free(index->bins);
    }

    // create new bins
    index->bins = (Bin**)malloc(INDEX_DEFAULT_GROW_AMOUNT*sizeof(Bin*));

    // trigger bin recreation when reinserting points
    index->is_initialized = false;

    Point* p = *index->phead;
    while (p != NULL) {
        index_insert(index, p);
        p = p->next;
    }
}

int8_t index_insert(Index* index, Point* p)
{
    /* Insert point into index, extend bins array if necessary */
    if (index->spread < 0)
        return 0;

    // first insert determines the index start x
    if (! index->is_initialized) {
        if (p->lineid->ltype == LTYPE_OHLC) {
            index->dmin = p->low;
            index->dmax = p->high;
        } else if (p->lineid->ltype == LTYPE_LINE) {
            index->dmin = p->y;
            index->dmax = p->y;
        }
        index_build(index);
        index->xmin = p->x;
    }

    // map x to index->bins array index
    int32_t ix = index_map_to_index(index, p->x);

    // check if index is too smoll to hold data
    if (ix > index->isize-1) {
        if (index_extend(index) < 0) {
            debug("Failed to extend index!\n");
            return -1;
        }
    } else if (ix < 0) {
        printf("Out of bounds, grow to left not implemented...\n");
        printf("ix   = %d\n", ix);
        printf("x    = %f\n", p->x);
        printf("y    = %f\n", p->y);
        printf("open = %f\n", p->open);
        printf("xmin = %f\n", index->xmin);
        printf("spread = %f\n", index->spread);
        return -1;
    }

    // update data limits
    index_set_data_limits(index, p);

    // keep track of last non-empty bin index
    if (ix > index->ilast)
        index->ilast = ix;

    // at this point, we know bin exists
    Bin* b  = index->bins[ix];
    b->is_empty = false;

    line_add_point(b, p);

    index->has_new_data = true;

    return 1;
}

int32_t index_get_gstart(Index* index, uint32_t gsize, uint32_t amount)
{
    /* calculate bin index of first group */
    int32_t last_group_i  = index->ilast - (index->ilast % gsize);
    int32_t first_group_i = last_group_i - (amount * gsize);
    first_group_i += gsize;
    //printf("first group i: %d\n", first_group_i);
    //printf("last group i:  %d\n", last_group_i);
    return first_group_i;
}

Bin* bin_create(Index* index, uint32_t i)
{
    /* Create and allocate new Bin struct that has a fixed X window.
     * It holds points that fall within this window.
     * This makes taking slices/grouping of data super fast.  */
    Bin* b = malloc(sizeof(Bin));

    // set x window
    b->wstart = (i*index->spread) + index->dmin;
    b->wend = b->wstart + index->spread;

    b->is_empty = true;

    // init line container
    b->lcontainers = malloc(index->nlines*sizeof(void*));

    // init void pointer array that will be casted to the appropriate type later
    void** vlb = b->lcontainers;
    for (int i=0 ; i<index->nlines ; i++,vlb++)
        *vlb = NULL;
    return b;
}

void bin_destroy(Bin* b, Index* index)
{
    void** lc = b->lcontainers;
    for (int i=0 ; i<index->nlines ; i++, lc++)
        free(*lc);
    free(b->lcontainers);
    free(b);
}

void point_print(Point* p)
{
    debug("%f => [%f, %f, %f, %f]\n", p->x, p->open, p->high, p->low, p->close);
}

Point* point_init(Index* index)
{
    /* Create a new point struct, don't call directly, use point_create_point() or point_create_cspoint() */
    Point* p = malloc(sizeof(Point));
    p->prev = NULL;
    p->next = NULL;

    index->npoints++;

    // connect point in linked list, this is used to regenerate index when spread changes
    if (*(index->phead) == NULL) {
        *(index->phead) = p;
        *(index->ptail) = p;
    } else {
        point_append(p, index->ptail);
    }

    return p;
}

Point* point_create_cspoint(Index* index, LineID* lineid, double x, double open, double high, double low, double close)
{
    assert(x && open && high && low && close);

    Point* p = point_init(index);
    p->x = x;
    p->open = open;
    p->high = high;
    p->low = low;
    p->close = close;
    p->lineid = lineid;


    if (index->lineids[lineid->id] == NULL)
        index->lineids[lineid->id] = lineid;

    // about to insert second point, we can calculate the spread now and reindex
    if (index->npoints > 1 && index->spread == -1) {
        if (p->x - (*index->phead)->x > 0) {
            index->spread = p->x - (*index->phead)->x;
            index_reindex(index);
        }
    }
    index_insert(index, p);
    return p;
}

Point* point_create_point(Index* index, LineID* lineid, double x, double y)
{
    assert(x && y && lineid);

    Point* p = point_init(index);
    p->x = x;
    p->y = y;
    p->lineid = lineid;

    if (index->lineids[lineid->id] == NULL)
        index->lineids[lineid->id] = lineid;

    // about to insert second point, we can calculate the spread now and reindex
    if (index->npoints > 1 && index->spread == -1) {
        if (p->x - (*index->phead)->x > 0) {
            index->spread = p->x - (*index->phead)->x;
            index_reindex(index);
        }
    }
    index_insert(index, p);
    return p;
}

void points_print(Point* p)
{
    uint32_t i = 0;

    while (p != NULL) {
        point_print(p);
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
    p->prev = prev;
    p->next = NULL;
}

void line_add_ohlc_point(OHLCContainer** lc, Point* p)
{
    if (*lc == NULL) {
        *lc = malloc(sizeof(OHLCContainer));
        (*lc)->is_empty = true;
    }

    if ((*lc)->is_empty) {
        (*lc)->is_empty = false;
        (*lc)->open = p->open;
        (*lc)->close = p->close;
        (*lc)->high = p->high;
        (*lc)->low = p->low;
        (*lc)->xmin = p->x;
        (*lc)->xmax = p->x;
        (*lc)->npoints = 1;
        return;
    }

    (*lc)->npoints++;

    // update line x lower limits if p->x is older than oldest point in line
    if (p->x < (*lc)->xmin) {
        (*lc)->open = p->open;
        (*lc)->xmin = p->x;
    }
    // update line x upper limits if p->x is newer than oldest point in line
    if (p->x > (*lc)->xmax) {
        (*lc)->close = p->close;
        (*lc)->xmax = p->x;
    }
    if (p->high > (*lc)->high)
        (*lc)->high = p->high;

    if (p->low < (*lc)->low)
        (*lc)->low = p->low;


    // NOTE: this keeps only last data in this bin ////////
    // Market data will come in for unfinished candle sticks.
    // We are only interested in the last data for this time period.
    // If we keep all the datapoints we get weird results for high and lows.
    (*lc)->open = p->open;
    (*lc)->close = p->close;
    //(*lc)->high = p->high;
    //(*lc)->low = p->low;
    //(*lc)->xmin = p->x;
    //(*lc)->xmax = p->x;
    (*lc)->npoints = 1;
    return;
    ///////////////////////////////////////////
}

void line_add_line_point(LineContainer** lc, Point* p)
{
    // initialize linebin if not yet created
    if (*lc == NULL) {
        *lc = malloc(sizeof(LineContainer));
        (*lc)->is_empty = true;
    }

    if ((*lc)->is_empty) {
        (*lc)->is_empty = false;
        (*lc)->y = p->y;
        (*lc)->npoints = 1;
        return;
    }

    (*lc)->y = get_avg((*lc)->y, (*lc)->npoints, p->y);

    (*lc)->npoints++;

    // TODO we should calculate an average here, for now we just take the highest y
    //if (p->y > (*lc)->y)
    //    (*lc)->y = p->y;
}

int8_t line_add_point(Bin* b, Point* p)
{
    // Cast container to right type and create pointer to it.
    // Then add point to it
    if (p->lineid->ltype == LTYPE_OHLC) {
        OHLCContainer** lc = (OHLCContainer**)(&(b->lcontainers[p->lineid->id]));
        line_add_ohlc_point(lc, p);
    }
    else if (p->lineid->ltype == LTYPE_LINE) {
        LineContainer** lc = (LineContainer**)(&(b->lcontainers[p->lineid->id]));
        line_add_line_point(lc, p);
    }

    return 1;
}

