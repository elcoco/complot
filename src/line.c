#include "line.h"

uint32_t lineid_counter = 0;

Line* line_init(char* name)
{
    Line* l = malloc(sizeof(Line));
    l->name = strdup(name);
    l->next = NULL;
    l->lineid = lineid_counter++;
    return l;
}

void line_destroy(Line* l)
{
    free(l->name);
    free(l);
}

void  line_print_lines(Line* l)
{
    /* For debugging */
    while (l != NULL) {
        printf("l: %s\n", l->name);
        l = l->next;
    }
}

int8_t line_set_data(Line* l, Groups* groups)
{
    /* Set new data in line, update axis data dimensions */

    // Error if line is not part of an axis
    if (l->axis == NULL)
        return -1;

    l->groups = groups;

    Axis* a = l->axis;

    // set/update axis dimensions
    if (a->is_empty) {
        a->is_empty = false;
        a->tdmin = groups->dmin;
        a->tdmax = groups->dmax;
        a->vdmin = groups->gmin;
        a->vdmax = groups->gmax;
    } else {
        if (groups->dmin < a->tdmin)
            a->tdmin = groups->dmin;
        if (groups->dmax > a->tdmax)
            a->tdmax = groups->dmax;

        if (groups->gmin < a->vdmin)
            a->vdmin = groups->gmin;
        if (groups->gmax > a->vdmax)
            a->vdmax = groups->gmax;
        //printf("using autorange: %f, %f | %f %f\n", a->vdmin, a->vdmax, a->tdmin, a->tdmax);
    }

    // digits before and after the dot
    a->nwhole = find_nwhole(a->dmax); 
    a->nfrac  = find_nfrac(a->dmax - a->dmin);
    a->txsize  = a->nwhole + 1 + a->nfrac;
    return 0;
}
