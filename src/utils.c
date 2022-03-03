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

int32_t map(double value, double in_min, double in_max, uint32_t out_min, uint32_t out_max)
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

void draw_border(WINDOW* w)
{
    int cols = getmaxx(w);
    int lines = getmaxy(w);
    mvwhline(w, 0, 1, B_H, cols-2);
    mvwhline(w, lines-1, 1, B_H, cols-2);
    mvwvline(w, 1, 0, B_V, lines-2);
    mvwvline(w, 1, cols-1, B_V, lines-2);

    mvwaddstr(w, 0, 0, B_TL);
    mvwaddstr(w, 0, cols-1, B_TR);
    mvwaddstr(w, lines-1, 0, B_BL);
    mvwaddstr(w, lines-1, cols-1, B_BR);
}


void fill_win(WINDOW* w, char c)
{
    for (int y=0 ; y<getmaxy(w) ; y++) {
        for (int x=0 ; x<getmaxx(w) ; x++) {
            mvwaddch(w, y, x, c);
        }
    }
}

Group* fast_forward_groups(Group* g, uint32_t amount)
{
    /* skip n amount groups, return pointer to next group */
    while (amount != 0) {
        g = g->next;
        amount--;
    }
    return g;
}

void debug(char* fmt, ...)
{

    char buf[100];

    va_list ptr;
    va_start(ptr, fmt);
    vsprintf(buf, fmt, ptr);
    va_end(ptr);

    FILE* fp = fopen("./complot.log", "a");
    fputs(buf, fp);
    fclose(fp);

}
