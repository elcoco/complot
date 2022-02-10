#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h> // read

#include "index.h"
#include "utils.h"
#include "matrix.h"

#define INDEX_GROW_AMOUNT 1000
#define INDEX_SPREAD 8
#define NLINES 2

#define GROUP_SIZE 2
#define GROUPS 70 // = xsize
#define YSIZE 60

// TODO extend index when out of bounds
// DONE get groups x amount of groups from last known data
// DONE create points linked list
// TODO create CSV file reader
// DONE add xwin dimension to group
// TODO draw candlesticks
// TODO return groups as linked list
// TODO keep track of data min and max. we need this info when drawing candlesticks
// DONE segfault when GROUP_SIZE = 1 or 2
// TODO now index data limits are used, but group data limits allows for auto scale

#define BUF_SIZE 1000
#define DELIM_STR ","
#define STDIN 0

enum lines {
    LINE1,
    LINE2
} lines;

bool fast_forward(char** c, char* search_lst, char* expected_lst, char* ignore_lst, char* buf)
{
    /* fast forward until a char from search_lst is found
     * Save all chars in buf until a char from search_lst is found
     * Only save in buf when a char is found in expected_lst
     *
     * If buf == NULL,          don't save chars
     * If expected_lst == NULL, allow all characters
     */

    // save skipped chars that are on expected_lst in buffer
    char* ptr = buf;

    // don't return these chars with buffer
    ignore_lst = (ignore_lst) ? ignore_lst : "";

    // exit at EOL
    if (strlen(*c) == 0)
        return false;

    while (!strchr(search_lst, **c)) {

        if (buf != NULL) {
            if (!strchr(ignore_lst, **c) && expected_lst == NULL)
                *ptr++ = **c;
            else if (!strchr(ignore_lst, **c) && strchr(expected_lst, **c))
                *ptr++ = **c;
            else
                return false;
        }
        (*c)++;
    }

    // terminate string
    if (ptr != NULL)
        *ptr = '\0';

    return true;
}

void die(char* msg)
{
    printf("ERROR: %s\n", msg);
    exit(0);
}

int8_t ctoi(char c)
{
    switch (c) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        default:
            return -1;
    }
}

//int read_stdin3(char* fmt)
int read_stdin(Index* index, uint8_t idt, uint8_t iopen, uint8_t ihigh, uint8_t ilow, uint8_t iclose)
{
    char buf[BUF_SIZE] = {'\0'};
    char tmpbuf[BUF_SIZE] = {'\0'};

    // x incrementer, must be a converted datetime but for now this will do
    uint32_t ix = 0;

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (ix == 0) {
            ix++;
            continue;
        }

        if (buf[strlen(buf)-1] == '\n') {
            char* spos = buf;
            int c = 0;
            double open, high, low, close;

            while (fast_forward(&spos, ",\n", NULL, NULL, tmpbuf)) {

                if (c == iopen)
                    open = atof(tmpbuf);
                else if (c == ihigh)
                    high = atof(tmpbuf);
                else if (c == ilow)
                    low = atof(tmpbuf);
                else if (c == iclose)
                    close = atof(tmpbuf);

                spos++;
                c++;
            }

            Point* p = point_create(ix, open, high, low, close);
            if (index_insert(index, LINE1, p) < 0)
                return -1;

        } else {
            printf("too long!!! %d: %s\n", strlen(buf), buf);
        }

        ix++;
    }

}

int main(int argc, char **argkv)
{

    Index* index = index_create(INDEX_GROW_AMOUNT, INDEX_SPREAD, NLINES);

    read_stdin(index, 0, 2, 3, 4, 5);

    index_print(index);

    Group* g = index_get_grouped(index, LINE1, GROUP_SIZE, GROUPS);
    groups_print(g);



    ViewPort* vp = vp_init(GROUPS, YSIZE); // x, y

    // x index
    int ix = 0;

    Group* gptr = g;
    double dmin = gptr->low;
    double dmax = gptr->high;

    // find data min max for group slice
    while (gptr != NULL) {
        if (! gptr->is_empty) {
            if (gptr->high > dmax)
                dmax = gptr->high;
            if (gptr->low < dmin)
                dmin = gptr->low;
        }
        gptr = gptr->next;
    }


    printf("INDEX dimensions: %f %f\n", index->dmin, index->dmax);
    printf("GROUP dimensions: %f %f\n", dmin, dmax);


    // draw candlesticks
    while (g != NULL) {

        if (! g->is_empty)
            
            vp_draw_candlestick(vp, g, ix, dmin, dmax);

        g = g->next;
        ix++;
    }

    vp_print(vp);

    //points_print(*(index->phead));
    //read_csv("/home/eco/dev/ccomplot/csv/XMRBTC_1m.csv");
              



    return 0;
}
