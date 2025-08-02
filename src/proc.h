#include <sys/types.h>

typedef struct process {
  pid_t pid;
  int argv_offset;
  int status;

  struct process* next;
} process_t;

extern process_t* HEAD;

int init_plist(void);
process_t* add_to_plist(pid_t, int argv_offset);
int free_plist(void);
