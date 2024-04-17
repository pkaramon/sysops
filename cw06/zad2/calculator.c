#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "communication.h"


double integrate(range_info r_info)
{
    double integral = 0;
    double dx = (r_info.stop - r_info.start) / r_info.n_intervals;

    for (long i = 0; i < r_info.n_intervals; i++) {
        double x = r_info.start + i * dx;
        double fx = 4 / (1 + x * x);
        integral += fx;
    }
    return integral * dx;
}

void remove_pipes()
{
    unlink(INPUT_PIPE_NAME);
    unlink(OUTPUT_PIPE_NAME);
}

int main(int argc, char const *argv[])
{
    remove_pipes();
    if (mkfifo(INPUT_PIPE_NAME, S_IRWXU)) {
        perror("could not create input pipe");
        return EXIT_FAILURE;
    }
    if (mkfifo(OUTPUT_PIPE_NAME, S_IRWXU)) {
        perror("could not create output pipe");
        return EXIT_FAILURE;
    }

    int in_pipe = open(INPUT_PIPE_NAME, O_RDONLY);
    if (in_pipe < 0) {
        printf("Failed to open input pipe");
        remove_pipes();
        return EXIT_FAILURE;
    }

    int out_pipe = open(OUTPUT_PIPE_NAME, O_WRONLY);
    if (out_pipe < 0) {
        remove_pipes();
        close(in_pipe);
        return EXIT_FAILURE;
    }

    range_info range;
    while (read(in_pipe, &range, sizeof(range_info)) > 0) {
        double result = integrate(range);
        if (write(out_pipe, &result, sizeof(result)) < 0) {
            perror("could not write to output pipe");
            break;
        }
    }

    close(in_pipe);
    close(out_pipe);

    return EXIT_SUCCESS;
}
