#include "yaxis.h"

Axis* axis_init(AxisSide side)
{
    Axis* a = malloc(sizeof(Axis));
    a->side = side;
    a->txsize = 0;
    a->tysize = 0;
    a->txstart = 0;

    a->nwhole = 0;
    a->nfrac = 0;

    a->tdmin = 0;
    a->tdmax = 0;

    a->vdmin = 0;
    a->vdmax = 0;

    a->dmin = 0;
    a->dmax = 0;

    // line linked list
    a->line = NULL;
    a->ltail = malloc(sizeof(Line*));
    *(a->ltail) = NULL;

    a->is_empty = true;
    a->autorange = true;

    return a;
}

void axis_add_line(Axis* a, Line* l)
{
    // connect line in linked list
    if (a->line == NULL) {
        a->line = l;
        *(a->ltail) = l;
    } else {
        Line* prev = *(a->ltail);
        *(a->ltail) = l;
        prev->next = l;
    }
    // add axis stuct to line struct
    l->axis = a;
}
