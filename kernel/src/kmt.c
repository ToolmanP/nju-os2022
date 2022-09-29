
#include <modules/kmt.h>
#include <os.h>


static cpu_t *cpus;
static threadpool_t *kthreadpool; // kernel thread-pools
// static processpool_t *processpool; // process pool

#define ccpu (cpus[cpu_current()])

static __always_inline void kmt_try_enable_interrupt()
{
    if(ccpu.locked == 0 && ccpu.acquired == 0 && ccpu.interrupted == 0)
        iset(true);
    
}

static __always_inline void kmt_conditional_disable_interrupt()
{
    if(ccpu.locked == 1 || ccpu.acquired > 0 || ccpu.interrupted == 1)
        iset(false);
} 

static void kmt_spin_init(spinlock_t *lk, const char *name)
{   
    lk->locked = 0;
    lk->acquirer = NULL;
    lk->name = name;
}

static inline void kmt_generic_spin_lock(spinlock_t *lk)
{
    while(__os_acquire_spin_lock(&lk->locked)==1){
        yield();
    }
    lk->acquirer = ccpu.tcurrent;
}

static inline void kmt_generic_spin_unlock(spinlock_t *lk)
{
    lk->acquirer = NULL;
    __os_spin_unlock(&lk->locked);
}

static inline void kmt_ccpu_lock()
{
    kmt_generic_spin_lock(&ccpu.lock);
    ccpu.locked = 1;
}

static inline void kmt_ccpu_unlock()
{   
    ccpu.locked = 0;
    kmt_generic_spin_unlock(&ccpu.lock);
}

static void kmt_spin_lock(spinlock_t *lk)
{   
    kmt_ccpu_lock();
    ccpu.acquired++;
    kmt_conditional_disable_interrupt();
    kmt_ccpu_unlock();
    kmt_generic_spin_lock(lk);
}

static void kmt_spin_unlock(spinlock_t *lk)
{   
    kmt_generic_spin_unlock(lk);
    kmt_ccpu_lock(lk);
    ccpu.acquired--;
    kmt_ccpu_unlock(lk);
    kmt_try_enable_interrupt();
}

static __always_inline thread_t * kmt_thread_alloc(task_t *task)
{   
    thread_t *thread = pmm->alloc(sizeof(thread_t));

    thread->task = task;
    thread->status = NOT_READY;

    return thread;
}

static __always_inline void kmt_thread_set_entry(thread_t *thread,const char *name, void (*entry)(void *arg), void *arg)
{
    task_t *task = thread->task;
    void *rsp = pmm->alloc(STACKSIZE);
    Area kstack = {.start = rsp, .end = rsp + STACKSIZE};

    task->arg = arg;
    task->stack = kstack;
    task->entry = entry;
    task->context = kcontext(kstack,entry,arg);

    thread->status = READY;
}

static __always_inline void kmt_thread_free(thread_t *thread)
{
    pmm->free(thread->task->stack.start);
    pmm->free(thread);
}

static __always_inline void kmt_threadpool_insert(threadpool_t *tpool,thread_t *thread)
{
    NODE_T(thread) *nthread = pmm->alloc(sizeof(NODE_T(thread)));
    NODE_INIT(nthread,thread);
    kmt_spin_lock(&tpool->lock);
    LIST_INSERT(tpool->pool,nthread);
    kmt_spin_unlock(&tpool->lock);
}

static __always_inline void kmt_threadpool_remove(threadpool_t *tpool,task_t *task)
{
    NODE_T(thread) *nthread = NULL;
    thread_t *thread = NULL;

    kmt_spin_lock(&tpool->lock);
    LIST_DELETE_BY_ELM_FIELD(thread,tpool->pool,task,task,nthread);
    kmt_spin_unlock(&tpool->lock);
    panic_on(nthread == NULL, "task not found!");
    NODE_GET_ELM(nthread,thread);
    panic_on(thread == NULL, "Find null thread?");
    if(thread->status == BLOCKED){
        thread->status = DEAD;
    }else{
        kmt_thread_free(thread);
        pmm->free(nthread);
    }
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg)
{   
    thread_t *thread = kmt_thread_alloc(task);
    kmt_thread_set_entry(thread,name,entry,arg);
    kmt_threadpool_insert(kthreadpool,thread);
    return 0;
}

