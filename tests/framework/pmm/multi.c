#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>

#include <sys/types.h>
#include <common.h>
#include <pthread.h>
#include <semaphore.h>

#define NTHREADS 16
#define SLOTS 64

typedef struct {
    int ntimes;
    int mode;
}parg_t;

typedef struct{
    void *q[SLOTS];
    int n;
} queue_t;

static sem_t lock,free,alloc;
static queue_t queue;

static __always_inline int generate_size(int mode)
{
    switch(mode){
        case 0:
            return rand()%(PGSIZE-1)+1; // Small shard memory allocation
        case 1:
            return rand()%(MALLOCMAX-PGSIZE)+PGSIZE; // Large shard memory allocation
        case 2:
            return rand()%(MALLOCMAX-1)+1; // Mixed intensive testing
    }
}

static __always_inline void *op_alloc(int mode)
{   
    void *ret;
    sem_wait(&free);
    sem_wait(&lock);
    ret = pmm->alloc(generate_size(mode));
    queue.q[queue.n++] = ret;
    sem_post(&lock);
    sem_post(&alloc);
    return ret;
}

static __always_inline void *op_free()
{
    void *ret;
    sem_wait(&alloc);
    sem_wait(&lock);
    ret = queue.q[--queue.n];
    pmm->free(ret);
    sem_post(&lock);
    sem_post(&free);
    return ret;
}

static void consumer(void *ptr)
{
    parg_t *arg = ptr;
    int ntimes = arg->ntimes;
    while(ntimes--){
        printf("F,%p",op_free());        
    }
    fprintf(stderr,"Terminating comsumer thread %d",gettid());
}

static void producer(void *ptr)
{
    parg_t *arg = ptr;
    int ntimes = arg->ntimes;
    int mode = arg->mode;
    while(ntimes--){
        printf("F,%p",op_alloc(mode));
    }
    fprintf(stderr,"Terminating producer thread %d",gettid());
}

static void exit_function()
{
    fprintf(stderr,"Current round of testing is over.");
}

int main(const char *arg)
{   
    setbuf(stdout,NULL);
    setbuf(stderr,NULL);

    int ntimes = atoi(arg);
    sem_init(&alloc,0,0);
    sem_init(&free,0,SLOTS);
    sem_init(&lock,0,1);
    atexit(exit_function);

    parg_t parg = {
        .ntimes = ntimes
    };
    
    for(int i=0;i<NTHREADS;i++){
        
    }
    return 0;
}