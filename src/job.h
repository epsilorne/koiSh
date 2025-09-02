#include <sys/types.h>

// A representation of a single process (task) within a job/pipeline
typedef struct process {
  pid_t pid;
  int argv_offset;
  int status;

  struct process* next;
} process_t;

/**
 * A representation of a single job, which may contain one or more tasks. The
 * idea is that if we can have several tasks connected by a pipeline, or one
 * singular task to represent one process (e.g. typical 'command' use case).
 */
typedef struct job {
  size_t n_tasks;           // how many tasks this job has
  process_t* tasks;         // a linked list of this job's tasks (processes)
  int last_status;

  struct job* next;
} job_t;

int init_plist(void);
process_t* add_to_plist(job_t*, pid_t, int);
int free_plist(process_t* head);

job_t* add_to_jlist();
int free_jlist(void);
