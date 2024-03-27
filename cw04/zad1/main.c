#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
    return EXIT_FAILURE;
  }

  char *endptr;
  errno = 0;
  long n_procs = strtol(argv[1], &endptr, 10);
  if (errno == ERANGE || *endptr != '\0') {
    fprintf(stderr, "invalid number of processes: %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  for (long i = 0; i < n_procs; i++) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork has failed");
      exit(EXIT_FAILURE);
    } else if (pid == 0) {
      printf("parent pid: %d, my pid: %d\n", (int)getppid(), (int)getpid());
      exit(EXIT_SUCCESS);
    }
  }

  for (long i = 0; i < n_procs; i++) {
    wait(NULL);
  }

  printf("number of processes: %ld\n", n_procs);
}
