#ifndef __LOCK_H
#define __LOCK_H

#include <common.h>

int __os_acquire_spin_lock(int *value);
void __os_spin_unlock(int *value);

#endif