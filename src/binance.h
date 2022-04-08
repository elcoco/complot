#ifndef BINANCE_H
#define BINANCE_H

#include "request.h"
#include "json.h"

void* binance_read_thread(void* thread_args);
char** do_binance_symbols_req();

#endif
