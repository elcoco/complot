#include "json.h"

/* create string of n amount of spaces */
void get_spaces(char *buf, uint8_t spaces) {
    uint8_t i;
    for (i=0 ; i<spaces ; i++) {
        buf[i] = ' ';
    }
    buf[i] = '\0';
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
        if ((pos->npos + i +1) < strlen(pos->json)) {
            *rptr = *(pos->c + i +1);               // add char to string
            rptr++;
        }
    }
    rctext[amount] = '\0';
    lctext[amount] = '\0';

    printf("JSON syntax error: %c @ (%d,%d)\n", *(pos->c), pos->rows, pos->cols);
    printf("%s%s%c%s<--%s%s\n", lctext, JRED, *(pos->c), JBLUE, JRESET, rctext);
}

double json_get_number(JSONObject* jo)
{
    return *((double*)jo->value);
}

char* json_get_string(JSONObject* jo)
{
    return (char*)jo->value;
}

bool json_get_bool(JSONObject* jo)
{
    return *((bool*)jo->value);
}

void json_print(JSONObject* jo, uint32_t level)
{
    uint8_t incr = 3;
    char space[level+1];
    get_spaces(space, level);

    if (jo != NULL) {
        printf("%s", space);
        if (jo->parent && jo->index >= 0 && jo->parent->dtype == JSON_ARRAY)
            printf("%s%d:%s ", JRED, jo->index, JRESET);
        if (jo->key)
            printf("%s%s:%s ", JWHITE, jo->key, JRESET);

        switch (jo->dtype) {

            case JSON_NUMBER:
                printf("%s%f%s\n", JGREEN, json_get_number(jo), JRESET);
                break;

            case JSON_STRING:
                printf("%s\"%s\"%s\n", JBLUE, json_get_string(jo), JRESET);
                break;

            case JSON_BOOL:
                printf("%s%s%s\n", JMAGENTA, json_get_bool(jo) ? "true" : "false", JRESET);
                break;

            case JSON_ARRAY:
                printf("%s[ARRAY]%s\n", JGREEN, JRESET);
                json_print(jo->value, level+incr);
                break;

            case JSON_OBJECT:
                printf("%s[OBJECT]%s\n", JGREEN, JRESET);
                json_print(jo->value, level+incr);
                break;

            case JSON_UNKNOWN:
                printf("%s[UNKNOWN]%s\n", JRED, JRESET);
                break;
        }
    }
    if (jo->next != NULL)
        json_print(jo->next, level);
}

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
    // keep track of rows/cols position
    if (*(pos->c) == '\n') {
        pos->rows += 1;
        pos->cols = 0;
    } else {
        (pos->cols)++;
    }
    (pos->npos)++;
    (pos->c)++;

    // NOTE: EOF should not be reached under normal conditions.
    //       This indicates corrupted JSON
    if (pos->npos >= pos->length) {
        printf(">>>>>>>>>> EOL @ c=%c, pos: %d, %d,%d\n", *(pos->c), pos->npos, pos->cols, pos->rows);
        return NULL;
    }
    return pos->c;
}

int json_destroy(JSONObject* jo)
{
    if (jo->dtype == JSON_OBJECT || jo->dtype == JSON_ARRAY)
        json_destroy(jo->value);

    if (jo->parent && (jo->parent->dtype == JSON_OBJECT || jo->parent->dtype == JSON_ARRAY)) {

        JSONObject* tmpobj = jo;
        while (tmpobj != NULL) {
            tmpobj = tmpobj->next;
        }

    }


    // TODO iter tree and free objects
    free(jo);
    return 0;
}

JSONObject* json_object_init(JSONObject* parent)
{
    JSONObject* jo = malloc(sizeof(JSONObject));
    jo->parent = parent;
    jo->children = NULL;
    jo->prev = NULL;
    jo->next = NULL;

    jo->length = -1;
    jo->index = -1;

    jo->dtype = JSON_UNKNOWN;

    jo->key = NULL;
    jo->value = NULL;

    jo->is_string = false;
    jo->is_number  = false;
    jo->is_bool   = false;
    jo->is_array  = false;
    jo->is_object = false;
    return jo;
}

