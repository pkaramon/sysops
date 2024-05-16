#define _XOPEN_SOURCE 700

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "shared_memory_spec.h"

volatile int keep_running = 1;

void sig_handler(int __attribute__((unused)) signum) { keep_running = 0; }

int setup_printer_queue(print_queue_t* queue)
{
    sem_t mutex, empty, full;
    if (sem_init(&mutex, 1, 1) < 0) {
        return -1;
    }
    if (sem_init(&empty, 1, MAX_JOBS) < 0) {
        sem_destroy(&mutex);
        return -1;
    }
    if (sem_init(&full, 1, 0) < 0) {
        sem_destroy(&mutex);
        sem_destroy(&empty);
        return -1;
    }

    print_queue_t q = {
        .head = 0, .tail = 0, .jobs = {{0}}, .mutex = mutex, .empty = empty, .full = full};
    *queue = q;
    return 0;
}

void clean_queue(print_queue_t* queue)
{
    if (sem_destroy(&queue->mutex) < 0) perror("sem_close mutex");
    if (sem_destroy(&queue->empty) < 0) perror("sem_close empty");
    if (sem_destroy(&queue->full) < 0) perror("sem_close full");
}

void printer_process(print_queue_t* queue, long printer_id)
{
    printf("printer %ld is ready \n", printer_id);
    while (keep_running) {
        sem_wait(&queue->full);
        sem_wait(&queue->mutex);

        char job[PRINTER_BUFFER_SIZE] = {0};

        memcpy(job, queue->jobs[queue->head], PRINTER_BUFFER_SIZE);
        queue->head = (queue->head + 1) % MAX_JOBS;

        sem_post(&queue->mutex);
        sem_post(&queue->empty);

        printf("printer %ld got job %s\n", printer_id, job);

        for (int i = 0; i < PRINTER_BUFFER_SIZE - 1; i++) {
            printf("%c", job[i]);
            fflush(stdout);
            sleep(1);
        }
        printf("\n");
    }
}

int main(int argc, char const* argv[])
{
    for (int i = 1; i < SIGRTMAX; i++) {
        signal(i, sig_handler);
    }

    if (argc < 2) {
        fprintf(stderr, "correct usage: %s <number of printers>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n_printers = strtol(argv[1], NULL, 10);
    if (n_printers <= 0 || n_printers > MAX_PRINTERS) {
        fprintf(stderr, "invalid number of printers: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    int shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("shm_open");
        return EXIT_FAILURE;
    }

    if (ftruncate(shm_fd, sizeof(print_queue_t))) {
        perror("ftruncate");
        shm_unlink(SHM_NAME);
        return EXIT_FAILURE;
    }

    print_queue_t* printer_queue =
        mmap(NULL, sizeof(print_queue_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (printer_queue == MAP_FAILED) {
        perror("mmap");
        shm_unlink(SHM_NAME);
        return EXIT_FAILURE;
    }

    if (setup_printer_queue(printer_queue) < 0) {
        fprintf(stderr, "could not create printer queue\n");
        clean_queue(printer_queue);
        munmap(printer_queue, sizeof(print_queue_t));
        shm_unlink(SHM_NAME);
        return EXIT_FAILURE;
    }

    pid_t* pids = calloc(n_printers, sizeof(pid_t));
    for (long i = 0; i < n_printers; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
        }
        else if (pid == 0) {
            printer_process(printer_queue, i);
        }
        else {
            pids[i] = pid;
        }
    }

    while (keep_running);

    for (long i = 0; i < n_printers; i++) {
        if (pids[i] != 0) {
            kill(pids[i], SIGKILL);
        }
    }

    free(pids);
    clean_queue(printer_queue);
    munmap(printer_queue, sizeof(print_queue_t));
    shm_unlink(SHM_NAME);
    return EXIT_FAILURE;
}
