#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "job.h"

// Pointer to the HEAD of the job_t linked list
job_t* JOB_HEAD = NULL;

/**
 * Add a new job_t to the tail of the linked list (JOB_HEAD). The linked list
 * will be allocated if it hasn't been already.
 */
job_t* add_to_jlist() {
  // If the HEAD has not been declared, our 'inserted' node becomes the HEAD
  if (!JOB_HEAD) {
    JOB_HEAD = malloc(sizeof(job_t));
    if (!JOB_HEAD) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    JOB_HEAD->tasks = NULL;
    JOB_HEAD->next = NULL;
    JOB_HEAD->n_tasks = 0;
    return JOB_HEAD;
  }

  // Otherwise, go through the linked list and insert the node
  job_t* curr = JOB_HEAD;
  while (curr->next) {
    curr = curr->next;
  }

  job_t* new = malloc(sizeof(job_t));
  if (!new) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  new->tasks = NULL;
  new->next = NULL;
  new->n_tasks = 0;
  curr->next = new;

  return new;
}

/**
 * Clean up the linked use, e.g. after execution of program(s). Will also free
 * the associated process linked list of a job.
 */
int free_jlist(void) {
  job_t* curr = JOB_HEAD;
  job_t* tmp;

  while (curr) {
    tmp = curr;
    free_plist(tmp->tasks);

    tmp->next = NULL;
    tmp->tasks = NULL;

    curr = curr->next;
    free(tmp);
  }

  JOB_HEAD = NULL;
  return 1;
}
