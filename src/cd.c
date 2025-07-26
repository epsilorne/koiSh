#include <stdio.h>
#include <unistd.h>

/**
 * cd implementation.
 */
int cd(char** args) {
  if (args[1] == NULL) {
    fprintf(stderr, "koish: expected a path for 'cd'!\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("cd");
    }
  }
  return 1;
}
