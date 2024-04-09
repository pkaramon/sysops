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

  char *reaction_type = argv[1];

  if (strcmp(reaction_type, "none") == 0) {
    signal(SIGUSR1, SIG_DFL);
  } else if (strcmp(reaction_type, "ignore") == 0) {
    signal(SIGUSR1, SIG_IGN);
  } else if (strcmp(reaction_type, "handler") == 0) {
    signal(SIGUSR1, handler);
  } else if (strcmp(reaction_type, "mask") == 0) {
  } else {
    fprintf(stderr, "unknown reaction type: %s\n", reaction_type);
    return EXIT_FAILURE;
  }

  raise(SIGUSR1);

  return EXIT_SUCCESS;
}
