#include "plot.h"


Plot* plot_init(uint32_t xsize, uint32_t ysize)
{
    Plot* pl = malloc(sizeof(Plot));

    pl->xsize = xsize;
    pl->ysize = ysize;

    pl->status_size = 2;
    pl->xaxis_ysize = 2;


    pl->laxis = axis_init(AXIS_LEFT);
    pl->raxis = axis_init(AXIS_RIGHT);
    return pl;
}
