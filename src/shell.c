#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"

// Default line buffer size before reallocating more memory
const size_t DEFAULT_BUFFER_SIZE = 1024;

// Default arg buffer count before reallocating more memory
const size_t DEFAULT_ARG_COUNT = 8;

const char* DELIMS = " \t\n\r";

/**
 * Evaluates the prompt to be displayed in the shell.
 */
char* eval_prompt(void) {
  // TODO: parse from a .config file
  return "\n\033[1mᗜˬᗜ > \033[0m";
}

/**
 * Reads a line from stdin, allocating memory as required.
 *
 * Returns a NULL-terminated string.
 */
char* sh_getline(void) {
  size_t buf_size = sizeof(char) * DEFAULT_BUFFER_SIZE;

  char* buf = malloc(buf_size);
  char c;
  int i = 0;

  if (!buf) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();
    if (c == '\0' || c == '\n') {
      buf[i++] = '\0';
      return buf;
    }
    else {
      buf[i++] = c;
    }

    // If buffer size exceeded, allocate more memory
    if (i >= buf_size) {
      buf_size += DEFAULT_BUFFER_SIZE;
      buf = realloc(buf, sizeof(char) * buf_size);
      
      if (!buf) {
        perror("realloc");
        exit(EXIT_FAILURE);
      }
    }
  } 
}

/**
 * Tokenises a whitespace-delimeted string into individual tokens (args).
 * Allocates memory (for the array of strings) as required.
 *
 * Returns a null terminated 'array' of strings (args).
 */
char** sh_getargs(char* line) {
  size_t buf_size = DEFAULT_ARG_COUNT;

  char** buf = malloc(sizeof(char*) * buf_size);
  char* token = strtok(line, DELIMS);
  int i = 0;

  if (!buf) {
    perror("buf");
    exit(EXIT_FAILURE);
  }

  while (token != NULL) {
    buf[i++] = token;

    if (i >= buf_size) {
      buf_size += DEFAULT_ARG_COUNT;
      buf = realloc(buf, sizeof(char*) * buf_size);

      if (!buf) {
        perror("realloc");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, DELIMS);
  }

  // Noting the array must be NULL-terminated for exec() to work
  buf[i] = NULL;
  return buf;
}

/**
 * Creates a child process to execute a program with given arguments.
 * 
 * args[0] is the name of the program to be executed, while args[1..n] are
 * optional args passed to that program.
 *
 * Returns 1 on success.
 */
int sh_exec(char** args) {
  pid_t pid;
  int status;

  if ((pid = fork()) == 0) {
    if (execvp(args[0], args) < 0) {
      if (errno == ENOENT) {
        fprintf(stderr, "koish: '%s' is not a valid command!\n", args[0]);
      }
      else {
        perror("execvp");
      }
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else {
    waitpid(pid, &status, WUNTRACED);
  }

  return 1;
}

/**
 * Handle processing of the argument string to determine if a built-in
 * command or program should be executed.
 *
 * If the first argument is a built-in command, it is executed.
 * Otherwise, it is a program and we call sh_exec() to execute it.
 *
 * The return value of the command/program is returned.
 */
int sh_process(char** args) {
  // If it is a built-in, execute it
  for (int i = 0; i < BUILTIN_COUNT; ++i) {
    if (strcmp(args[0], builtins[i]) == 0) {
      return exec_builtin[i](args);
    }
  }

  // Otherwise, we create a process and execute that program
  return sh_exec(args);
}

/**
 * Main loop for the shell; reads a line from stdin, tokenises and execute the
 * command/program.
 */
void sh_loop(void) {
  char* line;
  char** args;
  int status;

  do {
    printf("%s", eval_prompt());

    line = sh_getline();
    args = sh_getargs(line);
    status = sh_process(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char** argv) {
  sh_loop();
  return EXIT_SUCCESS;
}
