#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"
#include "plist.h"

// Default line buffer size before reallocating more memory
#define DEFAULT_BUFFER_SIZE 1024;

// Default arg buffer count before reallocating more memory
#define DEFAULT_ARG_COUNT 8;

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
  int i = 0, offset = 0;

  if (!buf) {
    perror("buff");
    exit(EXIT_FAILURE);
  }

  while (token) {
    /* The idea is to have an 'offset' representing where the argv starts
     * for a specific process_t struct. exec() requires the argv array to be
     * NULL-terminated, so we can set that and move the 'offset' AFTER the
     * terminator. For example, if we had something like:
     *
     * ls ; clear
     * ^
     * offset
     *
     * We would move the offset to...
     *
     * 'ls' '\0' 'clear'
     *            ^
     *            offset
     *
     */
    if (token[0] == ';') {
      buf[i++] = NULL;
      add_to_plist(0, buf + offset);
      offset = i;
    }
    else {
      buf[i++] = token;
    }

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

  buf[i] = NULL;
  add_to_plist(0, buf + offset);

  return buf;
}

/**
 * Create a child process -- given a process_t struct -- and executes it.
 *
 * Returns 1 on success.
 */
int sh_exec(process_t* p) {
  pid_t pid;
  int status;

  if ((pid = fork()) == 0) {
    if (execvp(p->argv[0], p->argv) < 0) {
      if (errno == ENOENT) {
        fprintf(stderr, "koish: '%s' is not a valid command!\n", p->argv[0]);
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
    // Update process struct pid with the child's pid
    p->pid = pid;
    waitpid(pid, &status, WUNTRACED);
  }

  return 1;
}

/**
 * Given a process_t, check if it represents a builtin command.
 *
 * Returns the index of the command, otherwise -1 if it is not a builtin.
 */
int check_builtin(process_t* p, int* status) {
  for (int i = 0; i < BUILTIN_COUNT; ++i) {
    if (strcmp(p->argv[0], builtins[i]) == 0) {
      return i;
    }
  }
  return -1;
}

/**
 * Handle processing of the process_t list to determine if a built-in
 * command or program should be executed.
 *
 * If the first argument is a built-in command, it is executed.
 * Otherwise, it is a program and we call sh_exec() to execute it.
 *
 * The return value of the latest command/program is returned.
 */
int sh_process(void) {
  process_t* curr_p = HEAD;
  int status = 1;
  int builtin_idx;

  while (curr_p) {
    char** args = curr_p->argv;

    if (!args[0]) {
      fprintf(stderr, "koish: did you mean to say something?\n");
    }
    /* Check if the command is a builtin and if it is, execute it. Otherwise,
    we create a process to execute the program. */
    else if ((builtin_idx = check_builtin(curr_p, &status)) != -1) {
      status = exec_builtin[builtin_idx](curr_p->argv);
    }
    else {
      status = sh_exec(curr_p);
      curr_p->status = status;
    }
    curr_p = curr_p->next;
  }

  return status;
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
    status = sh_process();

    free(line);
    free(args);
    free_plist();
  } while (status);
}

int main(int argc, char** argv) {
  sh_loop();
  return EXIT_SUCCESS;
}
