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

uint32_t count_digits(uint64_t n)
{
    return (n==0) ? 1 : floor(log10(n) + 1);
}

uint32_t find_nfrac(double f)
{
    /* return amount digits after decimal point */
    double frac = f - abs(f);
    char str[30];
    return sprintf(str, "%g", frac);
}

uint32_t find_nwhole(double f)
{
    /* return amount digits before decimal point */
    return count_digits(abs(f));
}

char* ts_to_dt(time_t t, char* fmt, char* buf, uint8_t buflen)
{
    // if ts is in microseconds, convert to seconds
    if (count_digits(t) == 13)
        t /= 1000;

    struct tm ts;
    localtime_r(&t, &ts);
    strftime(buf, buflen, fmt, &ts);
    return buf;
}
