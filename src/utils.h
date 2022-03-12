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

#include "index_groups.h"


#define B_TL "┌"
#define B_TR "┐"
#define B_BL "└"
#define B_BR "┘"
#define B_H  '-'
#define B_V  '|'

#define SLEEP_CHECK_INTERVAL 10000
#define MAX_SYMBOL_SIZE 10

typedef struct Group Group;


float get_avg(float avg, uint16_t i, float value);
uint16_t get_rand(uint16_t lower, uint16_t upper);
int32_t map_dec_to_i(float value, float amin, float amax, float bmin, float bmax);
int32_t map(double value, double in_min, double in_max, uint32_t out_min, uint32_t out_max);

uint32_t find_nfrac(double f);
uint32_t find_nwhole(double f);
uint32_t count_digits(uint64_t n);

char* ts_to_dt(time_t t, char* fmt, char* buf, uint8_t buflen);

void die(char* msg);

void draw_border(WINDOW* w);
void fill_win(WINDOW* w, char c);
Group* fast_forward_groups(Group* g, uint32_t amount);

void debug(char* fmt, ...);
bool non_blocking_sleep(int interval, bool(*callback)(void* arg), void* arg);

#endif
