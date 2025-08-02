#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "proc.h"

// TODO: only temporary
process_t* HEAD = NULL;

/**
 * Add a new process to the tail of the linked list. The linked list will be
 * allocated if it hasn't been already.
 */
process_t* add_to_plist(pid_t pid, int argv_offset) {
  if (!HEAD) {
    HEAD = malloc(sizeof(process_t));
    if (!HEAD) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    HEAD->pid = pid;
    HEAD->argv_offset = argv_offset;
    HEAD->next = NULL;
    return HEAD;
  }
  process_t* curr = HEAD;

  while (curr->next) {
    curr = curr->next;
  }

  process_t* new = malloc(sizeof(process_t));
  if (!new) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  new->pid = pid;
  new->argv_offset = argv_offset;
  new->next = NULL;
  curr->next = new;

  return new;
}

/**
 * Clean up the linked use, e.g. after execution of program(s).
 */
int free_plist(void) {
  process_t* curr = HEAD;
  process_t* tmp;

  while (curr) {
    tmp = curr;
    curr = curr->next;
    free(tmp);
  }

  HEAD = NULL;
  return 1;
}
