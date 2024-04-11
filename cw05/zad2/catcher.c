#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define WORK_NUMBERS 1
#define WORK_PRINT_N_CHANGES 2
#define WORK_FINISH 3

int work_type_updates = 0;

void work_dispatch(int work_type) {
  switch (work_type) {
  case WORK_NUMBERS:
    for (int i = 1; i <= 100; i++) {
      printf("%d\n", i);
    }
    break;
  case WORK_PRINT_N_CHANGES:
    printf("work type changes: %d\n", work_type_updates);
    break;
  }
}

void sig_handler(int __attribute__((unused)) sig, siginfo_t *siginfo,
                 void __attribute__((unused)) * context) {
  printf("received from PID: %ld\n", (long)siginfo->si_pid);

  int work_type = siginfo->si_value.sival_int;
  work_dispatch(work_type);

  work_type_updates++;

  if (kill(siginfo->si_pid, SIGUSR1) == -1) {
    perror("could not sent confirmation to sender");
    exit(EXIT_FAILURE);
  }
  if (work_type == WORK_FINISH) {
    printf("exiting...\n");
    exit(EXIT_SUCCESS);
  }
}

int main(void) {
  pid_t pid = getpid();
  printf("catcher pid: %d\n", (int)pid);

  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = sig_handler;

  if (sigaction(SIGUSR1, &act, NULL) == -1) {
    perror("could not setup signal handling");
    return EXIT_FAILURE;
  }

  while (1)
    pause();

  return 0;
}
