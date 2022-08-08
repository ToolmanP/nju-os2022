#include "co.h"
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>
#include <malloc.h>
#include <assert.h>

#define STACK_SIZE 1024*64

#ifdef LOCAL_MACHINE
  #define debug(...) printf(__VA_ARGS__)
#else
  #define debug()
#endif

enum co_status{
  CO_NEW = 1,
  CO_RUNNING,
  CO_WAITING,
  CO_DEAD
};

typedef struct co {

  const char *name;
  void (*func)(void *);
  void *arg;

  enum co_status status;
  struct co *waiter;
  jmp_buf context;
  uint8_t stack[STACK_SIZE];  
} co_t;

typedef struct __col{
  co_t *co;
  struct __col *next;
} __col_t;

co_t __co_boot = {
  .status = CO_RUNNING
};

co_t *co_current = &__co_boot;

__col_t *co_head,*co_tail;

static inline void stack_switch_call(void *sp,void *entry,uintptr_t arg){
  asm volatile (
    #if __x86_64__
      "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
        : : "b"((uintptr_t)sp), "d"(entry), "a"(arg) : "memory"
    #else
      "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
        : : "b"((uintptr_t)sp-8), "d"(entry), "a"(arg) : "memory"
    #endif
  );
}

static inline co_t *__co_alloc(){
  return malloc(sizeof(co_t));
}

static inline void __co_free(co_t *co){
  free(co);
}

static inline void __co_resume(co_t *co){
  assert(co->status == CO_NEW || co->status == CO_RUNNING);
  co_current = co;
  if(co->status == CO_NEW){
    co->status = CO_RUNNING;
    stack_switch_call(co->stack+STACK_SIZE-1,co->func,(uintptr_t)co->arg);
  }else{
    longjmp(co->context,0);
  }
}

static inline __col_t *__co_list_alloc(co_t *co){
  __col_t *entry = malloc(sizeof(__col_t));
  entry->co = co;
  entry->next = NULL;
  return entry;
}

static inline void __co_list_insert(co_t *co){
  co_tail->next = __co_list_alloc(co);
  co_tail = co_tail->next;
}

static inline void __co_list_delete(co_t *co){
  for(__col_t **curr = &co_head, *entry;*curr;){
    entry = *curr;
    if(entry->co == co){
      *curr = entry->next;
      __co_alloc(entry->co);
      free(entry);
      return;
    }else{
      curr = &(entry->next);
    }
  }
  assert(0); // connot reach here;
}

static inline co_t *__co_list_fetch(){
  for(__col_t *entry = co_head;entry;entry = entry->next){
    if(entry->co != co_current && entry->co->status != CO_WAITING)
      return entry->co; 
  }
  assert(0);
  return NULL;
}

__attribute__((constructor)) static inline void __co_init(){
  co_head = __co_list_alloc(co_current);
  co_tail = co_head;
  debug("constructor\n");
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  co_t *co = __co_alloc();
  co->arg = arg;
  co->func = func;
  co->name = name;
  co->status = CO_NEW;
  __co_list_insert(co);
  return co;
}

void co_wait(struct co *co) {

  co_current->status = CO_WAITING;
  co->waiter = co_current;
  
  while(co->status != CO_DEAD)
    co_yield();
  
  __co_list_delete(co);

}

void co_yield() {
  co_t *co = __co_list_fetch();
  int val = setjmp(co_current->context);
  if(val==0){
    __co_resume(co);
    co->status = CO_DEAD;
  }else{

  }
}