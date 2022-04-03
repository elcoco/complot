#include "utils.h"

void die(char* msg)
{
    printf("ERROR: %s\n", msg);
    abort();
    //exit(0);
}

double get_avg(double avg, uint32_t i, double value)
{
    /* get a rolling average */
    return (avg * i + value) / (i+1);
}

uint16_t get_rand(uint16_t lower, uint16_t upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}

bool y_is_in_view(WINDOW* win, uint32_t iy)
{
    /* check if y fits in window matrix */
    return (iy >= 0 && iy < getmaxy(win));
}

int32_t map(double value, double in_min, double in_max, uint32_t out_min, uint32_t out_max)
{
    return round((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

uint32_t count_digits(uint64_t n)
{
    return (n==0) ? 1 : floor(log10(n) + 1);
}

uint32_t find_nfrac2(double min, double max)
{
    /* Find the amount of decimals that should be displayed on the yaxis.
     * Compare ymax and ymin from left to right.
     * If digit is not same, this is amount of digits for precision.
     */
    char minbuf[100] = {'\0'};
    char maxbuf[100] = {'\0'};

    sprintf(minbuf, "%f", min - abs(min));
    sprintf(maxbuf, "%f", max - abs(max));

    char* pmin = minbuf;
    char* pmax = maxbuf;

    // skip 0. chars
    pmin += 2;
    pmax += 2;

    int i;

    for (i=0 ; i<strlen(minbuf) ; i++, pmin++, pmax++) {
        if (*pmin != *pmax)
            break;
    }
    return i+3;
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

    FILE* fp = fopen(LOG_PATH, "a");
    fputs(buf, fp);
    fclose(fp);
}

bool non_blocking_sleep(int interval, bool(*callback)(void* arg), void* arg)
{
    /* Do a non blocking sleep that checks for user input */
    struct timeval t_start, t_end;
    gettimeofday(&t_start, NULL);

    while (1) {
        gettimeofday(&t_end, NULL);
        if ((t_end.tv_sec*1000000 + t_end.tv_usec) - (t_start.tv_sec*1000000 + t_start.tv_usec) >= interval)
            break;

        if (callback(arg))
            return true;

        usleep(SLEEP_CHECK_INTERVAL);
    }
    return false;
}

char* str_to_lower(char* str)
{
    /* destructive convert string to lowercase */
    for (char* p=str ; *p != '\0' ; p++) {
        if (*p >= 'A' && *p <= 'Z')
            *p += ('a' - 'A');
    }
    return str;
}

void** reverse_arr(void **arr, int length)
{
    /* Reverse an array that is delimited with NULL */
    void** reversed = malloc(length * sizeof(void*));

    void** parr = arr;
    void** prev = reversed+length-2;

    while (*parr != NULL)
        *prev-- = *parr++;

    reversed[length-1] = NULL;
    return reversed;


}

void display_log_win()
{

    // position in data
    int32_t lpos = 0;

    // buffer size
    int32_t lsize = 500;

    char **lines = malloc(lsize * sizeof(char*));
    FILE* fp = fopen(LOG_PATH, "r");

    if (fp == NULL)
        goto cleanup;

    char buf[MAX_LOG_LINE_LENGTH] = {'\0'};

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (lsize-1 <= lpos) {
            lsize += 500;
            lines = realloc(lines, lsize * sizeof(char*));
        }

        // remove newline
        if (strlen(buf) && buf[strlen(buf)-1] == '\n')
            buf[strlen(buf)-1] = '\0';

        // add line number
        char tmp[MAX_LOG_LINE_LENGTH+MAX_LOG_LINE_NO_LENGTH] = {'\0'};
        sprintf(tmp, "%d: %s", lpos, buf);
        lines[lpos] = strdup(tmp);

        lpos++;
    }

    lines[lpos] = NULL;

    fclose(fp);

    char** reversed = reverse_arr((void**)lines, lpos+1);

    menu_show(reversed, LINES-6, COLS-6);

    cleanup:
        char** parr = lines;
        while (*parr != NULL)
            free(*parr++);

        free(lines);
        free(reversed);
}
