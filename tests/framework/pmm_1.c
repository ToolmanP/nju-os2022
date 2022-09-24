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
  void *ptr = pmm->alloc(20);
  printf("%p\n",ptr);
}

int main() {
  os->init();
  // mpe_init(os->run);
  test();
  return 0;
}
