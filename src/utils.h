#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>


typedef struct {
    bool is_paused;
    bool is_stopped;
    bool is_pan_changed;

    int clicked_x;
    int clicked_y;

    int panx;
    int pany;
} State;


float get_avg(float avg, uint16_t i, float value);
uint16_t get_rand(uint16_t lower, uint16_t upper);
int32_t map_dec_to_i(float value, float amin, float amax, float bmin, float bmax);
uint32_t map(double value, double in_min, double in_max, uint32_t out_min, uint32_t out_max);


#endif
