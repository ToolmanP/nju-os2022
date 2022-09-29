#include <common/lock.h>

int __os_acquire_spin_lock(int *value){
    return atomic_xchg(value,1); 
}

void __os_spin_unlock(int *value){
    atomic_xchg(value,0);
}