#pragma once

typedef struct {
    double start;
    double stop;
    long n_intervals;
} range_info;

#define INPUT_PIPE_NAME "integral_input"
#define OUTPUT_PIPE_NAME "integral_output"