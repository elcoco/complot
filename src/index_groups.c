#include "index_groups.h"

Group* group_ohlc_update(Group* g, OHLCContainer* lc)
{
    if (lc == NULL)
        return NULL;

    // set initial OHLC data from line in empty group
    if (g->is_empty) {
        g->is_empty = false;
        g->open  = lc->open;
        g->high  = lc->high;
        g->low   = lc->low;
        g->close = lc->close;

    } else {
        g->close = lc->close;

        if (lc->high > g->high)
            g->high = lc->high;
        if (lc->low < g->low)
            g->low = lc->low;
    }
    return g;
}

Group* group_line_update(Group* g, LineContainer* lc)
{
    if (lc == NULL)
        return NULL;

    if (g->is_empty) {
        g->is_empty = false;
        g->y  = lc->y;
        g->high = lc->y;
        g->low = lc->y;
    }

    g->y = get_avg(g->y, g->counter, lc->y);

    g->high = g->y;
    //if (g->y > g->high)
    //    g->high = g->y;
    if (g->y < g->low)
        g->low = g->y;

    g->counter++;

    return g;
}

GroupContainer* groupcontainer_init(Index* index, LineID* lineid)
{
    GroupContainer* gc = malloc(sizeof(GroupContainer));
    gc->plast = index_get_last_point(index, lineid);
    gc->is_empty = true;
    gc->lineid = lineid;

    return gc;
}

void groupcontainer_update_limits(GroupContainer* gc, Group* g)
{
    /* Update Groups data limits from Group struct */

    // set initial data dimensions in groups container
    if (gc->is_empty && !g->is_empty) {
        gc->is_empty = false;
        gc->gmin = g->low;
        gc->gmax = g->high;
    }
    // set data dimensions in groups container
    else if (!gc->is_empty && !g->is_empty) {
        if (g->high > gc->gmax)
            gc->gmax = g->high;
        if (g->low < gc->gmin)
            gc->gmin = g->low;
    }
}

Groups* index_get_grouped(Index* index, uint32_t gsize, uint32_t amount, int32_t x_offset, int32_t y_offset)
{
    /* Create groups, get data from last data */
    // calculate at which bin index the first group starts
    int32_t groups_istart = index_get_gstart(index, gsize, amount) + (x_offset*gsize);
    Bin** bins     = index->bins;
    Groups* groups = groups_init(index);

    // for every line index
    for (int li=0 ; li<index->nlines ; li++) {
        int32_t gstart = groups_istart;

        LineID* lineid = index->lineids[li];
        if (lineid == NULL)
            continue;


        groups->lines[li] = groupcontainer_init(index, lineid);
        GroupContainer* gc = groups->lines[li];

        // setup linked list
        Group* htmp = NULL;
        Group* ttmp = NULL;
        Group** ghead = &htmp;
        Group** gtail = &ttmp;

        // TODO for some reason, all gc's for second line are NULL
        
        for (int32_t gindex=0 ; gindex<amount ; gindex++, gstart+=gsize) {
            Group* g = group_init(index, gstart, gsize, gtail);

            if (*ghead == NULL) {
                *ghead = g;
                *gtail = g;
            }

            // set OHLC values from gcontainers in group
            for (int i=0 ; i<gsize ; i++) {

                // Check if we're trying to access a group beyond index boundaries
                if (gstart < 0 || gstart+i >= index->isize-1)
                    break;

                Bin* b = bins[gstart+i];

                if (lineid->ltype == LTYPE_LINE)
                    group_line_update(g, b->lcontainers[li]);
                else if (lineid->ltype == LTYPE_OHLC)
                    group_ohlc_update(g, b->lcontainers[li]);
            }
            // TODO this must update groupcontainer
            groupcontainer_update_limits(gc, g);
        }
        gc->group = *ghead;
    }
    return groups;
}

bool index_has_new_data(Index* index)
{
    bool state = index->has_new_data;
    index->has_new_data = false;
    return state;
}

Group* group_init(Index* index, int32_t gstart, uint32_t gsize, Group** gtail)
{
    Group* g = (Group*)malloc(sizeof(Group));
    g->wstart = index_map_to_x(index, gstart);
    g->wend   = index_map_to_x(index, gstart + gsize);
    g->is_empty = true;
    g->next = NULL;
    g->counter = 0;

    // Set unique constant id for this group.
    // This is used to keep x tickers in the right spot.
    g->id = gstart / gsize;

    // connect new group to linked list
    if (*gtail != NULL)
        group_append(g, gtail);

    return g;
}

void group_append(Group* g, Group** tail)
{
    /* Append point in linked list */
    Group* prev = *tail;
    *tail = g;
    prev->next = g;
    g->prev = prev;
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

Groups* groups_init(Index* index)
{
    // create groups container
    Groups* groups = (Groups*)malloc(sizeof(Groups));
    groups->is_empty = true;
    groups->dmin = index->dmin;
    groups->dmax = index->dmax;
    groups->nlines = index->nlines;

    groups->lines = malloc(index->nlines * sizeof(GroupContainer*));
    for (int i=0 ; i<index->nlines ; i++)
        groups->lines[i] = NULL;

    return groups;
}

void groups_destroy(Groups* groups)
{
    for (int li=0 ; li<groups->nlines ; li++) {

        GroupContainer* gc = groups->lines[li];
        if (gc == NULL)
            continue;

        Group* g = gc->group;
        Group* prev;

        while (g != NULL) {
            prev = g;
            g = g->next;
            free(prev);
        }
        free(gc);
    }

    free(groups->lines);
    free(groups);
}

void groups_print(Groups* groups)
{
    for (int li=0 ; li<groups->nlines ; li++) {
        GroupContainer* gc = groups->lines[li];
        if (gc == NULL)
            continue;

        if (gc->lineid->ltype == LTYPE_OHLC) {
            Group* g = gc->group;
            int32_t c = 0;
            printf("LINE: %d\n", li);
            printf("\n%5s %6s %5s %9s %9s %9s %9s %9s\n", "INDEX", "WSTART", "WEND",  "Y", "OPEN", "HIGH", "LOW", "CLOSE");

            while (g != NULL) {
                if (g->is_empty)
                    printf("%5d %6f %5f %9s %9s %9s %9s %9s\n", c, g->wstart, g->wend, "empty", "empty", "empty", "empty", "empty");
                else
                    printf("%5d %6f %5f %9.9f %9.9f %9.9f %9.9f %9.9f\n", c, g->wstart, g->wend, g->y, g->open, g->high, g->low, g->close);
                g = g->next;
                c++;
            }
        }
        else if (gc->lineid->ltype == LTYPE_LINE) {
            Group* g = gc->group;
            int32_t c = 0;
            printf("LINE: %d\n", li);
            printf("\n%5s %6s %5s %9s\n", "INDEX", "WSTART", "WEND",  "Y");

            while (g != NULL) {
                if (g->is_empty)
                    printf("%5d %6f %5f %9s\n", c, g->wstart, g->wend, "empty");
                else
                    printf("%5d %6f %5f %9.9f\n", c, g->wstart, g->wend, g->y);
                g = g->next;
                c++;
            }
        }
    }
}

