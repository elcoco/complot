#include "utils.h"

float get_avg(float avg, uint16_t i, float value)
{
    return (avg * i + value) / (i+1);
}

