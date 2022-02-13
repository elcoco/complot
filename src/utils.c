#include "utils.h"

float get_avg(float avg, uint16_t i, float value)
{
    return (avg * i + value) / (i+1);
}

uint16_t get_rand(uint16_t lower, uint16_t upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

uint32_t map(double value, double in_min, double in_max, uint32_t out_min, uint32_t out_max)
{

    //set_status(1, "mapping: %f %f %f %d %d => %d", value, in_min, in_max, out_min, out_max, (uint32_t)floor((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min));
    //usleep(10);
    //refresh();
    //printf("input: %f %f %f %d %d = %f\n", value, in_min, in_max, out_min, out_max, (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
    uint32_t i = round((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
    //i = (i<out_min) ? out_min : i;
    //i = (i>out_max) ? out_max : i;
    return i;
}
