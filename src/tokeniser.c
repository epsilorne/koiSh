#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"
#include "job.h"
#include "shell.h"

const char* DELIMS = " \t\n\r";
extern job_t* JOB_HEAD;

/**
 * Tokenise a whitespace-separated input string into an array of arguments, used
 * to allocate and insert process_t structs into HEAD.
 *
 * The idea is that exec() requires NULL-terminated argument vectors, so a
 * string like "ls -la ; clear" would become {"ls", "-la", "\0", "clear", "\0"},
 * and each process_t->argv will point to the buffer, plus some offset.
 *
 * Returns the buffer of NULL-terminated args, which must be freed.
 */
char** sh_tokenise(char* line) {
  size_t buf_size = DEFAULT_ARG_COUNT;

  char** buf = malloc(sizeof(char*) * buf_size);
  char* token = strtok(line, DELIMS);

  int pos = 0;

  // Represents the offset of the starting arg for the current process_t
  int offset = 0;

  process_t* temp = NULL;
  process_t* head = NULL;

  if (!buf) {
    perror("buff");
    exit(EXIT_FAILURE);
  }

  while (token) {
    // This should be the seperator for JOBS
    if (token[0] == ';') {
      buf[pos++] = NULL;
      temp = add_to_plist(temp, 0, offset);
      if (!head) {
        head = temp;
      }
      offset = pos;
    }
    // Pipe (|) should be the separator for PROCESSES/TASKS
    else {
      buf[pos++] = token;
    }

    if (pos >= buf_size) {
      buf_size += DEFAULT_ARG_COUNT;
      buf = realloc(buf, sizeof(char*) * buf_size);

      if (!buf) {
        perror("realloc");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, DELIMS);
  }

  buf[pos] = NULL;
  temp = add_to_plist(temp, 0, offset);
  if (!head) {
    head = temp;
  }

  // TODO: only temp until we have parsing for multiple jobs
  add_to_jlist(head);

  return buf;
}

