#include "collatz.h"
#include <dlfcn.h>
#include <stdio.h>

#ifdef USE_LIB_DYNAMIC
int main() {
  void *handle = dlopen("libcollatz.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "libcollatz.so could not be found\n");
  }

  int (*tcc)(int, int);
  tcc = (int (*)(int, int))dlsym(handle, "test_collatz_convergence");

  if(dlerror() != NULL) {
      fprintf(stderr, "Error when loading test_collatz_convergence");
  }

  for (int i=1; i < 10; i++) {
      printf("%d, steps: %d\n", i, tcc(i, 100));
  }
  dlclose(handle);
  return 0;
}
#else
int main() {
  for (int i = 1; i < 10; i++) {
    printf("%d, steps: %d\n", i, test_collatz_convergence(i, 1000));
  }
  return 0;
}
#endif
