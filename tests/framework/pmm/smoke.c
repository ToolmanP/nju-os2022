#include <common.h>
#include <klib.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/queue.h>


static inline void smoke_test()
{
  for(int i=0;i<(PGSIZE>>7)+1;i++)
    pmm->alloc(1<<7);
}

int main() {
  os->init();
  // mpe_init(os->run);
  smoke_test();
  return 0;
}
