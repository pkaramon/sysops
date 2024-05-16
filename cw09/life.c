#define _XOPEN_SOURCE 500

#include <locale.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "grid.h"

typedef struct {
    char *foreground;
    char *background;
    int start;  // inclusive
    int end;    // exclusive
} thread_args_t;

void *compute_part_of_grid(void *t_args)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);

    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0) {
        fprintf(stderr, "could not setup mask");
        exit(EXIT_FAILURE);
    }

    thread_args_t args = *(thread_args_t *)t_args;
    free(t_args);

    while (true) {
        int sig;
        if (sigwait(&set, &sig) != 0) {
            fprintf(stderr, "sigwait failed\n");
            continue;
        }

        if (sig != SIGUSR1) {
            continue;
        }

        update_subgrid(args.foreground, args.background, args.start, args.end);

        char *tmp = args.foreground;
        args.foreground = args.background;
        args.background = tmp;
    }

    return NULL;
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <n_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n_threads = strtol(argv[1], NULL, 10);
    if (n_threads < 1 || n_threads > GRID_HEIGHT * GRID_WIDTH) {
        fprintf(stderr, "Invalid number of threads\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));
    setlocale(LC_CTYPE, "");
    initscr();  // Start curses mode

    char *foreground = create_grid();
    char *background = create_grid();

    init_grid(foreground);

    pthread_t *threads = malloc(sizeof(pthread_t) * n_threads);
    for (long i = 0; i < n_threads; i++) {
        thread_args_t *args = malloc(sizeof(thread_args_t));
        args->foreground = foreground;
        args->background = background;
        args->start = i * (GRID_WIDTH * GRID_HEIGHT) / n_threads;
        if (i == n_threads - 1) {
            args->end = GRID_WIDTH * GRID_HEIGHT;
        }
        else {
            args->end = (i + 1) * (GRID_WIDTH * GRID_HEIGHT) / n_threads;
        }

        if (pthread_create(&threads[i], NULL, compute_part_of_grid, args) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    while (true) {
        draw_grid(foreground);

        // Step simulation
        // Send signal to all threads
        for (long i = 0; i < n_threads; i++) {
            if (pthread_kill(threads[i], SIGUSR1) != 0) {
                fprintf(stderr, "could not send signal to thread %ld\n", i);
            }
        }

        usleep(500 * 1000);
    }

    free(threads);
    endwin();  // End curses mode
    destroy_grid(foreground);
    destroy_grid(background);

    return 0;
}
