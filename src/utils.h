#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>   // for non blocking sleep
#include <curses.h>
#include <errno.h>

#include "config.h"
#include "index_groups.h"
#include "curses_menu.h"


#define B_TL "┌"
#define B_TR "┐"
#define B_BL "└"
#define B_BR "┘"
#define B_H  '-'
#define B_V  '|'

#define SLEEP_CHECK_INTERVAL 10000

// custom assert macro with formatted message and ui cleanup
#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, M, ...) if(!(A)) {ui_cleanup() ; log_error(M, ##__VA_ARGS__); assert(A); }

// forward declare
typedef struct Group Group;


uint16_t get_rand(uint16_t lower, uint16_t upper);
int32_t map_dec_to_i(float value, float amin, float amax, float bmin, float bmax);
int32_t map(double value, double in_min, double in_max, uint32_t out_min, uint32_t out_max);

uint32_t find_nfrac(double f);
uint32_t find_nfrac2(double min, double max);
uint32_t find_nwhole(double f);
uint32_t count_digits(uint64_t n);

char* ts_to_dt(time_t t, char* fmt, char* buf, uint8_t buflen);

void die(char* msg);

void draw_border(WINDOW* w);
void fill_win(WINDOW* w, char c);
Group* fast_forward_groups(Group* g, uint32_t amount);

void debug(char* fmt, ...);
bool non_blocking_sleep(int interval, bool(*callback)(void* arg), void* arg);
char* str_to_lower(char* str);

double get_avg(double avg, uint32_t i, double value);
bool y_is_in_view(WINDOW* win, uint32_t iy);
void xassert(bool expr, char* fmt, ...);

void display_log_win();

#endif
