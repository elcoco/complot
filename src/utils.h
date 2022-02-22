#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <curses.h>


#define B_TL "┌"
#define B_TR "┐"
#define B_BL "└"
#define B_BR "┘"
#define B_H  '-'
#define B_V  '|'

typedef struct {
    bool is_paused;
    bool is_stopped;
    bool is_pan_changed;

    int clicked_x;
    int clicked_y;

    int panx;
    int pany;

    bool set_autorange;

    // data dimensions, used for zooming and autorange etc...
    double dmin;
    double dmax;
    bool fit_all;
    int32_t gsize;
} State;


float get_avg(float avg, uint16_t i, float value);
uint16_t get_rand(uint16_t lower, uint16_t upper);
int32_t map_dec_to_i(float value, float amin, float amax, float bmin, float bmax);
uint32_t map(double value, double in_min, double in_max, uint32_t out_min, uint32_t out_max);

uint32_t find_nfrac(double f);
uint32_t find_nwhole(double f);
uint32_t count_digits(uint64_t n);

char* ts_to_dt(time_t t, char* fmt, char* buf, uint8_t buflen);

void die(char* msg);

void draw_border(WINDOW* w, unsigned int lines, unsigned int cols);
void fill_win(WINDOW* w, unsigned int lines, unsigned int cols, char c);

#endif
