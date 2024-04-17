#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const double range_start = 0.0;
const double range_stop = 1.0;

double calculate_part(double start, double dx, long steps)
{
    double area = 0.0;
    for (long i = 0; i < steps; i++) {
        double x = start + i * dx;
        double fx = 4.0 / (x * x + 1);
        area += fx * dx;
    }
    return area;
}

int distribute_calculations(long n_processes, long n_intervals, double dx, int *pipe_fds)
{
    long steps_per_process = n_intervals / n_processes;
    for (long i = 0; i < n_processes; i++) {
        int pid = fork();
        if (pid == -1) {
            perror("fork");
            return -1;
        }
        else if (pid == 0) {
            close(pipe_fds[2 * i]);

            double result =
                calculate_part(range_start + i * steps_per_process * dx, dx,
                               i == n_processes - 1 ? steps_per_process + n_intervals % n_processes
                                                    : steps_per_process);

            if (write(pipe_fds[2 * i + 1], &result, sizeof(result)) < 0) {
                fprintf(stderr, "failed to write to the pipe\n");
                return -1;
            }

            exit(0);
        }
        else {
            close(pipe_fds[2 * i + 1]);
        }
    }
}

int write_time_to_file(double elapsed_time, double dx, long n_processes)
{
    FILE *fp;
    const char *filename = "time-raport.txt";

    fp = fopen(filename, "a");
    if (fp == NULL) {
        perror("Error opening file");
        return -1;
    }

    if (fprintf(fp, "dx: %.10lf, n: %ld, time: %.10lf\n", dx, n_processes, elapsed_time) < 0) {
        perror("Error writing to file");
        fclose(fp);
        return -1;
    }

    if (fclose(fp) != 0) {
        perror("Error closing file");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <dx> <n_processes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    double dx = strtod(argv[1], NULL);
    long n_processes = strtol(argv[2], NULL, 10);

    if (n_processes <= 0) {
        fprintf(stderr, "invalid number of processes %ld\n", n_processes);
        return EXIT_FAILURE;
    }

    if (dx <= 0) {
        fprintf(stderr, "invalid interval width %lf\n", dx);
        return EXIT_FAILURE;
    }

    long n_intervals = (long)ceil((range_stop - range_start) / dx);
    if (n_intervals < n_processes) {
        fprintf(stderr, "too many processes for given dx\n");
        return EXIT_FAILURE;
    }

    int *pipe_fds = calloc(n_processes, 2 * sizeof(int));

    for (long i = 0; i < n_processes; i++) {
        if (pipe(pipe_fds + 2 * i) == -1) {
            fprintf(stderr, "could not create a pipe");
            free(pipe_fds);
            return EXIT_FAILURE;
        }
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (distribute_calculations(n_processes, n_intervals, dx, pipe_fds) == -1) {
        fprintf(stderr, "could not calculate the integral\n");
        free(pipe_fds);
        return EXIT_FAILURE;
    }

    double sum = 0;
    for (long i = 0; i < n_processes; i++) {
        double result = 0;
        if (read(pipe_fds[2 * i], &result, sizeof(result)) < 0) {
            fprintf(stderr, "count not read from pipe");
            free(pipe_fds);
            return EXIT_FAILURE;
        }
        sum += result;
    }
    free(pipe_fds);

    gettimeofday(&end, NULL);

    printf("final result = %lf\n", sum);

    double time_taken;
    time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;

    if (write_time_to_file(time_taken, dx, n_processes) == -1) {
        fprintf(stderr, "writing to file failed\n");
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
