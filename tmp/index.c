#include "index.h"


Index* index_init(uint8_t nlines)
{
    Index* index = (Index*)malloc(sizeof(Index));
    index->bins = (Bin**)malloc(INDEX_DEFAULT_GROW_AMOUNT*sizeof(Bin*));

    index->phead = (Point**)malloc(sizeof(Point*));
    index->ptail = (Point**)malloc(sizeof(Point*));
    *(index->phead) = NULL;
    *(index->ptail) = NULL;

    // data distance inbetween aray indices
    index->spread = INDEX_DEFAULT_SPREAD;

    index->grow_amount = INDEX_DEFAULT_GROW_AMOUNT;
    index->isize = INDEX_DEFAULT_GROW_AMOUNT;
    index->nlines = nlines;

    index->ilast = 0;
    index->has_new_data = false;

    //index->dmin = 0;
    //index->dmax = 0;

    index->is_initialized = false;

    return index;
}

void index_destroy(Index* index)
{
    for (int i=0 ; i<index->isize ; i++)
        bin_destroy(*(index->bins+i), index);
    free(index->bins);

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
    /* (re)build index, create bins
     * */
    //printf("Building index....\n");

    index->is_initialized = true;

    // init bins
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

    return 1;
}

Point* index_get_last_point(Index* index, uint32_t lineid)
{
    /* Get last datapoint for a specific lineid */
    Point* p = *(index->ptail);
    while (p != NULL) {
        if (p->lineid == lineid)
            return p;
        p = p->prev;
    }
    return NULL;
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
    b->lines = (LineBin**)malloc(index->nlines*sizeof(LineBin*));

    // init line conainers
    LineBin** lb = b->lines;
    for (int i=0 ; i<index->nlines ; i++,lb++) {
        *lb = (LineBin*)malloc(sizeof(LineBin));
        (*lb)->is_empty = true;
    }
    return b;
}

void bin_destroy(Bin* b, Index* index)
{
    LineBin** lb = b->lines;
    for (int i=0 ; i<index->nlines ; i++, lb++)
        free(*lb);
    free(b->lines);
    free(b);
}

int32_t index_map_to_index(Index* index, double x)
{
    /* Map data X value to an array index */
    //printf("(%f - %f) / %d = %d\n", x, index->xmin, index->spread, (x - index->xmin) / index->spread);
    return (x - index->xmin) / index->spread;
}

double index_map_to_x(Index* index, int32_t i)
{
    /* Map index to x value */
    return (i * index->spread) + index->xmin;
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
}

void group_append(Group* g, Group** tail)
{
    /* Append point in linked list */
    Group* prev = *tail;
    *tail = g;
    prev->next = g;
    g->prev = prev;
}

void index_set_data_limits(Index* index, Point* p)
{
    // find data limits and update index->dmin and index->dmax
    if (p->high > index->dmax)
        index->dmax = p->high;
    if (p->low < index->dmin)
        index->dmin = p->low;
}

void index_reindex(Index* index)
{
    /* After updating spread, recreate all bins from points linked list. */
    //printf("Reindexing index with spread: %f\n", index->spread);

    // destroy old bins
    for (int i=0 ; i<index->isize ; i++)
        bin_destroy(*(index->bins+i), index);
    free(index->bins);

    // create new bins
    index->bins = (Bin**)malloc(INDEX_DEFAULT_GROW_AMOUNT*sizeof(Bin*));
    index->is_initialized = false;

    Point* p = *index->phead;
    while (p != NULL) {
        index_insert(index, p);
        p = p->next;
    }
}

void point_print(Point* p)
{
    printf("%f => [%f, %f, %f, %f]\n", p->x, p->open, p->high, p->low, p->close);
}

Point* point_create(Index* index, uint32_t lineid, double x, double open, double high, double low, double close)
{
    Point* p = (Point*)malloc(sizeof(Point));
    p->x = x;
    p->open = open;
    p->high = high;
    p->low = low;
    p->close = close;
    p->lineid = lineid;

    index->npoints++;

    // connect point in linked list, this is used to regenerate index when spread changes
    if (*(index->phead) == NULL) {
        *(index->phead) = p;
        *(index->ptail) = p;
    } else {
        point_append(p, index->ptail);
    }

    // about to insert second point, we can calculate the spread now and reindex
    if (index->npoints == 2) {
        index->spread = p->x - (*index->phead)->x;
        index_reindex(index);
    }

    index_insert(index, p);
    return p;
}

