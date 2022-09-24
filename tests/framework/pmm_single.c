#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <sys/queue.h>
#include <time.h>
#include <common.h>
#include <klib.h>


typedef struct _list {
    void *ptr;
    SLIST_ENTRY(_list);
} list_t;

typedef SLIST_HEAD(_list_head,_list) head_t;

static __always_inline void single_thread_stress_test(int ntimes)
{
    
}

int main(int argc, char *argv[])
{
    assert(argc == 2);
    int ntimes = atoi(argv[1]);
    srand(time(NULL));
    os->init();
    single_thread_stress_test(ntimes);
    return 0;
}