#include "utils.h"

float get_avg(float avg, uint16_t i, float value)
{
    return (avg * i + value) / (i+1);
}

uint16_t get_rand(uint16_t lower, uint16_t upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}
