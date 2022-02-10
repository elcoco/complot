#include "utils.h"

float get_avg(float avg, uint16_t i, float value)
{
    return (avg * i + value) / (i+1);
}

uint16_t get_rand(uint16_t lower, uint16_t upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

int32_t map_dec_to_i(float value, float amin, float amax, float bmin, float bmax)
{
        /* Map/scale one range to another */
        double slope = 1.0 * (bmax - amin) / (amax - amin);
        double output = bmin + slope * (value - amin);
        printf("%%%%%f\n", output);
        return output;
}

uint32_t map(double value, double in_min, double in_max, uint32_t out_min, uint32_t out_max)
{
    printf("input: %f %f %f %d %d\n", value, in_min, in_max, out_min, out_max);
    printf(">>%f\n", (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
    return floor((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}
