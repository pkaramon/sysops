#include <stdio.h>
#include <stdlib.h>

#ifdef USE_LIB_DYNAMIC
#include <dlfcn.h>
int (*collatz_conjecture)(int input);
int (*test_collatz_convergence)(int input, int max_iter);
#else
#include "collatz/collatz.h"
#endif

int main() {
#ifdef USE_LIB_DYNAMIC
  void *handle = dlopen("libcollatz_shared.so", RTLD_LAZY);

  if (!handle) {
    fprintf(stderr, "library could not be loaded: %s\n", dlerror());
    exit(EXIT_FAILURE);
  }

  test_collatz_convergence = dlsym(handle, "test_collatz_convergence");

  if (dlerror() != NULL) {
    fprintf(stderr, "Error when loading test_collatz_convergence\n");
    dlclose(handle);
    exit(EXIT_FAILURE);
  }

  collatz_conjecture = dlsym(handle, "collatz_conjecture");
  if (dlerror() != NULL) {
    fprintf(stderr, "Error when loading collatz_conjecture\n");
    dlclose(handle);
    exit(EXIT_FAILURE);
  }
#endif

  for (int i = 1; i < 10; i++) {
    printf("%d, steps: %d\n", i, test_collatz_convergence(i, 19));
  }
  printf("collatz_conjecture(5) = %d\n", collatz_conjecture(5));

#ifdef USE_LIB_DYNAMIC
  dlclose(handle);
#endif
  return 0;
}
