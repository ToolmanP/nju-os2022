#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

#define MAXCPUS 8
#define PGBITS 12
#define PGSIZE 1<<PGBITS
#define BITMAPLEVELS 18
#define BITMAPMAX 1<<BITMAPLEVELS
#define MALLOCMAX 1<<24
#define MALLOC_FAILURE (uintptr_t)-1

#define lch(x) (x<<1ul)
#define rch(x) ((x<<1ul)|1ul)
#define laddr(addr,cur) (addr)
#define raddr(addr,cur) (addr | (1ul << cur))

#define noinline __attribute__((noinline))

typedef struct _slab{
    void *ptr;
    struct _slab *next;
} slab_t;

typedef struct _shard{
    void *pg,*shard_base;
    struct _slab *free;
} shard_t;

typedef struct _arena{
    shard_t shards[PGBITS];
} arena_t;
