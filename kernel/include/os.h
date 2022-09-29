#include <common.h>
#include <common/queue.h>

#define STACKSIZE 8192
#define IRQ_MIN 0
#define IRQ_MAX INT32_MAX

enum{
  READY = 0,
  RUNNING,
  WAITING,
  DEAD,
};

struct spinlock {
  const char *name;
  int locked;
};

struct task {
    const char *name;
    void (*entry)(void *arg);
    void *arg;
    Area stack;
    Context *context;
};

typedef struct thread{
  task_t *task;
  int status;
} thread_t;

NODE_TYPEDEF(task_t *,task);
NODE_TYPEDEF(thread_t *,thread);
LIST_TYPEDEF(task);
LIST_TYPEDEF(thread);
QUEUE_TYPEDEF(task);
QUEUE_TYPEDEF(thread);

typedef struct process{
  LIST_T(thread) *prthreads;
  LIST_T(thread) *pwthreads;
  int pid;
  int status;
} process_t;


NODE_TYPEDEF(process_t *,process);
LIST_TYPEDEF(process);
QUEUE_TYPEDEF(process);

typedef struct cpu{
  thread_list_t *kthreads;
  spinlock_t lock;
  int cpuid;
} cpu_t;

struct semaphore {
  const char *name;
  thread_queue_t *wthreads;
  spinlock_t lock;
  int value;
};
