#include "collatz.h"

int collatz_conjecture(int input) {
  if (input % 2 == 0) {
    return input / 2;
  } else {
    return 3 * input + 1;
  }
}

int test_collatz_convergence(int input, int max_iter) {
  int iter_count = 0;
  while (input != 1 && iter_count < max_iter) {
    input = collatz_conjecture(input);
    iter_count++;
  }

  if (input != 1 && iter_count == max_iter) {
    return -1;
  }

  return iter_count;
}
