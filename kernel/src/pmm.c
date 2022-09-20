#include <common.h>

static void *_sbrk = NULL;
static char *bmap = NULL;
static int locked = 0;

static arena_t arenas[MAXCPUS];
static int _pmm_lock = 0;

static inline int xchg(int *addr, int val);
static inline void xchg_lock();
static inline void xchg_unlock();
static inline void bmap_init(size_t size);
static inline int bmap_getbit(int x);
static inline void bmap_setbit(int x);
static inline void bmap_clearbit(int x);

static inline int xchg(int *addr, int val)
{
  int result;
  asm volatile("lock xchg %0,%1":"+m"(*addr),"=a"(result):"1"(val));
  return result;
}

static inline void xchg_lock()
{
  while(xchg(&_pmm_lock,1) == 1);
}

static inline void xchg_unlock()
{
  xchg(&_pmm_lock,0);
}

static inline void bmap_init(size_t size)
{
  bmap = (char *)_sbrk;
  _sbrk += size;
}

static inline int bmap_getbit(int x)
{
  return bmap[x>>3] & (1<<(x&7));
}

static inline void bmap_setbit(int x)
{
  bmap[x>>3] |= 1<<(x&7);
}

static inline void bmap_clearbit(int x)
{
  bmap[x>>3] &= ~(1<<(x&7));
}

static int power_round(size_t size)
{
  int r=0,set=size&1;

  while(size>>=1){
    set+=(size&1);
    r++;
  }

  if(set>1) r++;
  return r;
}

static uintptr_t pgalloc_search(uintptr_t addr, int x, int cur, int target)
{
  uintptr_t ret = MALLOC_FAILURE;
  if(cur == target){

    if(bmap_getbit(x) == 1)
      return MALLOC_FAILURE;
    else{
      return addr;
    }

  }else{

    ret = pgalloc_search(laddr(addr,cur), lch(x), cur-1, target);

    if(ret != MALLOC_FAILURE){
      bmap_setbit(lch(x));
      return ret;
    }

    ret = pgalloc_search(raddr(addr,cur), rch(x), cur-1, target);

    if(ret != MALLOC_FAILURE)
      bmap_setbit(rch(x));

  }
  return ret;
}


static void pgalloc_free(uintptr_t addr,int x, int cur, int target)
{
  if(cur == target){
    bmap_clearbit(x);
    return;
  }

  if(addr & (1ul << cur))
    pgalloc_free(addr, rch(x), cur-1, target);
  else
    pgalloc_free(addr, lch(x), cur-1, target);

  if(bmap_getbit(lch(x)) == 0 && bmap_getbit(rch(x)) == 0 && bmap_getbit(x) == 1)
    bmap_clearbit(x);
}


static inline void *kalloc_slow_path(int power)
{ 
  assert(power >= PGBITS);
  uintptr_t npages = pgalloc_search(0,1,BITMAPLEVELS,power-PGBITS);
  return _sbrk+(npages<<PGBITS);
}

static inline void *kalloc_fast_path(int power)
{
  return (void *)MALLOC_FAILURE;
}

static inline void *pgalloc()
{
  return kalloc_slow_path(PGBITS);
}

static inline void insert_free_slab(slab_t **free, void **shard_base, void *ptr)
{
  slab_t *new_slab = (slab_t *)(*shard_base);
  new_slab->ptr = ptr;
  new_slab->next = *free;
  *free = new_slab;
  *shard_base = *shard_base + sizeof(slab_t);
}

static inline slab_t *fetch_free_slab(slab_t **free)
{ 
  assert(*free != NULL);
  slab_t *free_slab = *free;
  *free = (*free)->next;
  return free_slab;
}

static inline void shard_init(shard_t *shard)
{
  shard->pg = pgalloc();
  shard->shard_base = pgalloc();
  shard->free = NULL;
}

static inline void arena_init(arena_t *arena)
{
  for(int i=0;i<PGBITS;i++)
    shard_init(&arena->shards[i]);
}

static void *kalloc(size_t size)
{
  if(size>MALLOCMAX || size == 0)
    return (void *)MALLOC_FAILURE;

  int power = power_round(size);

  if(power < PGBITS)
    return kalloc_fast_path(power);
  else
    return kalloc_slow_path(power); 
  return NULL;
}

static void kfree(void *ptr)
{

}

static void pmm_init()
{
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  _sbrk = heap.start;
  
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