char fforward(Position* pos, char* search_lst, char* expected_lst, char* unwanted_lst, char* ignore_lst, char* buf)
{
    /* fast forward until a char from search_lst is found
     * Save all chars in buf until a char from search_lst is found
     * Only save in buf when a char is found in expected_lst
     * Error is a char from unwanted_lst is found
     *
     * If buf == NULL,          don't save chars
     * If expected_lst == NULL, allow all characters
     * If unwanted_lst == NULL, allow all characters
     */
    // TODO char can not be -1

    // save skipped chars that are on expected_lst in buffer
    char* ptr = buf;

    // don't return these chars with buffer
    ignore_lst = (ignore_lst) ? ignore_lst : "";
    unwanted_lst = (unwanted_lst) ? unwanted_lst : "";

    while (!strchr(search_lst, *(pos->c))) {
        if (strchr(unwanted_lst, *(pos->c)))
            return -1;

        if (expected_lst != NULL) {
            if (!strchr(expected_lst, *(pos->c)))
                return -1;
        }
        if (buf != NULL && !strchr(ignore_lst, *(pos->c)))
            *ptr++ = *(pos->c);

        pos_next(pos);
    }
    // terminate string
    if (ptr != NULL)
        *ptr = '\0';

    char ret = *(pos->c);

    return ret;
}

JSONStatus json_parse_number(JSONObject* jo, Position* pos)
{
    /* All numbers are floats */
    char tmp[MAX_BUF] = {'\0'};
    char c;

    jo->dtype = JSON_NUMBER;
    jo->is_number = true;

    if ((c = fforward(pos, ", ]}\n", "0123456789-null.", NULL, "\n", tmp)) < 0) {
        printf("Error while reading JSON: \"%s\" + \"%c\"\n", tmp, *(pos->c));
        print_error(pos, LINES_CONTEXT);
        return PARSE_ERROR;
    }
    //jo->value = strdup(tmp);
    double* value = malloc(sizeof(double));
    *value = atof(tmp);
    jo->value = value;
    return STATUS_SUCCESS;
}

JSONStatus json_parse_bool(JSONObject* jo, Position* pos)
{
    char tmp[MAX_BUF] = {'\0'};
    char c;

    jo->dtype = JSON_BOOL;
    jo->is_bool = true;

    if ((c = fforward(pos, ", ]}\n", "truefalse", NULL, "\n", tmp)) < 0) {
        printf("Error while reading JSON: \"%s\" + \"%c\"\n", tmp, *(pos->c));
        print_error(pos, LINES_CONTEXT);
        return PARSE_ERROR;
    }
    bool* value = malloc(sizeof(bool));

    if (strcmp(tmp, "true") == 0)
        *value = true;
    else if (strcmp(tmp, "false") == 0)
        *value = false;
    else
        return PARSE_ERROR;

    jo->value = value;
    return STATUS_SUCCESS;
}

JSONStatus json_parse_key(JSONObject* jo, Position* pos)
{
    /* Parse key part of an object */
    char key[MAX_BUF] = {'\0'};
    char c;

    // skip to start of key
    if ((c = fforward(pos, "\"'}", ", \n", NULL, "\n", NULL)) < 0) {
        printf("Error while reading JSON: \"%c\"\n", *(pos->c));
        print_error(pos, LINES_CONTEXT);
        return PARSE_ERROR;
    }
    if (c == '}')
        return END_OF_OBJECT;

    pos_next(pos);

    // read key
    if ((c = fforward(pos, "\"'\n", NULL, NULL, "\n", key)) < 0) {
        printf("Error while reading JSON: \"%s\" + \"%c\"\n", key, *(pos->c));
        print_error(pos, LINES_CONTEXT);
        return PARSE_ERROR;
    }
    if (c == '}')
        return END_OF_OBJECT;
    
    // skip over "
    pos_next(pos);

    // find colon
    if ((c = fforward(pos, ":", " \n", NULL, "\n", NULL)) < 0) {
        printf("Error while reading JSON: \"%c\"\n", *(pos->c));
        print_error(pos, LINES_CONTEXT);
        return PARSE_ERROR;
    }

    // skip over colon
    pos_next(pos);

    jo->key = strdup(key);
    return STATUS_SUCCESS;
}

