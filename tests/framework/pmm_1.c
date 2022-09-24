#include <kernel.h>
#include <klib.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/queue.h>

int _pmm_nbits_round(size_t size);

static inline void test()
{
  printf("%d\n",_pmm_nbits_round(4096));
  void *ptr1 = pmm->alloc(1);
  void *ptr2 = pmm->alloc(1);
  printf("%p %p\n",ptr1,ptr2);
}

int main() {
  os->init();
  // mpe_init(os->run);
  test();
  return 0;
}
