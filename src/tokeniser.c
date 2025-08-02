#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"
#include "proc.h"
#include "shell.h"

const char* DELIMS = " \t\n\r";

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

  if (!buf) {
    perror("buff");
    exit(EXIT_FAILURE);
  }

  while (token) {
    if (token[0] == ';') {
      buf[pos++] = NULL;
      add_to_plist(0, offset);
      offset = pos;
    }
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
  add_to_plist(0, offset);

  return buf;
}

