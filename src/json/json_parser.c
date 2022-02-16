#include "json_parser.h"


/* recursive print nodes */
void print_all_rec(JSONNode *node, uint8_t pos) {
    uint8_t inc = 2;        // amount of spaces to increment each iteration
    char space[pos+1];
    get_spaces(space, pos);

    if (node != NULL) {
        switch (node->dtype) {
            case STR:
                printf("%s[STR] %s: %s\n", space, node->key, (char*)node->value);
                break;
            case ARR:
                printf("%s[ARR] %s\n", space, node->key);
                print_all_rec(node->child, pos+inc);
                break;
            case INT:
                printf("%s[INT] %s: %d\n", space, node->key, *((int32_t*)node->value));
                break;
            case FLOAT:
                printf("%s[FLOAT] %s: %f\n", space, node->key, *((float*)node->value));
                break;
            case OBJ:
                printf("%s[OBJ] %s\n", space, node->key);
                print_all_rec(node->child, pos+inc);
                break;
            case BOOL:
                if (node->value > 0)
                    printf("%s[BOOL] %s: true\n", space, node->key);
                else
                    printf("%s[BOOL] %s: false\n", space, node->key);
                break;
        }
                
        // in case of array or multiple kv pairs in object
        if (node->next != NULL) {
            print_all_rec(node->next, pos);
        }
    }
}

/* recursive print all nodes, call this function in code */
void print_all(JSONNode *node) {
    print_all_rec(node, 0);
}

void sanitize(char *string) {
    // destructive removal of all non-visible characters from string
    char tmp[strlen(string)];
    uint32_t j = 0;
    for (uint32_t i=0 ; i<strlen(string) ; i++) {
        if (string[i] > 31 && string[i] < 127) {
            tmp[j] = string[i];
            j++;
        } else if (string[i] == '\n') {
            tmp[j] = '\n';
            j++;
        }
    tmp[j+1] = '\0';
    }
    strcpy(string, tmp);
}

/* create string of n amount of spaces */
void get_spaces(char *buf, uint8_t spaces) {
    uint8_t i;
    for (i=0 ; i<spaces ; i++) {
        buf[i] = ' ';
    }
    buf[i] = '\0';
}

/* create a new node */
JSONNode* init_node() {
    JSONNode *node = (JSONNode*)malloc(sizeof(JSONNode));
    node->next   = NULL;
    node->prev   = NULL;
    node->child  = NULL;
    node->parent = NULL;

    node->key    = NULL;
    node->value  = NULL;

    node->print_all = print_all;
    node->get = get;
    node->deep_get = deep_get;
    return node;
}

/* find node by key */
JSONNode* get(JSONNode *node, char *key) {
    if (node->dtype != OBJ) {
        set_err(ERR_WRONGTYPE);
        return NULL;
    }

    JSONNode *np = node->child;
    while (np != NULL) {
        if (strcmp(np->key, key) == 0)
            return np;
        np = np->next;
    }
    return NULL;
}

/* find node by keys */
JSONNode* deep_get(JSONNode *node, uint8_t length, ...) {
    va_list valist;
    va_start(valist, length);
    JSONNode *np = node;

    for (uint8_t i=0 ; i<length ; i++) {
        np = np->get(np, va_arg(valist, char*));
        if (np == NULL) {
            va_end(valist);
            return NULL;
        }
    }
    va_end(valist);
    return np;
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
        if ((int8_t)(pos->pos - j) >= 0) {
            *lptr = *(pos->c - j);                  // add char to string
            lptr++;
        }
        // check if we go out of right string bounds
        // BUG this is not bugfree
        if ((pos->pos + i) < strlen(pos->json)) {
            *rptr = *(pos->c + i);               // add char to string
            rptr++;
        }
    }
    rctext[amount] = '\0';
    lctext[amount] = '\0';

    printf("JSON syntax error: %c @ (%d,%d)\n", *(pos->c), pos->rows, pos->cols);
    printf("%s%s%c%s<--%s%s\n", lctext, JRED, *(pos->c), JBLUE, JRESET, rctext);
}

/* increment pointer till char matches with a char in list, return matching char */
char skip_to_chr(Position *pos, char* list) {
    char whitelist[] = " \n";                     // return error if char is not in whitelist, continue if it is
    while (*(pos->c) != '\0') {
        if (strchr(list, *(pos->c)) != NULL)            // check if c is a char that we're looking for
            return *(pos->c);
        else if (strchr(whitelist, *(pos->c)) == NULL) {    // check if c is ok to skip, otherwise return error
            return -1;
        }
        // keep track of rows/cols position
        if (*(pos->c) == '\n') {
            pos->rows += 1;
            pos->cols = 0;
        }
        pos->next(pos);
    }
    return -1;
}

