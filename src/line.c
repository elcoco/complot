#include "line.h"

// line->id counter
static int linei = 0;

Line* line_init(char* name)
{
    /* Don't call directly, use line_ohlc_init() or line_line_init() instead */
    Line* l = malloc(sizeof(Line));
    l->name = strdup(name);
    l->next = NULL;
    l->gc = NULL;
    l->lineid = malloc(sizeof(LineID));
    l->lineid->id = linei++;
    l->color = CGREEN;
    l->is_enabled = true;
    return l;
}

Line* line_ohlc_init(char* name)
{
    Line* l = line_init(name);
    strcpy(l->icon, LINE_OHLC_ICON);
    l->lineid->ltype = LTYPE_OHLC;
    return l;
}

Line* line_line_init(char* name)
{
    Line* l = line_init(name);
    strcpy(l->icon, LINE_LINE_ICON);
    strcpy(l->chr,  LINE_DEFAULT_LINE_CHR);
    l->lineid->ltype = LTYPE_LINE;
    return l;
}

void line_destroy(Line* l)
{
    free(l->name);
    free(l->lineid);
    free(l);
}

int8_t line_set_data(Line* l, GroupContainer* gc)
{
    /* Set new data in line, update axis data dimensions */

    // Error if line is not part of an axis
    if (l->axis == NULL)
        return -1;

    if (gc == NULL)
        return 0;

    if (gc->is_empty)
        return 0;

    l->gc = gc;

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
