#ifndef JSON_H
#define JSON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAX_BUF 500
#define LINES_CONTEXT 100

#define JRESET   "\x1B[0m"
#define JRED     "\x1B[31m"
#define JGREEN   "\x1B[32m"
#define JYELLOW  "\x1B[33m"
#define JBLUE    "\x1B[34m"
#define JMAGENTA "\x1B[35m"
#define JCYAN    "\x1B[36m"
#define JWHITE   "\x1B[37m"

typedef enum JSONDtype JSONDtype;
typedef struct JSONObject JSONObject;
typedef struct Position Position;

enum JSONDtype {
    JSON_STRING,
    JSON_FLOAT,
    JSON_OBJECT,
    JSON_BOOL,
    JSON_ARRAY,
    JSON_UNKNOWN
};

struct JSONObject {
    /* When object is the root object, parent is NULL */
    JSONObject* parent;

    JSONDtype dtype;

    // When object is child of array, the key is NULL
    char* key;

    // NOTE: value and children are mutually exclusive!

    /* In case of int/float/bool/string:
     *      -  value can be accessed by using one of the helper functions
     *      - children is NULL
     */
    void* value;

    /* In case of array/object:
     *      - the children is an array
     *      - array length is accessed by using jo->length attribute
     *      - children can be iterated as a linked list
     *      - value is NULL
     */
    JSONObject** children;

    // in case of object or array, this stores the array length
    uint32_t length;

    // If jsonobject is part of an array this gives access to its siblings
    JSONObject* next;
    JSONObject* prev;

    bool is_bool;
    bool is_string;
    bool is_float;
    bool is_int;
    bool is_array;
    bool is_object;
};


struct Position {
    uint32_t npos;      // char counter
    char *json;         // full json string
    char *c;            // pointer to current char in json string

    // row/col counter used to report position of json errors
    uint32_t rows;      
    uint32_t cols;

    // increment position in json string
    char* (*pos_next)(Position *self);
};

JSONObject* json_load(char* buf);
JSONObject* json_load_file(char *path);
void        json_print(JSONObject* jo);

// get value casted to the appropriate type
char*       json_get_string(JSONObject* jo);
float       json_get_float(JSONObject* jo);
JSONObject* json_get_array(JSONObject* jo);

// read json string
JSONObject* json_read(Position* pos);


//char*       pos_next(Position *pos);

JSONObject* json_object_init();

char fforward(Position* pos, char* search_lst, char* expected_lst, char* ignore_lst, char* buf);

#endif
