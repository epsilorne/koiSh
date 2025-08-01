#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"
#include "proc.h"
#include "shell.h"
#include "tokeniser.h"

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
 * Given an input process_t, create a child process to execute the program with
 * argument(s) specified.
 *
 * Returns 1 if exec() did not fail.
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
 * Check if an input process_t represents a builtin command.
 *
 * Returns the index of the command if it is builtin, otherwise -1.
 */
int check_builtin(process_t* p) {
  for (int i = 0; i < BUILTIN_COUNT; ++i) {
    if (strcmp(p->argv[0], builtins[i]) == 0) {
      return i;
    }
  }
  return -1;
}

/**
 * Handle processing of the process_t list to sequentially execute processes.
 *
 * If the command is determined to be a builtin, the specific builtin will be
 * executed. Otherwise, it is a program and we call sh_exec() to manage it.
 *
 * The 'status' of the latest command is returned, which is typically 1, but 0
 * when the exit() builtin is called.
 */
int sh_process(void) {
  // TODO: have 'HEAD' be defined here instead of proc.c
  process_t* curr_p = HEAD;
  int status = 1;
  int builtin_idx;

  while (curr_p && status > 0) {
    char** args = curr_p->argv;

    if (!args[0]) {
      fprintf(stderr, "koish: did you mean to say something?\n");
    }
    /* Check if the command is a builtin and if it is, execute it. Otherwise,
    we create a process to execute the program. */
    else if ((builtin_idx = check_builtin(curr_p)) != -1) {
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
 * command/program. Will break loop when status <= 0, i.e. exit() was called.
 */
void sh_loop(void) {
  char* line;
  char** args;
  int status;

  do {
    printf("%s", eval_prompt());

    line = sh_getline();
    args = sh_tokenise(line);
    status = sh_process();

    free(line);
    free(args);
    free_plist();
  } while (status > 0);
}

int main(int argc, char** argv) {
  sh_loop();
  return EXIT_SUCCESS;
}
