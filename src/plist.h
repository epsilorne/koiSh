#include <sys/types.h>

typedef struct process {
  pid_t pid;
  char** argv;
  int status;

  struct process* next;
} process_t;

extern process_t* HEAD;

int init_plist(void);
process_t* add_to_plist(pid_t, char** argv);
int free_plist(void);
