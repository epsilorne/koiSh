#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "job.h"

/**
 * Add a new process to the tail of the linked list. The linked list will be
 * allocated if it hasn't been already.
 */
process_t* add_to_plist(process_t* head, pid_t pid, int argv_offset) {
  if (!head) {
    head = malloc(sizeof(process_t));
    if (!head) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    head->pid = pid;
    head->argv_offset = argv_offset;
    head->next = NULL;
    return head;
  }

  while (head->next) {
    head = head->next;
  }

  process_t* new = malloc(sizeof(process_t));
  if (!new) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  new->pid = pid;
  new->argv_offset = argv_offset;
  new->next = NULL;
  head->next = new;

  return new;
}

/**
 * Clean up the linked use, e.g. after execution of program(s).
 */
int free_plist(process_t* head) {
  process_t* curr = head;
  process_t* tmp;

  while (curr) {
    tmp = curr;
    curr = curr->next;
    free(tmp);
  }

  head = NULL;
  return 1;
}
