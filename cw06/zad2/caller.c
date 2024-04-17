#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "communication.h"

int main()
{
    int input_pipe;
    int output_pipe;

    if ((input_pipe = open(INPUT_PIPE_NAME, O_WRONLY)) < 0) {
        perror("Failed to open input pipe");
        return EXIT_FAILURE;
    }

    if ((output_pipe = open(OUTPUT_PIPE_NAME, O_RDONLY)) < 0) {
        perror("Failed to open input pipe");
        close(input_pipe);
        return EXIT_FAILURE;
    }

    while(1) {
        range_info r_info;
        if(scanf("%lf", &r_info.start) <= 0) {
            fprintf(stderr, "failed to read start range\n"); 
            break;
        }
        if(scanf("%lf", &r_info.stop) <= 0) {
            fprintf(stderr, "failed to read stop range\n"); 
            break;
        }
        if(scanf("%ld", &r_info.n_intervals) <= 0) {
            fprintf(stderr, "failed to read number of intervals\n"); 
            break;
        }

        if(r_info.n_intervals <= 0 || r_info.start >= r_info.stop) {
            fprintf(stderr, "invalid range\n");
            break;
        }

        if(write(input_pipe, &r_info, sizeof(range_info)) < 0) {
            perror("failed to write to input pipe");
            break;
        }

        double integral = 0;
        if(read(output_pipe, &integral, sizeof(integral)) < 0) {
            perror("failed to read from output pipe");
            break;
        }

        printf("Integral: %lf\n", integral);

    }

    close(input_pipe);
    close(output_pipe);

    return 0;
}
