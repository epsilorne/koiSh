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
 * Given an input process_t, create a child process to execute the program with
 * argument(s) specified.
 *
 * NOTE: the input argv is assumed to 'belong' to the input process_t, i.e. the
 * offset has already been applied.
 *
 * Returns 1 if exec() did not fail.
 */
int sh_exec(process_t* p, char** argv, int stdin_fd, int stdout_fd) {
  pid_t pid;
  int status;

  if ((pid = fork()) == 0) {
    // Set the STDIN and STDOUT of the child to the input FDs
    if (dup2(stdin_fd, STDIN_FILENO) < 0 || dup2(stdout_fd, STDOUT_FILENO) < 0) {
      fprintf(stderr, "koish: error setting STDIN/STDOUT of program, with stdin: %d, stdout: %d\n", stdin_fd, stdout_fd);
      exit(EXIT_FAILURE);
    }

    if (execvp(argv[0], argv) < 0) {
      if (errno == ENOENT) {
        fprintf(stderr, "koish: '%s' is not a valid command!\n", argv[0]);
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
 * Handle processing of the job_t list to sequentially execute jobs/tasks.
 *
 * For each job_t in the list, we execute its tasks (list of process_t).
 *
 * If the command is determined to be a builtin, the specific builtin will be
 * executed. Otherwise, it is a program and we call sh_exec() to manage it.
 *
 * The 'status' of the latest command is returned, which is typically 1, but 0
 * when the exit() builtin is called.
 */
int sh_process(char** argv) {
  job_t* curr_j = JOB_HEAD;

  int status = 1;
  int builtin_idx;

  int stdin_fd = STDIN_FILENO, stdout_fd = STDOUT_FILENO;
  int old_stdin, old_stdout;
  int tasks_count;

  // Iterate through each job and action its associated tasks
  while (curr_j) {
    process_t* curr_p = curr_j->tasks;
    tasks_count = curr_j->n_tasks;

    // Store file descriptors (STDIN and STDOUT) for our n - 1 pipes
    int* fds = tasks_count > 1 ? malloc(sizeof(int) * (tasks_count - 1)) : 0;

    /* Create the actual file descriptors for the pipes (noting first one is
    'reading', second one is 'writing') */
    for (int i = 0; i < tasks_count - 1; ++i) {
      if (pipe(fds + (i * 2)) < 0) {
        fprintf(stderr, "koish: could not create pipes\n");
      }
    }

    for (int i = 0; i < tasks_count; ++i) {
      // Designate FDs for STDIN and STDOUT based on the no. of tasks
      if (tasks_count <= 1) {
        stdin_fd = STDIN_FILENO;
        stdout_fd = STDOUT_FILENO;
      }
      // For more than one task, we need to set up our pipes
      else {
        // The first task's STDIN needs to be the 'true' STDIN
        stdin_fd = i == 0 ? STDIN_FILENO : fds[(i - 1) * 2];
        // The last task's STDOUT needs to be the 'true' STDOUT
        stdout_fd = i == tasks_count - 1 ? STDOUT_FILENO : fds[(i * 2) + 1];
      }

      // Get argv for the current process_t
      char** curr_args = argv + curr_p->argv_offset;

      if (!curr_args[0]) {
        fprintf(stderr, "koish: did you mean to say something?\n");
      }
      /* Check if the command is a builtin and if it is, execute it. Otherwise,
         we create a process to execute the program. */
      else if ((builtin_idx = check_builtin(curr_p, curr_args)) != -1) {
        // TODO: fix this; poorly defined
        old_stdin = dup(stdin_fd);
        old_stdout = dup(stdout_fd);

        // For builtins, we just temporarily set STDIN and STDOUT
        if (dup2(stdin_fd, STDIN_FILENO) < 0 || dup2(stdout_fd, STDOUT_FILENO) < 0) {
          fprintf(stderr, "koish: error setting STDIN/STDOUT of builtin\n");
        }

        status = exec_builtin[builtin_idx](curr_args);

        // Once executed, restore STDIN and STDOUT
        if (dup2(old_stdin, stdin_fd) < 0 || dup2(old_stdout, stdout_fd) < 0) {
          fprintf(stderr, "koish: error setting STDIN/STDOUT of builtin\n");
        }
      }
      else {
        status = sh_exec(curr_p, curr_args, stdin_fd, stdout_fd);
        curr_p->status = status;
      }
      curr_p = curr_p->next;
    }
    free(fds);
    curr_j = curr_j->next;
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
