#include <common.h>
#include <klib.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/queue.h>


static inline void smoke_test()
{
  void *ptr1 = pmm->alloc(245);
  void *ptr2 = pmm->alloc(113);
  printf("%p %p \n",ptr1,ptr2);
}

int main() {
  os->init();
  // mpe_init(os->run);
  smoke_test();
  return 0;
}