int8_t index_insert(Index* index, Point* p)
{
    /* Insert point into index, extend bins array if necessary */

    // first insert determines the index start x
    if (! index->is_initialized) {
        index->dmin = p->low;
        index->dmax = p->high;
        index_build(index);
        index->xmin = p->x;
    }

    // map x to index->bins array index
    int32_t ix = index_map_to_index(index, p->x);

    // check if index is too smoll to hold data
    if (ix > index->isize-1) {
        if (index_extend(index) < 0)
            return -1;

    } else if (ix < 0) {
        printf("Out of bounds, grow to left not implemented... %d, %f, %f\n", ix, p->x, p->open);
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

    LineBin* lb = b->lines[p->lineid];
    line_add_point(lb, p);

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

Group* group_create(Index* index, int32_t gstart, uint32_t gsize, Group** gtail)
{
    Group* g = (Group*)malloc(sizeof(Group));
    g->wstart = index_map_to_x(index, gstart);
    g->wend   = index_map_to_x(index, gstart + gsize);
    g->is_empty = true;
    g->next = NULL;

    // Set unique constant id for this group.
    // This is used to keep x tickers in the right spot.
    g->id = gstart / gsize;

    // connect new group to linked list
    if (*gtail != NULL)
        group_append(g, gtail);

    return g;
}

Groups* index_get_grouped(Index* index, uint32_t lineid, uint32_t gsize, uint32_t amount, int32_t x_offset, int32_t y_offset)
{
    /* Create groups, get data from last data */
    // calculate at which bin index the first group starts
    int32_t gstart = index_get_gstart(index, gsize, amount) + (x_offset*gsize);

    Bin** bins = index->bins;
    Groups* groups = groups_init(index, lineid);

    // setup linked list
    Group* htmp = NULL;
    Group* ttmp = NULL;
    Group** ghead = &htmp;
    Group** gtail = &ttmp;

    for (int32_t gindex=0 ; gindex<amount ; gindex++, gstart+=gsize) {
        Group* g = group_create(index, gstart, gsize, gtail);

        if (*ghead == NULL) {
            *ghead = g;
            *gtail = g;
        }

        // set OHLC values from line in group
        for (int i=0 ; i<gsize ; i++) {

            // Check if we're trying to access a group beyond index boundaries
            if (gstart < 0 || gstart+i >= index->isize-1)
                break;

            Bin* b = bins[gstart+i];
            LineBin* lb = b->lines[lineid];

            if (lb->is_empty)
                continue;

            // set initial OHLC data from line in empty group
            if (g->is_empty) {
                g->is_empty = false;
                g->open  = lb->open;
                g->high  = lb->high;
                g->low   = lb->low;
                g->close = lb->close;

            } else {
                g->close = lb->close;

                if (lb->high > g->high)
                    g->high = lb->high;
                if (lb->low < g->low)
                    g->low = lb->low;
            }
        }
        groups_update_limits(groups, g);
    }
    groups->group = *ghead;
    return groups;
}

void groups_update_limits(Groups* groups, Group* g)
{
    /* Update Groups data limits from Group struct */

    // set initial data dimensions in groups container
    if (groups->is_empty && !g->is_empty) {
        groups->is_empty = false;
        groups->gmin = g->low;
        groups->gmax = g->high;
    }
    // set data dimensions in groups container
    else if (!groups->is_empty && !g->is_empty) {
        if (g->high > groups->gmax)
            groups->gmax = g->high;
        if (g->low < groups->gmin)
            groups->gmin = g->low;
    }
}

Groups* groups_init(Index* index, uint32_t lineid)
{
    // create groups container
    Groups* groups = (Groups*)malloc(sizeof(Groups));
    groups->is_empty = true;
    groups->dmin = index->dmin;
    groups->dmax = index->dmax;
    groups->plast = index_get_last_point(index, lineid);
    return groups;
}

void groups_destroy(Groups* groups)
{
    Group* g = groups->group;
    Group* prev;
    while (g != NULL) {
        prev = g;
        g = g->next;
        free(prev);
    }
    free(groups);
}

bool index_has_new_data(Index* index)
{
    bool state = index->has_new_data;
    index->has_new_data = false;
    return state;
}

void groups_print(Group* g)
{
    int32_t c = 0;
    printf("\n%5s %6s %5s %9s %9s %9s %9s\n", "INDEX", "WSTART", "WEND",  "OPEN", "HIGH", "LOW", "CLOSE");

    while (g != NULL) {
        if (g->is_empty)
            printf("%5d %6f %5f %9s %9s %9s %9s\n", c, g->wstart, g->wend, "empty", "empty", "empty", "empty");
        else
            printf("%5d %6f %5f %9.9f %9.9f %9.9f %9.9f\n", c, g->wstart, g->wend, g->open, g->high, g->low, g->close);
        g = g->next;
        c++;
    }
}

void index_print(Index* index)
{
    for (int i=0 ; i<index->isize ; i++) {
        Bin* b = index->bins[i];
        LineBin* lb = b->lines[0];

        if (b->is_empty)
            continue;

        printf("BIN %3d: %f:%f %d => %9f %9f %9f %9f\n", i,
                                              b->wstart,
                                              b->wend,
                                              lb->npoints,
                                              lb->open,
                                              lb->high,
                                              lb->low,
                                              lb->close);
    }
}

int8_t line_add_point(LineBin* lb, Point* p)
{
    lb->npoints++;

    if (lb->is_empty) {
        lb->is_empty = false;
        lb->open = p->open;
        lb->close = p->close;
        lb->high = p->high;
        lb->low = p->low;
        lb->xmin = p->x;
        lb->xmax = p->x;
        return 1;
    }
    // update line x lower limits if p->x is older than oldest point in line
    if (p->x < lb->xmin) {
        lb->open = p->open;
        lb->xmin = p->x;
    }
    // update line x upper limits if p->x is newer than oldest point in line
    if (p->x > lb->xmax) {
        lb->close = p->close;
        lb->xmax = p->x;
    }
    if (p->high > lb->high)
        lb->high = p->high;

    if (p->low < lb->low)
        lb->low = p->low;

    return 1;
}