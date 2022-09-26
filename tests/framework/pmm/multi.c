#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>

#include <sys/types.h>
#include <common.h>
#include <pthread.h>
#include <semaphore.h>

  
#define NTHREADS 8
#define SLOTS 256

typedef struct {
    int ntimes;
    int mode;
}parg_t;

typedef struct{
    void *q[SLOTS];
    int n;
} queue_t;

static sem_t lock,slot_free,slot_alloc;
static queue_t queue;

typedef struct{
    pthread_t tid;
    int id;
    parg_t *parg;
} thread_t;

thread_t producer_threads[NTHREADS];
thread_t consumer_threads[NTHREADS];

void *kalloc_thread(size_t size,int cpuid);
void kfree_thread(void *ptr,int cpuid);

static __always_inline int generate_size(int mode)
{
    switch(mode){
        case 0:
            return rand()%(PGSIZE-1)+1; // Small shard memory allocation
        case 1:
            return rand()%(MALLOCMAX-PGSIZE)+PGSIZE; // Large shard memory allocation
        case 2:
            return rand()%(MALLOCMAX-1)+1; // Mixed intensive testing
        default:
            assert(0);
    }
    return -1;
}

static __always_inline void op_alloc(int mode,int id)
{   
    void *ret;
    sem_wait(&slot_free);
    sem_wait(&lock);
    ret = kalloc_thread(generate_size(mode),id);
    queue.q[queue.n++] = ret;
    printf("A,%p\n",ret);
    sem_post(&lock);
    sem_post(&slot_alloc);
}

static __always_inline void op_free(int id)
{
    void *ret;
    sem_wait(&slot_alloc);
    sem_wait(&lock);
    ret = queue.q[--queue.n];
    kfree_thread(ret,id);
    printf("F,%p\n",ret);
    sem_post(&lock);
    sem_post(&slot_free);
}

static void *consumer(void *ptr)
{
    thread_t *t = ptr;
    volatile int ntimes = t->parg->ntimes;
    int id = t->id;
    while(ntimes--){
        op_free(id);
    }
    fprintf(stderr,"Terminating comsumer thread\n");
    return NULL;
}

static void *producer(void *ptr)
{
    thread_t *t = ptr;
    int ntimes = t->parg->ntimes;
    int mode = t->parg->mode;
    int id = t->id;
    while(ntimes--){
        op_alloc(mode,id);
    }
    fprintf(stderr,"Terminating producer thread\n");
    return NULL;
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
    sem_init(&slot_alloc,0,0);
    sem_init(&slot_free,0,SLOTS);
    sem_init(&lock,0,1);
    srand(time(NULL));
    atexit(exit_function);

    os->init();

    UNUSED(consumer);
    UNUSED(producer);

    parg_t parg = {
        .ntimes = ntimes,
        .mode = 2
    };
    
    for(int i=0;i<NTHREADS;i++){
        producer_threads[i].id = i;
        producer_threads[i].parg = &parg;

        consumer_threads[i].id = i;
        consumer_threads[i].parg = &parg;
        pthread_create(&producer_threads[i].tid,NULL,producer,&producer_threads[i]);
        pthread_create(&consumer_threads[i].tid,NULL,consumer,&consumer_threads[i]);
    }

    
    for(int i=0;i<NTHREADS;i++){
        pthread_join(producer_threads[i].tid,NULL);
        pthread_join(consumer_threads[i].tid,NULL);
    }

    return 0;
}