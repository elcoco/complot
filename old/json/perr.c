#include "perr.h"

// thread local variable stores latest error number
__thread Err error_no = ERR_NOERROR;

static const char *err_str[] = {
    "No error",
    "Unknown error",
    "Syntax error in JSON string",
    "Syntax error while reading bool",
    "Syntax error while reading string",
    "Syntax error while reading array",
    "Syntax error while reading object",
    "Syntax error while reading number",
    "Wrong type"

};


// set error number
void set_err(Err error) {
    error_no = error;
}

// reset error
void clear_err() {
    set_err(ERR_NOERROR);
}

// print latest error message and reset error_no
void perr() {
    fprintf(stderr, "Error: %s\n", err_str[error_no]);
    set_err(ERR_NOERROR);
}

// print custom message and latest error message and reset error_no
void pcerr(char *msg) {
    fprintf(stderr, "%s: %s\n", msg, err_str[error_no]);
    set_err(ERR_NOERROR);
}
