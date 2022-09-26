#include <os.h>

static void kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg)
{
        
}

static void kmt_teardown(task_t *task)
{

}

static void kmt_spin_init(spinlock_t *lk, const char *name)
{

}

static void kmt_spin_lock(spinlock_t *lk)
{

}

static void kmt_spin_unlock(spinlock_t *lk)
{

}

static void kmt_sem_init(sem_t *sem, const char *name, int value)
{
    sem->name = name;
    sem->value = value;
    kmt_spin_init(&sem->lock,NULL);
}

static void kmt_sem_wait(sem_t *sem)
{

}

static void kmt_sem_post(sem_t *sem)
{

}

static void kmt_init()
{

}

MODULE_DEF(kmt) = {
 
};
