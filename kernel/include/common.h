
#ifndef __COMMON_H
#define __COMMON_H
#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

#define MAXCPUS 8
#define PGBITS 12
#define PGSIZE (1<<PGBITS)
#define BITMAPLEVELS 17
#define BITMAPMAX (1<<BITMAPLEVELS)
#define MALLOCMAX (1<<14)
#define MALLOC_FAILURE ((uintptr_t)-1)

#define lch(x) (x<<1ul)
#define rch(x) ((x<<1ul)|1ul)
#define laddr(addr,cur) (addr)
#define raddr(addr,cur) (addr | (1ul << (cur-1)))
#define UNUSED(x) (void)x

#define PAGE_TRACE 1
#define SLAB_TRACE 0

#define noinline __attribute__((noinline))


typedef struct _slab{
    void *ptr;
    struct _slab *next;
    int nbits;
} slab_t;

typedef struct _shard{
    void *pg,*slab_ptr;
    size_t unallocated,slots;
    struct _slab *free;
    int _sh_lock;
} shard_t;

typedef struct _arena{
    shard_t shards[PGBITS];
    struct _slab *allocated;
    int _al_lock;
    int _cpuid;
} arena_t;

#define SLOT_MAX (PGSIZE/(sizeof(slab_t)))

#endif