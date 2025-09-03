#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"
#include "shell.h"
#include "tokeniser.h"
#include "job.h"

extern job_t* JOB_HEAD;

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
 * Check if an input process_t represents a builtin command.
 *
 * NOTE: the input argv is assumed to 'belong' to the input process_t, i.e. the
 * offset has already been applied.
 *
 * Returns the index of the command if it is builtin, otherwise -1.
 */
int check_builtin(process_t* p, char** argv) {
  for (int i = 0; i < BUILTIN_COUNT; ++i) {
    if (strcmp(argv[0], builtins[i]) == 0) {
      return i;
    }
  }
  return -1;
}

/*
 * TODO: redo doco
*/
int sh_process(char** argv) {
  job_t* j = JOB_HEAD;
  int n, builtin_idx, status = -1;
  pid_t pid;
  char** p_args;

  // Iterate through each job in the linked list
  for (;j && status; j = j->next) {
    process_t* p = j->tasks;
    n = j->n_tasks;

    // Iterate through the tasks associated with this job
    for(int i = 0; i < n && p; ++i, p = p->next) {
      p_args = argv + p->argv_offset;

      if (!p_args[0]) {
        fprintf(stderr, "koish: did you mean to say something?\n");
      }
      else if ((builtin_idx = check_builtin(p, p_args)) != -1) {
        if ((status = exec_builtin[builtin_idx](p_args)) <= 0) break;
      }
      else {
        if ((pid = fork()) < 0) {
          perror("fork");
          exit(EXIT_FAILURE);
        }

        // Child process will run the program specified
        else if (pid == 0) {
          if (execvp(p_args[0], p_args) < 0) {
            if (errno == ENOENT) {
              fprintf(stderr, "koish: '%s' is not a valid command!\n", p_args[0]);
            }
            else {
              perror("execvp");
            }
          }
          // We only end up here if something went wrong...
          exit(EXIT_FAILURE);
        }

        // Parent waits for the child to finish before processing next task
        else {
          waitpid(pid, &status, WUNTRACED);
          status = 1;
        }
      }
    }
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
    status = sh_process(args);

    free(line);
    free(args);
    free_jlist();
  } while (status > 0);
}

int main(int argc, char** argv) {
  sh_loop();
  return EXIT_SUCCESS;
}
