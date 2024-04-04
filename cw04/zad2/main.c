#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int global = 0;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
    return EXIT_FAILURE;
  }

  char *progname = strrchr(argv[0], '/') + 1;
  printf("program name: %s\n", progname);
  char *dirname = argv[1];

  int local = 0;

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork has failed");
    return EXIT_FAILURE;
  } else if (pid == 0) {
    // in child process
    printf("child process\n");
    global++;
    local++;
    printf("child pid = %d, parent pid = %d\n", (int)getpid(), (int)getppid());
    printf("child's local = %d, child's global = %d\n", local, global);

    if (execl("/bin/ls", "ls", dirname, NULL) == -1) {
      perror("failed to execute /bin/ls");
      exit(EXIT_FAILURE);
    }
  } else {
    // in parent process
    int status = 0;
    wait(&status);
    int child_exit_status = WEXITSTATUS(status);

    printf("parent process\n");
    printf("parent pid = %d, child pid = %d\n", (int)getpid(), (int)pid);
    printf("Child exit code: %d\n", child_exit_status);
    printf("Parent's local = %d, parent's global = %d\n", local, global);
    return child_exit_status;
  }
}
