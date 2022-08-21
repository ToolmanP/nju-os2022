#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>
#include <malloc.h>
#include <assert.h>
#include <stdatomic.h>

#include "co.h"

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

#if __x86_64__
  #define ALIGN_SIZE 16
#else
  #define ALIGN_SIZE 8
#endif

#define __ALIGN__ __attribute__((aligned(ALIGN_SIZE)))

typedef struct co  {

  const char *name;
  void (*func)(void *);
  void *arg; 

  enum co_status status;
  struct co *waiter;
  jmp_buf context;
  uint32_t yield_cnt;

  uint8_t stack[STACK_SIZE] __ALIGN__;
} co_t;

typedef struct __col{
  co_t *co;
  struct __col *next;
} __col_t;

static co_t __co_boot = {
  .name = "main",
  .func = NULL,
  .arg = NULL,
  .status = CO_RUNNING,
  .yield_cnt = 0
};

static volatile co_t *co_current = &__co_boot;
static volatile co_t **co_current_p = &co_current;

static __col_t *co_head;

static inline void stack_switch_call(void *sp,void *entry,uintptr_t arg){
  asm volatile (
    #if __x86_64__
        "movq %0, %%rsp; movq %2, %%rdi; call *%1"
          : : "b"((uintptr_t)sp), "d"(entry), "a"(arg) : "memory"
    #else
        "movl %0, %%esp; movl %2, (%0); call *%1"
          : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg) : "memory"
    #endif
  );
}

static inline co_t *__co_alloc(){
  return malloc(sizeof(co_t));
}

static inline void __co_free(co_t *co){
  free(co);
}


static inline __col_t *__co_list_alloc(co_t *co){
  __col_t *entry = malloc(sizeof(__col_t));
  entry->co = co;
  entry->next = NULL;
  return entry;
}

static inline void __co_list_insert(co_t *co){
  __col_t *entry = __co_list_alloc(co);
  entry->next = co_head;
  co_head = entry;
}

static inline void __co_list_delete(co_t *co){
  for(__col_t **curr = &co_head, *entry;*curr;){
    entry = *curr;
    if(entry->co == co){
      *curr = entry->next;
      __co_free(entry->co);
      free(entry);
      return;
    }else{
      curr = &(entry->next);
    }
  }
  assert(0); // connot reach here;
}

static inline co_t *__co_list_fetch(){
  uint32_t minn = 0xFFFFFFFF;
  co_t *ret = NULL;
  for(__col_t *entry = co_head;entry;entry = entry->next){
    if(entry->co != co_current && entry->co->status != CO_DEAD){
        if(entry->co->yield_cnt < minn){
          minn = entry->co->yield_cnt;
          ret = entry->co;
        }
      }
  }
  assert(ret);
  return ret;
}

__attribute__((noinline))
static void __co_resume(co_t *co){

  assert(co->status != CO_DEAD);

  co_current = co;

  if(co->status == CO_NEW){
    co->status = CO_RUNNING;
    stack_switch_call(co->stack+STACK_SIZE,co->func,(uintptr_t)co->arg);
  }else{
    longjmp(co->context,0);
  }
  
  (*co_current_p)->status = CO_DEAD;
  __sync_synchronize();
  co_yield(); // context switch
}

__attribute__((constructor)) static inline void __co_init(){
  co_head = __co_list_alloc((co_t *)co_current);
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  co_t *co = __co_alloc();
  co->arg = arg;
  co->func = func;
  co->name = name;
  co->status = CO_NEW;
  co->yield_cnt = 0;

  __co_list_insert(co);
  return co;
}

void co_wait(struct co *co) {

  co_current->status = CO_WAITING;  
  co->waiter = (co_t *)co_current;
  
  while(co->status != CO_DEAD)
    co_yield();
  
  co->waiter = NULL;
  co_current->status = CO_RUNNING;
  __co_list_delete(co);
  
}

void co_yield() {
  co_t *entry = __co_list_fetch();
  int val = setjmp(((co_t *)co_current)->context);

  entry->yield_cnt++;
  
  switch(val){
    case 0:
      __co_resume(entry);
      break;
    case 1:
      break;
    default:
      assert(0); // never reach here
  };

}
