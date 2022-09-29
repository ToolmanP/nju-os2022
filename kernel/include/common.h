
#ifndef __COMMON_H
#define __COMMON_H

#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

#define MAXCPUS 8
#define PGBITS 12
#define PGSIZE (1<<PGBITS)
#define UNUSED(x) (void)x

#define PAGE_TRACE 0
#define SLAB_TRACE 0

#define slog(op,arena,slab)\
    printf(op"[S][cpu#%d][nbits:%d]: %p [%s:%d]\n",arena->_cpuid,slab->nbits,slab->ptr,__func__,__LINE__)

#define slog_ex(op,tl,ex,slab)\
    printf(op"[S][cpu#%d->#%d][nbits:%d]: %p [%s:%d]\n",ex->_cpuid,tl->_cpuid,slab->nbits,slab->ptr,__func__,__LINE__)

#define plog(op,ptr,cpuid)\
    printf(op"[G][cpu#%d]: %p [%s:%d]\n",cpuid,ptr,__func__,__LINE__)

#endif