/* read string and return char* holding string
 * pass string as double pointer so position is incremented also in caller */
char* read_string(Position *pos) {
    if (skip_to_chr(pos, "\"") == -1) {         // exit if start of string is not found
        return NULL;
    }

    uint16_t chunk_size = 1024;
    uint32_t str_length = 1024;
    char *tmp = (char*)calloc(str_length, 1);
    char *ptr = tmp;        // use this pointer to iterate char array

    pos->next(pos);

    while (*(pos->c) != '\0') {
        // test for overflow, grow array if necessary
        if ((ptr - tmp) >= str_length) {
            str_length += chunk_size;
            tmp = (char*)realloc(tmp, str_length);
        }

        if (*(pos->c) == '"') {
            if (*(pos->c-1) != '\\') {   // check if double quote is escaped
                *ptr = '\0';            // terminate string
                pos->next(pos);         // skip last double quote
                return tmp;
            }
        }

        *ptr = *(pos->c);             // copy char to tmp char array
        pos->next(pos);
        ptr++;
    }
    free(tmp);
    return NULL;                // return NULL when no string is found
}

int8_t read_bool(Position *pos, JSONNode *node) {
    char tmp[100];       // holds string representation of bool
    char *ptr = tmp;    // used for itering
    int32_t *ret = (int32_t*)malloc(sizeof(int32_t));

    while (*(pos->c) != '\0') {
        if (strchr(", ]}\n", *(pos->c))) {
            *ptr = '\0';            // terminate string
            if (strcmp(tmp, "true") == 0) {
                *ret = 1;
            } else if (strcmp(tmp, "false") == 0) {
                *ret = 0;
            } else {
                printf("Failed to read bool: %s\n", tmp);
                return -1;
            }
            node->value = ret;
            return 0;
        } else if (strchr("truefalse", *(pos->c))) {
            *ptr = *(pos->c);
        } else {
            printf("Failed to read bool\n");
            return -1;                // return NULL when no array is found
        }

        pos->next(pos);
        ptr++;
    }
    return -1;                // return NULL when no array is found
}

/* read int or float */
int8_t read_numeric(Position *pos, JSONNode *node) {
    char tmp[500];
    char *ptr = tmp;

    node->dtype = INT;      // default type to integer

    while (*(pos->c) != '\0') {
        if (strchr(", ]}\n", *(pos->c))) {
            *ptr = '\0';            // terminate string
            if (strcmp(tmp, "null") == 0) {
                node->value = (int32_t*)malloc(sizeof(int32_t));
                *((int32_t*)node->value) = -1;
            } else if (node->dtype == INT) {
                node->value = (int32_t*)malloc(sizeof(int32_t));
                *((int32_t*)node->value) = atoi(tmp);
            } else {
                node->value = (float*)malloc(sizeof(float));
                *((float*)node->value) = atof(tmp);
            }
            return 0;
        } else if (*(pos->c) == '.') {
            *ptr = *(pos->c);
            node->dtype = FLOAT;
        } else if (strchr("0123456789-null", *(pos->c))) {
            *ptr = *(pos->c);
        } else {
            printf("Failed to read numeric value\n");
            return -1;
        }

        pos->next(pos);
        ptr++;
    }
    return -1;                // return NULL when no array is found
}

/* builds a linked list representing an array */
int8_t read_array(Position *pos, JSONNode *node) {
    if (skip_to_chr(pos, "[") < 0) {
        return -1;
    }
    pos->next(pos);

    // indicate first node in object, this will be the child
    // other childs will be reachable by linked list as child->next
    uint8_t first_child = 1;

    while (*(pos->c) != '\0') {
        if (*(pos->c) == ']') {
            pos->next(pos);
            return 0;
        } else if (*(pos->c) == ',') {
            pos->next(pos);
        }

        JSONNode *new_node = init_node();

        if (read_value(pos, new_node) < 0) {
            free(new_node);
            return -1;
        }

        // add node as child or if sequential node, to linked list
        if (first_child) {
            node->child = new_node;
            first_child = 0;
        } else {
            node->next = new_node;
        }
        node = new_node;

        // find separator or end of array
        if (skip_to_chr(pos, ",]") < 0) {
            free(new_node);
            return -1;
        }
    }
    return -1;                // return NULL when no array is found
}

/* read a full JSON object */
int8_t read_obj(Position *pos, JSONNode *node) {
    if (skip_to_chr(pos, "{") < 0) {    // find opening curly braces
        return -1;
    } else {
        pos->next(pos);
    }

    // indicate first node in object, this will be the child
    // other childs will be reachable by linked list as child->next
    uint8_t first_child = 1;

    while (*(pos->c) != '\0') {
        if (*(pos->c) == '}') {
            pos->next(pos);
            return 0;
        } else if (*(pos->c) == ',') {
            pos->next(pos);
        }
            
        JSONNode *new_node = init_node();

        if (read_kv(pos, new_node) == -1) {
            free(new_node);
            return -1;
        }

        // add new node to linked list, first node will be the child
        // other children can be found at child->next
        if (first_child) {
            node->child = new_node;
            first_child = 0;
        } else {
            node->next = new_node;
        }

        node = new_node;

        // find separator or end of array
        if (skip_to_chr(pos, ",}") < 0) {
            free(new_node);
            return -1;
        }
    }
    return -1;                // return NULL when no array is found
}

