#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void sig_handler(int sig_no) {
  printf("Confirmation received, signal number: %d\n", sig_no);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "not enough arguments, usage: ./sender catcherPID 1|2|3\n");
    return EXIT_FAILURE;
  }

  char *endptr;
  errno = 0;
  long catcher_pid = strtol(argv[1], &endptr, 10);
  if (errno == ERANGE || *endptr != '\0') {
    fprintf(stderr, "invalid catcher id: %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  int work_type = atoi(argv[2]);
  if (!(1 <= work_type && work_type <= 3)) {
    fprintf(stderr, "invalid work type: %s\n", argv[2]);
    return EXIT_FAILURE;
  }

  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = sig_handler;
  if (sigaction(SIGUSR1, &act, NULL) == -1) {
    perror("could not setup signal handler");
    return EXIT_FAILURE;
  }

  union sigval value;
  value.sival_int = work_type;

  if (sigqueue((pid_t)catcher_pid, SIGUSR1, value) == -1) {
    perror("could not send send to catcher");
    return EXIT_FAILURE;
  }

  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGUSR1);
  sigdelset(&mask, SIGINT);

  if (sigsuspend(&mask) == -1 && errno == EINTR) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
