#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int traverse_directory(const char *dirpath, DIR *dirp) {
  if (chdir(dirpath) == -1) {
    perror("cannot cd into passed directory");
    return EXIT_FAILURE;
  }

  struct dirent *entry;
  struct stat fileinfo;
  long long all_files_size = 0;
  while ((entry = readdir(dirp)) != NULL) {
    if (stat(entry->d_name, &fileinfo) == -1) {
      perror("stat failed");
      return EXIT_FAILURE;
    }
    if (S_ISDIR(fileinfo.st_mode))
      continue;

    printf("%s %ld\n", entry->d_name, fileinfo.st_size);
    all_files_size += fileinfo.st_size;
  }
  printf("total size: %lld\n", all_files_size);
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  char *dirpath = NULL;
  if (argc == 1) {
    dirpath = ".";
  } else if (argc == 2) {
    dirpath = argv[1];
  } else {
    fprintf(stderr, "Wrong number of arguments %d\n", argc);
    return EXIT_FAILURE;
  }

  DIR *dirp = opendir(dirpath);
  if (dirp == NULL) {
    perror("Could not open dir");
    return EXIT_FAILURE;
  }

  if (traverse_directory(dirpath, dirp) != EXIT_SUCCESS) {
    fprintf(stderr, "could not successfully traverse the directory\n");
  }

  closedir(dirp);
  return EXIT_SUCCESS;
}
