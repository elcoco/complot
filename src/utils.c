#include "utils.h"

void die(char* msg)
{
    printf("ERROR: %s\n", msg);
    exit(0);
}

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
    return round((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

uint32_t count_digits(uint32_t n)
{
    return floor(log10(n) + 1);
}

uint32_t find_nfrac(double f)
{
    /* return amount digits after decimal point */
    double frac = f - abs(f);
    char str[30];
    return sprintf(str, "%g", frac) -2;
}

uint32_t find_nwhole(double f)
{
    /* return amount digits before decimal point */
    return count_digits(abs(f));
}
