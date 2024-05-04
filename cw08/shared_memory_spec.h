#pragma once

#include <semaphore.h>

#define PRINTER_BUFFER_SIZE 10
#define SHM_NAME "/shm_printers_server"
#define MAX_PRINTERS 50

typedef enum { PRINTER_IDLE, PRINTER_WORKING } printer_state_t;

typedef struct {
    sem_t sem;
    char buffer[PRINTER_BUFFER_SIZE];
    printer_state_t state;
} printer_t;

typedef struct {
    printer_t printers[MAX_PRINTERS];
    long number_of_printers;
} printer_list_t;