/* parse value for type and enter in node */
int8_t read_value(Position *pos, JSONNode *node) {
    // go to first valid character
    char chr = skip_to_chr(pos, "\"[{1234567890-n.tf");

    if (chr == '"') {
        node->dtype = STR;
        if((node->value=read_string(pos)) == NULL) {
            set_err(ERR_STR);
            return -1;
        }

    } else if (chr == '[') {
        node->dtype = ARR;
        if (read_array(pos, node) < 0) {
            set_err(ERR_ARR);
            return -1;
        }

    } else if (chr == '{') {
        node->dtype = OBJ;
        if (read_obj(pos, node) < 0) {
            set_err(ERR_OBJ);
            return -1;
        }

    } else if (strchr("0123456789-n.", chr)) {
        if (read_numeric(pos, node) < 0) {
            set_err(ERR_NUM);
            return -1;
        }

    } else if (strchr("tf", chr)) {
        node->dtype = BOOL;
        if (read_bool(pos, node) < 0) {
            set_err(ERR_BOOL);
            return -1;
        }
    } else {
        set_err(ERR_UNKNOWN);
        return -1;
    }
    return 0;
}

/* read key and value pair */
int8_t read_kv(Position *pos, JSONNode *node) {
    if((node->key=read_string(pos)) == NULL)
        return -1;

    // exit if collon is not found
    if (skip_to_chr(pos, ":") < 0)
        return -1;
    else
        pos->next(pos);

    return read_value(pos, node);
}


char* next(Position *pos) {
    (pos->c)++;
    (pos->pos)++;
    (pos->cols)++;
    return pos->c;
}

/* parse a string for json, return object */
JSONNode* parse(char *str) {
    JSONNode *root = init_node();   // create root node
    root->key = strdup("root");

    // create position object used to keep track of position in json string while parsing
    Position* pos = (Position*)malloc(sizeof(Position));
    pos->next = next;
    pos->pos = 0;
    pos->cols = 1;
    pos->rows = 1;
    pos->json = str;
    pos->c = pos->json;     // pointer used to iter string one char at a time

    if (skip_to_chr(pos, "{[") < 0) {
        printf("Error while reading JSON\n");
        set_err(ERR_SYNTAX);
        print_error(pos, LINES_CONTEXT);
        return NULL;
    }

    switch (*(pos->json)) {
        case '{':
            root->dtype = OBJ;
            if (read_obj(pos, root) < 0) {
                print_error(pos, LINES_CONTEXT);
                return NULL;
            }
            break;
        case '[':
            root->dtype = ARR;
            if (read_array(pos, root) < 0) {
                print_error(pos, LINES_CONTEXT);
                return NULL;
            }
            break;
    }
    return root;
}

/* read file from disk and parse JSON */
JSONNode* parse_file(char *path) {
    // read file in chunks and dynamically allocate memory for buffer
    uint32_t chunk_size = 1000;   // read file in chunks
    uint32_t offset     = 0;    // offset in buffer to write data to
    uint32_t n_read     = 0;    // store amount of chars read from file
    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        printf("File doesn't exist\n");
        return NULL;
    }

    char *buf = (char*)calloc(chunk_size, 1);

    while ((n_read=fread(buf + offset, 1, chunk_size, fp)) > 0) {
        offset += n_read;
        buf = (char*)realloc(buf, offset+chunk_size);
    }

    buf[offset] = '\0';     // properly end string array
    fclose(fp);
    printf("read: %ld bytes\n", strlen(buf));

    JSONNode *node = parse(buf);
    free(buf);
    return node;
}

/*
int main() {
    //JSONNode *json = parse_file("/home/eco/tmp/misc/example.json");
    //JSONNode *json = parse_file("/home/eco/tmp/misc/ledgerlive-logs-2021.04.09-22.40.04-b492eec.json");
    JSONNode *json = parse_file("example.json");
    if (json == NULL) {
        perr();
        return -1;
    }

    //JSONNode *json = parse(str);
    json->print_all(json);

    JSONNode* node = json->deep_get(json, 4, "quiz", "sport", "q1", "options");
    if (node == NULL) {
        perr();
        printf(">> node not found\n");
    } else {
        printf(">> found %s\n", node->key);
        print_all(node);
    }

}
*/
