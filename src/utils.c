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
    //printf("input: %f %f %f %d %d = %f\n", value, in_min, in_max, out_min, out_max, (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
    return floor((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}
