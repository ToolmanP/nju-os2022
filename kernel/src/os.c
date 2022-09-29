#include <common.h>

#include <modules/kmt.h>
#include <os.h>
#define MAXIRQS 64

typedef struct irq{
  int seq,event;
  handler_t handler;
} irq_t;

typedef struct irqlist{
  irq_t irqs[MAXIRQS];
  int nirqs;
}irqlist_t;

irqlist_t irql;

static void irq_swap(irq_t *a, irq_t *b)
{
  irq_t tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;  
}

static void irq_sort(irq_t *irqs,int l,int r) // Quicksort why not ?
{ 
  if(l>=r) return;
  irq_t tmp = irqs[l];
  int i = l;
  int j = r;
  while(i<j){
      while(irqs[j].seq >= tmp.seq) j--;
      if(i>=j) break;
      irq_swap(&irqs[i],&irqs[j]);
      while(irqs[i].seq < tmp.seq) i++;
      if(i>=j) break;
      irq_swap(&irqs[i],&irqs[j]);
  }
  irqs[i] = tmp;
  irq_sort(irqs,l,i-1);
  irq_sort(irqs,i+1,r);
}

static int context_check(Context *ctx)
{
  extern char end;
  int status = 0;
#if __ARCH__ == x86_64-qemu
  Area rip_area = {(void *)0x1000b0,&end};
  Area rsp_area = {NULL,heap.end};

  status |= !(IN_RANGE((void *)ctx->rip,rip_area));
  status |= !(IN_RANGE((void *)ctx->rsp,rsp_area));
  return status;
#else
  return -1;
#endif
}

static Context *os_context_save_irq_handler(Event ev, Context *ctx)
{
  kmt_context_save(ev,ctx);
  return ctx;
}

static Context *os_context_schedule_irq_handler(Event ev, Context *ctx)
{
  return kmt_context_schedule(ev,ctx);
}

static Context *os_syscall_irq_handler(Event ev,Context *ctx)
{
  return (void *)-1;
}

static Context *os_pagefault_irq_handler(Event ev,Context *ctx)
{
  return (void *)-1;
}

static Context *os_timer_irq_handler(Event ev, Context *ctx)
{
  return (void *)-1;
}

static Context *os_devio_irq_handler(Event ev, Context *ctx)
{
  return (void *)-1;
}

static Context *os_error_irq_handler(Event ev, Context *ctx)
{
  panic("CPU Error Happened!");
  return NULL;
}

static Context *os_trap(Event ev, Context *ctx)
{
  Context *next = NULL;
  for(int i=0;i<irql.nirqs;i++){
    if(irql.irqs[i].event == EVENT_NULL || ev.event == irql.irqs[i].event){
      next = irql.irqs[i].handler(ev,ctx);
      if(next) break;
    }
  }
  panic_on(next == (void *)-1, "Not Implemented irq handler");
  panic_on(next == NULL, "Returning null context");
  panic_on(context_check(next) != 0, "Returning invalid context");
  return next;

}

static void os_irq(int seq, int event, handler_t handler)
{
  irql.irqs[irql.nirqs++].seq = seq;
  irql.irqs[irql.nirqs].handler = handler;
  irql.irqs[irql.nirqs].event = event;
}

static void os_init()
{
  pmm->init();
  kmt->init();
  irq_sort(irql.irqs,0,irql.nirqs-1);
  os_irq(IRQ_MIN,EVENT_NULL,os_context_save_irq_handler);
  os_irq(300,EVENT_IRQ_IODEV,os_devio_irq_handler);
  os_irq(200,EVENT_IRQ_TIMER,os_timer_irq_handler);
  os_irq(100,EVENT_SYSCALL,os_syscall_irq_handler);
  os_irq(400,EVENT_PAGEFAULT,os_pagefault_irq_handler);
  os_irq(500,EVENT_ERROR,os_error_irq_handler); 
  os_irq(IRQ_MAX,EVENT_NULL,os_context_schedule_irq_handler);
}

static void os_run()
{
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_irq
};
