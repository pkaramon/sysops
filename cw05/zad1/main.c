#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void handler(int sig_no) { printf("received a signal with no: %d\n", sig_no); }

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(
        stderr,
        "invalid number of arguments: ./signals none|ignore|handler|mask\n");
    return EXIT_FAILURE;
  }

  char *react_t = argv[1];
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if (strcmp(react_t, "none") == 0) {
    act.sa_handler = SIG_DFL;
  } else if (strcmp(react_t, "ignore") == 0) {
    act.sa_handler = SIG_IGN;
  } else if (strcmp(react_t, "handler") == 0) {
    act.sa_handler = handler;
  } else if (strcmp(react_t, "mask") == 0) {
    sigset_t mask;
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
      perror("cuold not setup mask");
      return EXIT_FAILURE;
    }
  } else {
    fprintf(stderr, "unknown reaction type: %s\n", react_t);
    return EXIT_FAILURE;
  }

  if (strcmp(react_t, "mask") != 0 && sigaction(SIGUSR1, &act, NULL) == -1) {
    perror("could not setup signal handling");
    return EXIT_FAILURE;
  }

  if (raise(SIGUSR1) != 0) {
    fprintf(stderr, "could not raise signal\n");
    return EXIT_FAILURE;
  }

  if (strcmp(react_t, "mask") == 0) {
    sigset_t pending_signals;
    if (sigpending(&pending_signals) == -1) {
      perror("could not get pending signals");
      return EXIT_FAILURE;
    }
    printf("SIGUSR1 is pending: %s\n",
           sigismember(&pending_signals, SIGUSR1) ? "yes" : "no");
  }

  return EXIT_SUCCESS;
}
