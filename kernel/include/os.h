#include <common.h>

struct task {
  const char *name;
  void (*entry)(void *arg);
  void *arg;
  
};

struct spinlock {
  const char *name;
  int locked;
};

struct semaphore {
  const char *name;
  int value;
  struct spinlock lock;
};
