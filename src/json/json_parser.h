#ifndef JSON_H
#define JSON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>         // uint types
#include <string.h>         // uint types
#include <stdarg.h>         // variable amount of arguments

#include "perr.h"

typedef struct JSONNode JSONNode;
typedef struct Position Position;
typedef enum DType DType;

#define JRESET   "\x1B[0m"
#define JRED     "\x1B[31m"
#define JGREEN   "\x1B[32m"
#define JYELLOW  "\x1B[33m"
#define JBLUE    "\x1B[34m"
#define JMAGENTA "\x1B[35m"
#define JCYAN    "\x1B[36m"
#define JWHITE   "\x1B[37m"

#define LINES_CONTEXT 100

enum DType {
    //INVALID,
    INT,
    STR,
    ARR,
    OBJ,
    BOOL,
    FLOAT
};

struct JSONNode {
    char *key;
    void *value;            // void pointer needs to be casted to appropriate type
    DType dtype;            // record which data type the value needs to be casted to

    // arrays and objects are implemented as linked lists
    JSONNode *child;        // child of object
    JSONNode *parent;       // child of object
    JSONNode *prev;         // go to next child
    JSONNode *next;         // go to prev child

    // get typecasted value
    JSONNode* (*get_str)(JSONNode *self);
    JSONNode* (*get_int)(JSONNode *self);
    JSONNode* (*get_bool)(JSONNode *self);
    JSONNode* (*get_float)(JSONNode *self);
    JSONNode* (*get)(JSONNode *self, char *key);     // get key from object
    JSONNode* (*deep_get)(JSONNode *self, uint8_t length, ...);

    // recursively print all nodes
    void (*print_all)(JSONNode *self);

    // check flag for type
    uint8_t is_string;
    uint8_t is_int;
    uint8_t is_float;
    uint8_t is_object;
    uint8_t is_array;
    uint8_t is_bool;
};

struct Position {
    uint32_t pos;       // char counter
    uint32_t rows;      // row counter
    uint32_t cols;      // column counter
    char *json;         // full json string
    char *c;            // pointer to current char in json string
    char* (*next)(Position *self);  // move c to point to the next character
};
char* next(Position *self);

// keep track of position in json string
//Position *pos;

//uint32_t rows;            // global Y position in json string
//uint32_t cols;            // global Y position in json string

// public //
void print_all(JSONNode *node);
JSONNode* parse(char *json);                                // parse string and return json object
JSONNode* parse_file(char *path);                           // parse file contents
JSONNode* get(JSONNode *node, char *key);                   // find node by key
JSONNode* deep_get(JSONNode *node, uint8_t length, ...);    // deep find node by keys

// private //
int8_t read_kv(Position *pos, JSONNode *node);
int8_t read_value(Position *pos, JSONNode *node);

// used by read_value to detect type
char*  read_string(Position *pos);
int8_t read_array(Position *pos, JSONNode *node);
int8_t read_obj(Position *pos, JSONNode *node);
int8_t read_bool(Position *pos, JSONNode *node);
int8_t read_numeric(Position *pos, JSONNode *node);

void print_all_rec(JSONNode *node, uint8_t pos);
char skip_to_chr(Position *pos, char* list);
void get_spaces(char *buf, uint8_t spaces);
JSONNode* init_node();          // get a new node
void print_error(Position *pos, uint32_t amount);          // print context for error message


#endif
