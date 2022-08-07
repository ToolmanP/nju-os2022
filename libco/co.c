#include "co.h"
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>

#define STACK_SIZE 8192

enum co_status{
  CO_NEW = 1,
  CO_RUNNING,
  CO_WAITING,
  CO_DEAD
};

struct co {
  char *name;
  void (*func)(void *);
  void *arg;

  enum co_status status;
  struct co *waiter;
  jmp_buf context;
  uint8_t stack[STACK_SIZE];  
};

struct co* current;

static inline void stack_switch_call(void *sp,void *entry,uintptr_t arg){
  asm volatile (
    #if __x86_64__
      "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
        : : "b"((uintptr_t)sp), "d"(entry), "a"(arg) : "memory"
    #else
      "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
        : : "b"((uintptr_t)sp-4), "d"(entry), "a"(arg) : "memory"
    #endif
  );
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  return NULL;
}

void co_wait(struct co *co) {
}

void co_yield() {
}
