#define _XOPEN_SOURCE 700

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shared_memory_spec.h"

void destroy_printer_semaphores(printer_list_t* printer_list, long n_printers) {
    for (long i = 0; i < n_printers; i++) {
        if(sem_destroy(&printer_list->printers[i].sem) < 0) perror("sem_destroy");
    }
}

int setup_printer_list(printer_list_t *printer_list, long n_printers)
{
    printer_list->number_of_printers = n_printers;
    for (long i = 0; i < printer_list->number_of_printers; i++) {
        sem_t printer_sem;
        if(sem_init(&printer_sem, 1, 1) < 0) {
            perror("sem init");
            return i;
        }
        printer_t printer = {
            .state = PRINTER_IDLE,
            .sem = printer_sem,
            .buffer = {0}
        };

        printer_list->printers[i] = printer;
    }
    return printer_list->number_of_printers;
}


int main(int argc, char const *argv[]){
    shm_unlink(SHM_NAME);
    if(argc < 2) {
        fprintf(stderr, "correct usage: %s <number of printers>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n_printers = strtol(argv[1], NULL, 10);
    if(n_printers <= 0 || n_printers > MAX_PRINTERS) {
        fprintf(stderr, "invalid number of printers: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    int shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(shm_fd == -1) {
        perror("shm_open");
        return EXIT_FAILURE;
    }

    if(ftruncate(shm_fd, sizeof(printer_list_t))) {
        perror("ftruncate");
        shm_unlink(SHM_NAME);
        return EXIT_FAILURE;
    }

    printer_list_t *printer_list = mmap(NULL, sizeof(printer_list_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(printer_list == MAP_FAILED) {
        perror("mmap");
        shm_unlink(SHM_NAME);
        return EXIT_FAILURE;
    }

    long n_initialized = setup_printer_list(printer_list, n_printers);
    if(n_initialized != n_printers) {
        fprintf(stderr, "could not setup printers\n");
        destroy_printer_semaphores(printer_list, n_initialized);
        munmap(printer_list, sizeof(printer_list_t));
        shm_unlink(SHM_NAME);
        return EXIT_FAILURE;
    }



    for(long i = 0; i < printer_list->number_of_printers; i++) {
        pid_t pid = fork();
        if(pid < 0) {
            perror("fork");
            // TODO kill the remanining child processes
            destroy_printer_semaphores(printer_list, n_initialized);
            munmap(printer_list, sizeof(printer_list_t));
            shm_unlink(SHM_NAME);
            return EXIT_FAILURE;
        } else if(pid == 0) {
            while(1) {
                if(printer_list->printers[i].state == PRINTER_WORKING) {
                }
            }
        }

    }

    munmap(printer_list, sizeof(printer_list_t));
    shm_unlink(SHM_NAME);
    return EXIT_FAILURE;
}
