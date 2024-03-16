#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef BLOCK_SIZE

/*
** reverses the infile byte by byte and puts the result into outfile
*/
int reverse_file(FILE *infile, FILE *outfile) {
  if (fseek(infile, -1, SEEK_END) != 0) {
    perror("Could not seek the input file");
    return EXIT_FAILURE;
  }

  long bytes_remaninig = ftell(infile) + 1;
  while (bytes_remaninig > 0) {
    int c = fgetc(infile);
    if (ferror(infile)) {
      perror("Could not read from infile");
      return EXIT_FAILURE;
    }
    if (fputc(c, outfile) == EOF) {
      perror("Failed to write to file");
    }
    fseek(infile, -2, SEEK_CUR);
    bytes_remaninig--;
  }

  return EXIT_SUCCESS;
}
#else
void reverse_text(char *text, size_t length) {
  for (size_t i = 0; i < length / 2; i++) {
    char temp = text[i];
    text[i] = text[length - i - 1];
    text[length - i - 1] = temp;
  }
}

/*
** reverses the infile in blocks of size BLOCK_SIZE, puts the result
** into outfile
*/
int reverse_file(FILE *infile, FILE *outfile) {
  if (fseek(infile, 0, SEEK_END) != 0) {
    perror("Could not seek the input file");
    return EXIT_FAILURE;
  }

  long bytes_remaining = ftell(infile);
  char buf[BLOCK_SIZE];
  fseek(infile, bytes_remaining % BLOCK_SIZE * (-1), SEEK_END);

  while (bytes_remaining > 0) {
    size_t bytes_read = fread(buf, sizeof(char), BLOCK_SIZE, infile);
    if (ferror(infile)) {
      perror("Error when reading input file");
      return EXIT_FAILURE;
    }
    bytes_remaining -= bytes_read;

    reverse_text(buf, bytes_read);
    size_t written_bytes = fwrite(buf, sizeof(char), bytes_read, outfile);
    if (written_bytes < bytes_read) {
      perror("Failed to write to output file");
      return EXIT_FAILURE;
    }

    fseek(infile, -BLOCK_SIZE - bytes_read, SEEK_CUR);
  }

  return EXIT_SUCCESS;
}
#endif

int validate_number_of_arguments(int argc) {
  int n_args = argc - 1;
  if (n_args != 2) {
    fprintf(stderr,
            "invalid number of arguments: %d, provide paths to two files\n",
            n_args);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int main(int argc, char const *argv[]) {
  if (validate_number_of_arguments(argc) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  FILE *infile = fopen(argv[1], "r");
  if (infile == NULL) {
    perror("Could not open the input file");
    return EXIT_FAILURE;
  }

  FILE *outfile = fopen(argv[2], "w");
  if (outfile == NULL) {
    perror("Could not open the output file");
    fclose(infile);
    return EXIT_FAILURE;
  }

  int exit_status = reverse_file(infile, outfile);

  fclose(infile);
  fclose(outfile);
  if (exit_status != 0) {
    fprintf(stderr, "Reversing the input fail failed\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}