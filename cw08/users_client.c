#define _XOPEN_SOURCE 700

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "shared_memory_spec.h"

volatile int keep_running = 1;

void sig_handler(int __attribute__((unused)) signum) {
    keep_running = 0;
}


void generate_random_text(char *buf, int size)
{
    for (int i = 0; i < size-1; i++) {
        buf[i] = (char)('a' + rand() % 26);
    }
    buf[size-1] = '\0';
}

void user_process(print_queue_t* queue, long user_id) {
    srand(time(NULL) + user_id);
    printf("user %ld is ready\n", user_id);
    while(keep_running) {
        char buf[PRINTER_BUFFER_SIZE];
        generate_random_text(buf, PRINTER_BUFFER_SIZE);

        sem_wait(&queue->empty);
        sem_wait(&queue->mutex);

        memcpy(queue->jobs[queue->tail], buf, PRINTER_BUFFER_SIZE);
        queue->tail = (queue->tail+1) % MAX_JOBS;

        sem_post(&queue->mutex);
        sem_post(&queue->full);

        printf("user %ld sent %s\n", user_id, buf);
        sleep(1 + rand() % 4);
    }
}

int main(int argc, char const* argv[])
{
    for (int i = 0; i < SIGRTMAX; i++) {
        signal(i, sig_handler);
    }

    if (argc < 2) {
        fprintf(stderr, "correct usage: %s <number of users>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n_users = strtol(argv[1], NULL, 10);
    if (n_users <= 0) {
        fprintf(stderr, "invalid number of users: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    int shm_fd = shm_open(SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("shm_open");
        return EXIT_FAILURE;
    }

    print_queue_t* printer_queue =
        mmap(NULL, sizeof(print_queue_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (printer_queue == MAP_FAILED) {
        perror("mmap");
        shm_unlink(SHM_NAME);
        return EXIT_FAILURE;
    }

    pid_t* pids = calloc(n_users,  sizeof(pid_t));
    for (long i = 0; i < n_users; i++)
    {
        int pid = fork();
        if(pid < 0) {
            perror("fork");
        } else if(pid == 0) {
            user_process(printer_queue, i);
        } else {
            pids[i] = pid;
        }
    }
    

    while(keep_running)
        ;

    for (long i = 0; i < n_users; i++)
    {
        if(pids[i] != 0) {
            kill(pids[i], SIGKILL);
        }
    }

    free(pids);
    munmap(printer_queue, sizeof(print_queue_t));
    return 0;
}
