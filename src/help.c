#include <stdio.h>

#include "version.h"
#include "builtin.h"

/**
 * Display an informative prompt with all built-in commands.
 */
int help(char** args) {
  printf("\033[32m");
  printf("                   ,,              ,,        \n");
  printf(" 7MM               db   .M\"\"\"bgd  7MM        \n");
  printf("  MM                   ,MI    \"Y   MM        \n");
  printf("  MM  ,MP',pW\"Wq. 7MM   MMb.       MMpMMMb.  \n");
  printf("  MM ;Y  6W'   `Wb MM    `YMMNq.   MM    MM  \n");
  printf("  MM;Mm  8M     M8 MM  .     `MM   MM    MM  \n");
  printf("  MM `Mb.YA.   ,A9 MM  Mb     dM   MM    MM  \n");
  printf(".JMML. YA.`Ybmd9'.JMML.P\"Ybmmd\"  .JMML  JMML.\n");
  printf("\033[0m");

  printf("----------\n");
  printf("koiSh v%s\n\n", VERSION);
  printf("Execute a program by entering its name.\n");
  printf("Otherwise, the following commands are built-in:\n\n");

  for (int i = 0; i < BUILTIN_COUNT; ++i) {
    printf("  %s\t\t%s\n", builtins[i], builtins_desc[i]);
  }

  printf("\nYou can also use `man <program_name>` for specific information on a program.\n");

  return 1;
}
