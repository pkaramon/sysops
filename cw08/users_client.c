#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    if(argc < 2) {
        fprintf(stderr, "correct usage: %s <number of users>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n_users = strtol(argv[1], NULL, 10);
    if(n_users <= 0) {
        fprintf(stderr, "invalid number of users: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    
    return 0;
}
