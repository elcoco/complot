#include "json.h"

/* read file from disk and parse JSON */
JSONObject* json_load_file(char *path)
{
    // read file in chunks and dynamically allocate memory for buffer
    uint32_t chunk_size = 1000;   // read file in chunks
    uint32_t offset     = 0;    // offset in buffer to write data to
    uint32_t n_read     = 0;    // store amount of chars read from file
    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        printf("File doesn't exist\n");
        return NULL;
    }

    char *buf = calloc(chunk_size, 1);

    while ((n_read=fread(buf + offset, 1, chunk_size, fp)) > 0) {
        offset += n_read;
        buf = realloc(buf, offset+chunk_size);
    }

    buf[offset] = '\0';     // properly end string array
    fclose(fp);
    printf("read: %ld bytes\n", strlen(buf));

    JSONObject* jo = json_load(buf);
    return jo;
}


char* pos_next(Position *pos)
{
    /* Increment position in json string */
    (pos->npos)++;
    (pos->cols)++;
    return (pos->c)++;
}

int json_destroy(JSONObject* jo)
{
    free(jo);
    return 0;
}

JSONObject* json_object_init()
{
    JSONObject* jo = malloc(sizeof(JSONObject));
    jo->parent = NULL;
    jo->prev = NULL;
    jo->next = NULL;

    jo->dtype = JSON_UNKNOWN;

    jo->key = NULL;
    jo->value = NULL;

    jo->is_string = false;
    jo->is_float  = false;
    jo->is_int    = false;
    jo->is_bool   = false;
    jo->is_array  = false;
    jo->is_object = false;
    return jo;
}

char fforward(Position* pos, char* search_lst, char* expected_lst, char* ignore_lst, char* buf)
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

    while (!strchr(search_lst, *(pos->c))) {
        if (buf != NULL) {
            if (!strchr(ignore_lst, *(pos->c)) && expected_lst == NULL)
                *ptr++ = *(pos->c);
            else if (!strchr(ignore_lst, *(pos->c)) && strchr(expected_lst, *(pos->c)))
                *ptr++ = *(pos->c);
            else if (!strchr(ignore_lst, *(pos->c)) && !strchr(expected_lst, *(pos->c)))
                return -1;
        }
        // keep track of rows/cols position
        if (*(pos->c) == '\n') {
            pos->rows += 1;
            pos->cols = 0;
        }
        pos->pos_next(pos);
    }
    // terminate string
    if (ptr != NULL)
        *ptr = '\0';

    return *(pos->c);
}

/* print context for error message */
void print_error(Position *pos, uint32_t amount) {
    char lctext[2*LINES_CONTEXT];       // buffer for string left from current char
    char rctext[2*LINES_CONTEXT];       // buffer for string right from current char

    char *lptr = lctext;
    char *rptr = rctext;

    // get context
    for (uint8_t i=0,j=amount ; i<amount ; i++, j--) {

        // check if we go out of left string bounds
        if ((int8_t)(pos->npos - j) >= 0) {
            *lptr = *(pos->c - j);                  // add char to string
            lptr++;
        }
        // check if we go out of right string bounds
        // BUG this is not bugfree
        if ((pos->npos + i) < strlen(pos->json)) {
            *rptr = *(pos->c + i);               // add char to string
            rptr++;
        }
    }
    rctext[amount] = '\0';
    lctext[amount] = '\0';

    printf("JSON syntax error: %c @ (%d,%d)\n", *(pos->c), pos->rows, pos->cols);
    printf("%s%s%c%s<--%s%s\n", lctext, JRED, *(pos->c), JBLUE, JRESET, rctext);
}


JSONObject* json_parse(Position* pos)
{
    JSONObject* jo = json_object_init();

    char tmp[MAX_BUF] = {'\0'};
    char c;

    if ((c = fforward(pos, "[{\"'", NULL, "\n", tmp)) < 0) {
        printf("Error while reading JSON: \"%s\" + \"%c\"\n", tmp, *(pos->c));
        print_error(pos, LINES_CONTEXT);
        return NULL;
    }
    switch (c) {
        case '{':
            printf("parse object\n");
            break;
        case '[':
            printf("parse array\n");
            break;
        case '\'':
            printf("parse string\n");
            break;
        case '\"':
            printf("parse string\n");
            break;
    }

    printf("tmp: \"%s\"\n", tmp);
    printf("c: \"%c\"\n", c);
    return jo;
}

JSONObject* json_load(char* buf)
{
    Position* pos = malloc(sizeof(Position));
    pos->json     = buf;
    pos->c        = buf;
    pos->npos     = 0;
    pos->cols     = 1;
    pos->rows     = 1;
    pos->pos_next = &pos_next;

    JSONObject* jo = json_parse(pos);


    // cleanup
    free(pos->json);
    free(pos);

    return jo;
}