static void kmt_teardown(task_t *task)
{
    kmt_threadpool_remove(kthreadpool,task);
}

static void kmt_sem_init(sem_t *sem, const char *name, int value)
{   
    sem->wthreads = pmm->alloc(sizeof(LIST_T(thread)));
    LIST_INIT(sem->wthreads);
    sem->value = value;
    sem->name = name;
    kmt_spin_init(&sem->lock,NULL);
}

static void kmt_sem_wait(sem_t *sem)
{   
    kmt_spin_lock(&sem->lock);
    if(sem->value == 0){
        ccpu.tcurrent->status = BLOCKED;
        QUEUE_PUSH(sem->wthreads,ccpu.ncurrent);
        kmt_spin_unlock(&sem->lock);
        yield();
    }else{
        sem->value--;
        ccpu.tcurrent->status = RUNNING;
        kmt_spin_unlock(&sem->lock);
    }
}

static void kmt_sem_post(sem_t *sem)
{
    NODE_T (thread) *nthread = NULL;
    thread_t *thread = NULL;
    kmt_spin_lock(&sem->lock);
    if(sem->value == 0){
        while(sem->wthreads->length>0){
            QUEUE_POP(sem->wthreads,nthread);
            NODE_GET_ELM(nthread,thread);
            if(thread->status == BLOCKED){
                thread->status = RUNNING;
                break;
            }else{
                kmt_thread_free(thread);
                pmm->free(nthread);
            }
        }
    }
    sem->value++;
    kmt_spin_unlock(&sem->lock);
}

void kmt_context_save(Event ev, Context *ctx)
{  
   kmt_ccpu_lock();
   ccpu.tcurrent->task->context = ctx;
   kmt_ccpu_unlock();
}

Context *kmt_context_schedule(Event ev,Context *ctx)
{   
    NODE_T(thread) *nthread = NULL;
    thread_t *thread = NULL;
    Context *next = NULL;
    kmt_spin_lock(&kthreadpool->lock);
    ccpu.interrupted = 0;
    LIST_FOREACH(kthreadpool->pool,nthread){
        NODE_GET_ELM(nthread,thread);
        if(thread->status == WAITING || thread->status == READY){
            ccpu.tcurrent->status = WAITING;
            ccpu.tcurrent = thread;
            ccpu.ncurrent = nthread;
            next = thread->task->context;
        }
    }
    ccpu.interrupted = 0;
    kmt_spin_unlock(&kthreadpool->lock);
    return next;
}

static void kmt_cpu_init(int cpuid)
{   
    NODE_T(thread) *nthread = pmm->alloc(sizeof(NODE_T(thread)));
    thread_t *thread = kmt_thread_alloc(NULL);
    NODE_INIT(nthread,thread);

    cpus[cpuid].acquired = 0;
    cpus[cpuid].cpuid = cpuid;
    cpus[cpuid].interrupted = 0;
    cpus[cpuid].tcurrent = thread;
    cpus[cpuid].ncurrent = nthread;
    kmt_spin_init(&cpus[cpuid].lock, NULL);
}

static void kmt_init()
{
    cpus = pmm->alloc(MAXCPUS * sizeof(cpu_t));
    for(int i=0;i<MAXCPUS;i++)
        kmt_cpu_init(i);

}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = kmt_create,
    .sem_init = kmt_sem_init,
    .sem_signal = kmt_sem_post,
    .sem_wait = kmt_sem_wait,
    .spin_init = kmt_spin_init,
    .spin_lock = kmt_spin_lock,
    .spin_unlock = kmt_spin_unlock,
    .teardown = kmt_teardown
};
