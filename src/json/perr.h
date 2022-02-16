#ifndef PERR_H
#define PERR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>       
#include <string.h>        


enum Err {
    ERR_NOERROR,
    ERR_UNKNOWN,
    ERR_SYNTAX,
    ERR_BOOL,
    ERR_STR,
    ERR_ARR,
    ERR_OBJ,
    ERR_NUM,
    ERR_WRONGTYPE
};

typedef enum Err Err;




void set_err(Err error);   // set error number
void clear_err();               // reset error
void perr();                    // print latest error message and reset error_no
void pcerr(char *msg);          // print custom message and latest error message and reset error_no

#endif
