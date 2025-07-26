#include <unistd.h>

/**
 * To add new built-in commands, include their header files and define
 * their name + function in builtins[] and builtin_funcs[] respectively.
 */
#include "cd.h"
#include "help.h"
#include "exit.h"

// Names and descriptions of built-in commands
char* builtins[] = {
  "help",
  "exit",
  "cd",
};

char* builtins_desc[] = {
  "display useful information",
  "exit the current shell instance",
  "change the current directory"
};


// Function pointers to built-in implementations
int (*exec_builtin[]) (char**) = {
  help,
  sh_exit,
  cd
};

int BUILTIN_COUNT = sizeof(builtins) / sizeof(char*);
