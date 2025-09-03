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

/**
 * Handle processing of the job_t list to execute jobs and their tasks.
 *
 * For each job_t in the list, we execute its tasks (a list of process_t). Each
 * task is connected via a pipe (delimited by '|'), allowing for a pipeline of
 * commands to be run with data being transferred between them. (Specifically,
 * the pipe() syscall connects the STDOUT of one task to the STDIN of another.)
 *
 * If the command is a builtin, it is executed directly in the shell. Otherwise,
 * a child process is forked to execute the program.
 *
 * Returns the 'status' of the latest command, which is typically 1, but 0 when
 * the 'exit' builtin is called.
*/
int sh_process(char** argv) {
  int n, builtin_idx, status = -1, fds[2];
  char** p_args;
  job_t* j = JOB_HEAD;
  pid_t pid;

  // Iterate through each job in the linked list
  for (; j && status; j = j->next) {
    process_t* p = j->tasks;
    n = j->n_tasks;

    /* In a pipeline, we can pass the 'previous' READ FD to feed the output of
     * previous tasks as the STDIN of other tasks. (Except for the first and
     * last tasks in the pipeline.) */
    int prev_read_fd = -1;

    // Iterate through the tasks associated with this job
    for(int i = 0; i < n && p; ++i, p = p->next) {
      _Bool last_task = i >= n - 1;
      p_args = argv + p->argv_offset;

      // An empty input...
      if (!p_args[0]) {
        fprintf(stderr, "koish: did you mean to say something?\n");
        status = 1;
        continue;
      }

      // Setup pipe FDs for non-last tasks, storing in the fd[2] array
      if (!last_task && pipe(fds) < 0) {
        perror("pipe");
      }

      // If the command is a built in, it is executed in the shell
      if ((builtin_idx = check_builtin(p, p_args)) != -1) {
        if ((status = exec_builtin[builtin_idx](p_args)) <= 0)
          break;
      }

      // Otherwise, it's a program that will be run in a child process
      else {
        if ((pid = fork()) < 0) {
          perror("fork");
          exit(EXIT_FAILURE);
        }

        // Child process will run the program specified
        if (pid == 0) {
          /* If we have a READ FD from the previous task, we use that as STDIN
           * for this task. */
          if (prev_read_fd != -1) {
            if (dup2(prev_read_fd, STDIN_FILENO) < 0)
              perror("dup2");

            // Need to close that FD (no longer in use; it's now STDIN)
            if (close(prev_read_fd) < 0)
              perror("close (0)");
          }

          /* Unless this is the last task in the queue, we set STDOUT to be the
           * WRITE end of our created pipe. (Otherwise, it's usual STDOUT.) */
          if (!last_task) {
            if (dup2(fds[1], STDOUT_FILENO) < 0)
              perror("dup2");

            // Need to close the pipe FDs (no longer in use)
            if (close(fds[1]) < 0 || close(fds[0]) < 0)
              perror("close (1)");
          }

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

        // Parent (shell) waits for the child to finish before running next task
        else {
          // We don't need the READ (input) FD; we can get rid of it
          if (prev_read_fd != -1 && close(prev_read_fd) < 0)
            perror("close (2)");

          /* Close the WRITE FD of the pipe (except for the last task, who'll
          * use STDIN. */
          if (!last_task && close(fds[1]) < 0)
            perror("close (3)");

          prev_read_fd = fds[0];
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
