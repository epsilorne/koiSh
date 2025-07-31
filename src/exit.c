#include <stdio.h>

/**
 * Returns 0, breaking the shell loop.
 */
int sh_exit(char** args) {
  fprintf(stderr, "koish: seeya!\n");
  return -1;
}