JSONStatus json_parse_array(JSONObject* jo, Position* pos)
{
    jo->dtype = JSON_ARRAY;
    jo->length = 0;

    JSONObject* head = NULL;
    JSONObject* tail = NULL;

    while (1) {
        JSONObject* child = json_object_init(jo);

        JSONStatus ret = json_parse(child, pos);
        if (ret < 0)
            return ret;
        else if (ret == END_OF_ARRAY)
            break;

        if (head == NULL) {
            head = child;
            tail = child;
        } else {
            JSONObject* prev = tail;
            prev->next = child;
            child->prev = prev;
            tail = child;
        }
        jo->length++;
    }
    jo->value = head;
    tail->next = NULL;

    // we know the index length now so lets create the array
    jo->children = malloc(jo->length * sizeof(JSONObject));
    JSONObject* child = jo->value;
    for (int i=0 ; i<jo->length ; i++) {
        jo->children[i] = child;
        child->index = i;
        child = child->next;
    }

    return STATUS_SUCCESS;
}

JSONStatus json_parse_object(JSONObject* jo, Position* pos)
{
    jo->dtype = JSON_OBJECT;
    jo->length = 0;

    JSONObject* head = NULL;
    JSONObject* tail = NULL;

    while (1) {
        JSONObject* child = json_object_init(jo);

        JSONStatus ret_key = json_parse_key(child, pos);
        if (ret_key < 0)
            return ret_key;
        else if (ret_key == END_OF_OBJECT)
            break;

        // parse the value
        JSONStatus ret_value = json_parse(child, pos);
        if (ret_value < 0)
            return ret_value;
        else if (ret_value == END_OF_OBJECT)
            break;

        if (head == NULL) {
            head = child;
            tail = child;
        } else {
            JSONObject* prev = tail;
            prev->next = child;
            child->prev = prev;
            tail = child;
        }
        jo->length++;

    }
    jo->value = head;
    tail->next = NULL;

    // we know the index length now so lets create the array
    jo->children = malloc(jo->length * sizeof(JSONObject));
    JSONObject* child = jo->value;
    for (int i=0 ; i<jo->length ; i++) {
        jo->children[i] = child;
        child->index = i;
        child = child->next;
    }

    return STATUS_SUCCESS;
}

JSONStatus json_parse_string(JSONObject* jo, Position* pos)
{
    char tmp[MAX_BUF] = {'\0'};
    char c;

    jo->dtype = JSON_STRING;
    jo->is_string = true;

    if ((c = fforward(pos, "\"'\n", NULL, NULL, "\n", tmp)) < 0) {
        printf("Error while reading JSON: \"%s\" + \"%c\"\n", tmp, *(pos->c));
        print_error(pos, LINES_CONTEXT);
        return PARSE_ERROR;
    }
    jo->value = strdup(tmp);

    // step over " char
    pos_next(pos);

    return STATUS_SUCCESS;
}

JSONStatus json_parse(JSONObject* jo, Position* pos)
{
    char tmp[MAX_BUF] = {'\0'};
    char c;

    // detect type
    if ((c = fforward(pos, "\"[{1234567890-n.tf}]", NULL, NULL, "\n", tmp)) < 0) {
        printf("Error while reading JSON: \"%s\" + \"%c\"\n", tmp, *(pos->c));
        print_error(pos, LINES_CONTEXT);
        return PARSE_ERROR;
    }

    if (c == '[') {
        pos_next(pos);
        return json_parse_array(jo, pos);
    }
    else if (c == ']') {
        pos_next(pos);
        return END_OF_ARRAY;
    }
    else if (c == '{') {
        pos_next(pos);
        return json_parse_object(jo, pos);
    }
    else if (c == '}') {
        pos_next(pos);
        return END_OF_OBJECT;
    }
    else if (c == '"' || c == '\'') {
        pos_next(pos);
        return json_parse_string(jo, pos);
    }
    else if (strchr("0123456789-n.", c)) {
        return json_parse_number(jo, pos);
    }
    else if (strchr("tf", c)) {
        return json_parse_bool(jo, pos);
    }
    else {
        return STATUS_ERROR;
    }
}

JSONObject* json_load(char* buf)
{
    Position* pos = malloc(sizeof(Position));
    pos->json     = buf;
    pos->c        = buf;
    pos->npos     = 0;
    pos->cols     = 1;
    pos->rows     = 1;
    pos->length   = strlen(buf);

    JSONObject* root = json_object_init(NULL);
    JSONStatus ret = json_parse(root, pos);

    // cleanup
    free(pos->json);
    free(pos);

    return (ret < 0) ? NULL : root;
}
