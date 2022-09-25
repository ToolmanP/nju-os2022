#include <common.h>
#include <klib.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/queue.h>


static inline void smoke_test()
{
  void *ptr1 = pmm->alloc(2048);
  void *ptr2 = pmm->alloc(2048);
  void *ptr3 = pmm->alloc(2048);
  printf("%p %p %p\n",ptr1,ptr2,ptr3);
}

int main() {
  os->init();
  // mpe_init(os->run);
  smoke_test();
  return 0;
}
