#pragma once

#include <semaphore.h>

#define PRINTER_BUFFER_SIZE 3
#define SHM_NAME "/shm_printers_server"
#define MAX_PRINTERS 50
#define MAX_JOBS 10

typedef struct {
    char jobs[MAX_JOBS][PRINTER_BUFFER_SIZE];
    int head;
    int tail;
    sem_t mutex;
    sem_t empty;
    sem_t full;
} print_queue_t;
