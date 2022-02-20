#include "read_thread.h"

void* read_file_thread(void* args)
{
    Args* a = args;

    char buf[BUF_SIZE] = {'\0'};
    char tmpbuf[BUF_SIZE] = {'\0'};
    uint32_t xsteps = 5;

    // x incrementer, must be a converted datetime but for now this will do
    double ix = 0;
    //FILE* fp = fopen("csv/test.csv", "r");
    FILE* fp = fopen("csv/btcusd.csv", "r");
    //FILE* fp = fopen("csv/XMRBTC_1m.csv", "r");

    while (fgets(buf, sizeof(buf), fp) != NULL && !a->is_stopped) {
        // skip first line
        //if (ix == 0) {
        //    ix+=xsteps;
        //    continue;
        //}

        if (buf[strlen(buf)-1] == '\n') {
            char* spos = buf;
            int c = 0;
            double dt, open, high, low, close;

            while (fast_forward(&spos, ",\n", NULL, NULL, tmpbuf)) {

                // Reset local, using UTF-8 makes atof() wait for ',' instead of '.'
                // as a float delimiter
                // So to reset the previous: setlocale(LC_ALL, "");
                setlocale(LC_NUMERIC,"C");

                // NOTE atof failes on unicode!!!!
                if (c == a->iopen)
                    open = atof(tmpbuf);
                else if (c == a->ihigh)
                    high = atof(tmpbuf);
                else if (c == a->ilow)
                    low = atof(tmpbuf);
                else if (c == a->iclose)
                    close = atof(tmpbuf);
                else if (c == a->idt)
                    dt = atof(tmpbuf);

                spos++;
                c++;
                tmpbuf[0] = '\0';
            }

            pthread_mutex_lock(a->lock);
            point_create(a->index, LINE1, dt, open, high, low, close);
            pthread_mutex_unlock(a->lock);

        } else {
            printf("too long!!! %lu: %s\n", strlen(buf), buf);
        }

        ix+=xsteps;

        usleep(10000);
    }
    return NULL;
}

/*
void* read_stdin_thread(void* args)
{
    Args* a = args;

    char buf[BUF_SIZE] = {'\0'};
    char tmpbuf[BUF_SIZE] = {'\0'};
    uint32_t xsteps = 5;

    // x incrementer, must be a converted datetime but for now this will do
    uint32_t ix = 0;

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (ix == 0) {
            ix+=xsteps;
            continue;
        }

        if (buf[strlen(buf)-1] == '\n') {
            char* spos = buf;
            int c = 0;
            double dt, open, high, low, close;

            while (fast_forward(&spos, ",\n", NULL, NULL, tmpbuf)) {

                // Reset local, using UTF-8 makes atof() wait for ',' instead of '.'
                // as a float delimiter
                // So to reset the previous: setlocale(LC_ALL, "");
                setlocale(LC_NUMERIC,"C");

                // NOTE atof failes on unicode!!!!
                if (c == a->idt)
                    dt = atof(tmpbuf);
                else if (c == a->iopen)
                    open = atof(tmpbuf);
                else if (c == a->ihigh)
                    high = atof(tmpbuf);
                else if (c == a->ilow)
                    low = atof(tmpbuf);
                else if (c == a->iclose)
                    close = atof(tmpbuf);

                spos++;
                c++;
                tmpbuf[0] = '\0';
            }

            Point* p = point_create(LINE1, ix, open, high, low, close);

            pthread_mutex_lock(a->lock);
            if (index_insert(a->index, p) < 0)
                printf("Failed to insert point\n");
            pthread_mutex_unlock(a->lock);



        } else {
            printf("too long!!! %lu: %s\n", strlen(buf), buf);
        }

        ix+=xsteps;

        printf("read data\n");
        usleep(10000000);
    }
}
*/


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


