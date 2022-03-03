#include "line.h"

Line* line_init(char* name, LineID* lineid)
{
    Line* l = malloc(sizeof(Line));
    l->name = strdup(name);
    l->next = NULL;
    l->groups = NULL;
    l->lineid = lineid;

    if (lineid->ltype == LTYPE_OHLC)
        strcpy(l->icon, CS_ICON);
    else if (lineid->ltype == LTYPE_LINE)
        strcpy(l->icon, LINE_ICON);
    else
        strcpy(l->icon, LINE_ICON);

    strcpy(l->chr,  DEFAULT_LINE_CHR);
    l->color = CGREEN;
    return l;
}

void line_destroy(Line* l)
{
    free(l->name);
    free(l);
}

int8_t line_set_data(Line* l, GroupContainer* gc)
{
    /* Set new data in line, update axis data dimensions */

    static int i = 0;
    i++;

    // Error if line is not part of an axis
    if (l->axis == NULL)
        return -1;

    if (gc == NULL)
        return 0;
        //return -1;

    if (gc->is_empty)
        return 0;

    l->groups = gc;

    Yaxis* a = l->axis;

    // set/update axis dimensions
    if (a->is_empty) {
        a->is_empty = false;
        a->tdmin = gc->dmin;
        a->tdmax = gc->dmax;
        a->vdmin = gc->gmin;
        a->vdmax = gc->gmax;
    } else {
        if (gc->dmin < a->tdmin)
            a->tdmin = gc->dmin;
        if (gc->dmax > a->tdmax)
            a->tdmax = gc->dmax;

        if (gc->gmin < a->vdmin)
            a->vdmin = gc->gmin;
        if (gc->gmax > a->vdmax)
            a->vdmax = gc->gmax;
    }
    return 0;
}
