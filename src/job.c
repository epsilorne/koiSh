#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "job.h"

job_t* JOB_HEAD = NULL;

/**
 * Add a new process to the tail of the linked list. The linked list will be
 * allocated if it hasn't been already.
 */
job_t* add_to_jlist(process_t* tasks) {
  if (!JOB_HEAD) {
    JOB_HEAD = malloc(sizeof(job_t));
    if (!JOB_HEAD) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    JOB_HEAD->tasks = tasks;
    JOB_HEAD->next = NULL;
    return JOB_HEAD;
  }
  job_t* curr = JOB_HEAD;
  
  while (curr->next) {
    curr = curr->next;
  }

  job_t* new = malloc(sizeof(job_t));
  if (!new) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  new->tasks = tasks;
  new->next = NULL;
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
    curr = curr->next;
    free(tmp);
  }

  JOB_HEAD = NULL;
  return 1;
}
