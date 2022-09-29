#ifndef __OS_H
#define __OS_H

#include <common.h>
#include <common/queue.h>

#define STACKSIZE 8192
#define IRQ_MIN 0
#define IRQ_MAX INT32_MAX

enum thread_state{
  READY = 0,
  RUNNING,
  WAITING,
  BLOCKED,
  NOT_READY,
  DEAD,
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
  enum thread_state status;
} thread_t;

struct spinlock {
  const char *name;
  int locked;
  thread_t *acquirer; // generic acquirer
};

NODE_TYPEDEF(task_t *,task);
NODE_TYPEDEF(thread_t *,thread);
LIST_TYPEDEF(task);
LIST_TYPEDEF(thread);
QUEUE_TYPEDEF(task);
QUEUE_TYPEDEF(thread);

typedef struct process{
  LIST_T(thread) *prthreads;
  int pid;
  int status;
} process_t;


NODE_TYPEDEF(process_t *,process);
LIST_TYPEDEF(process);
QUEUE_TYPEDEF(process);

typedef struct cpu{
  thread_t *tcurrent; // current 
  thread_node_t  *ncurrent;
  spinlock_t  lock; // cpu-local lock
  int cpuid;
  int acquired; // acquired locks excluding cpu-local lock
  int locked; // is current cpu locked ?
  int interrupted; // is in an interrupt handler?
} cpu_t;

struct semaphore {
  const char *name;
  QUEUE_T(thread) *wthreads;
  spinlock_t lock;
  int value;
};

typedef struct _threadpool{
  LIST_T(thread) *pool;
  spinlock_t lock;
} threadpool_t;

typedef struct _processpool{
  LIST_T(process) *pool;
  spinlock_t lock;
}processpool_t;

#endif