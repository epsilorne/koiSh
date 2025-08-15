#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "job.h"

/**
 * Add a process_t to the tail of the input job_t's tasks list. Will initialise
 * the list if it has not been allocated yet.
 */
process_t* add_to_plist(job_t* job, pid_t pid, int argv_offset) {
  process_t* head = job->tasks;

  if (!head) {
    head = malloc(sizeof(process_t));
    if (!head) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    head->pid = pid;
    head->argv_offset = argv_offset;

    head->next = NULL;
    job->tasks = head;

    return head;
  }

  process_t* curr = head;
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
  job->tasks = head;

  return new;
}

/**
 * Clean up the linked list, e.g. after execution of program(s). This should be
 * managed by the 'parent' job_t.
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
