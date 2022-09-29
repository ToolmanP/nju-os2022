#include <os.h>
#include <modules/kmt.h>

static cpu_t *cpus;
static NODE_T(thread) *ctnodes[MAXCPUS];

#define ccpu (cpus[cpu_current()])
#define ctnode (ctnodes[cpu_current()])

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg);
static void kmt_teardown(task_t *task);
static void kmt_spin_lock(spinlock_t *lk);
static void kmt_spin_unlock(spinlock_t *lk);
static void kmt_sem_init(sem_t *sem, const char *name, int value);
static void kmt_sem_wait(sem_t *sem);
static void kmt_sem_post(sem_t *sem);
static void kmt_init();

__always_inline void kmt_context_save(Event ev, Context *ctx)
{   
    UNUSED(ev);
    thread_t *thread;
    NODE_GET_ELM(ctnode,thread);
    thread->task->context = ctx;    
}

__always_inline Context *kmt_context_switch(Event ev,Context *ctx)
{   
    UNUSED(ev);
    NODE_T(thread) *node;
    thread_t *thread;
    Context *next = NULL;
    kmt_spin_lock(&ccpu.lock);
    LIST_FOREACH(thread,ccpu.kthreads,node){
        NODE_GET_ELM(node,thread);
        if(node != ctnode && thread->status == RUNNING){
            next = thread->task->context;
            ctnode = node;
        }
    }
    panic_on(next == NULL, "Returning null context");
    kmt_spin_unlock(&ccpu.lock);
    return next;
}

static __always_inline thread_t * thread_alloc(task_t *task,const char *name)
{
    thread_t *thread = pmm->alloc(sizeof(thread_t));
    task->stack = (Area){NULL,NULL};
    task->context = NULL;
    task->name = name;
    task->context = NULL;
    task->entry = NULL;
    thread->task = task;
    thread->status = RUNNING;
    return thread;
}

static __always_inline void thread_set_entry(thread_t *thread,void (*entry)(void *arg), void *arg)
{
    task_t *task = thread->task;
    void *rsp = pmm->alloc(STACKSIZE);
    Area kstack = {.start = rsp, .end = rsp+STACKSIZE};
    task->arg = arg;
    task->entry = entry;
    task->stack = kstack;
    task->context = kcontext(kstack,entry,arg);
}

static __always_inline void thread_free(thread_t *thread)
{
    pmm->free(thread->task->stack.start);
    pmm->free(thread->task);
    pmm->free(thread);
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg)
{   
    kmt_spin_lock(&ccpu.lock);
    NODE_T(thread) *thread_node = pmm->alloc(sizeof(NODE_T(thread)));
    thread_t *thread = thread_alloc(task,name);
    thread_set_entry(thread,entry,arg);
    NODE_INIT(thread_node,thread);
    LIST_INSERT(ccpu.kthreads,thread_node);
    kmt_spin_unlock(&ccpu.lock);
    return 0;
}

static void kmt_teardown(task_t *task)
{   
    NODE_T(thread) *node;
    thread_t *thread;

    kmt_spin_lock(&ccpu.lock);
    NODE_GET_ELM(ctnode,thread);
    int is_current = (thread->task == task) ? 1 : 0;
    LIST_DELETE_BY_ELM_FIELD(thread,ccpu.kthreads,task,task,node);
    NODE_GET_ELM(node,thread);
    if(thread->status == WAITING){
        thread->status = DEAD;
    }else{
        thread_free(thread);
        pmm->free(node);
    }
    kmt_spin_unlock(&ccpu.lock);

    if(is_current) // current thread's task is the task we need to tear down we need to perform context switch
        yield();
}

static void kmt_spin_init(spinlock_t *lk, const char *name)
{   
    lk->name = name;
    lk->locked = 0;
}

static void kmt_spin_lock(spinlock_t *lk)
{   
    while(atomic_xchg(&lk->locked,1) == 1){
        yield();
    }
}

static void kmt_spin_unlock(spinlock_t *lk)
{   
    atomic_xchg(&lk->locked,0);
}

static void kmt_sem_init(sem_t *sem, const char *name, int value)
{   
    sem->name = name;
    sem->value = value;
    sem->wthreads = pmm->alloc(sizeof(QUEUE_T(thread)));
    QUEUE_INIT(sem->wthreads);
    kmt_spin_init(&sem->lock,NULL);
}

static void kmt_sem_wait(sem_t *sem)
{   
    thread_t *thread;
    volatile int status;
    kmt_spin_lock(&sem->lock);
    if(sem->value == 0){
        status = WAITING;
    }else{
        sem->value--;
        status = RUNNING;
    }
    kmt_spin_unlock(&sem->lock);
    
    kmt_spin_lock(&ccpu.lock);
    NODE_GET_ELM(ctnode,thread);
    thread->status = status;
    if(status == WAITING){
        kmt_spin_lock(&sem->lock);
        QUEUE_PUSH(sem->wthreads,ctnode);
        kmt_spin_unlock(&sem->lock);
        kmt_spin_unlock(&ccpu.lock);
        yield();
    }else{
        kmt_spin_unlock(&ccpu.lock);
    }
}

static void kmt_sem_post(sem_t *sem)
{
    NODE_T(thread) *node;
    thread_t *thread;
    kmt_spin_lock(&sem->lock);
    if(sem->value == 0){
        while(sem->wthreads->length>0){
            QUEUE_POP(sem->wthreads,node);
            NODE_GET_ELM(node,thread);
            if(thread->status == DEAD){
                thread_free(thread);
                pmm->free(node);
            }else{
                thread->status == RUNNING;
                break;
            }
        }
    }
    sem->value++;
    kmt_spin_unlock(&sem->lock);
}

static void kmt_init()
{   
    task_t *task;
    thread_t *thread;

    cpus = pmm->alloc(MAXCPUS * sizeof(cpu_t));  
    for(int i=0;i<MAXCPUS;i++){
        cpus[i].kthreads = pmm->alloc(sizeof(LIST_T(thread)));
        LIST_INIT(cpus[i].kthreads);
        ctnodes[i] = pmm->alloc(sizeof(NODE_T(thread)));
        task = pmm->alloc(sizeof(task));
        thread = thread_alloc(task,"Startup");
        NODE_INIT(ctnodes[i],thread);
        LIST_INSERT(cpus[i].kthreads,ctnodes[i]);
    }
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
