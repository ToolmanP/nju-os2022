#include <common.h>

static void *_sbrk = NULL;
static char *_pmm_bmap = NULL;
static arena_t arenas[MAXCPUS];
static int _pmm_global_lock = 0;

static inline int xchg(int *addr, int val)
{
  int result;
  asm volatile("lock xchg %0,%1":"+m"(*addr),"=a"(result):"1"(val));
  return result;
}

static inline int _pmm_try_spin_lock(int *lock)
{
  int lock_state = xchg(lock,1);
  return lock_state;
}

static inline void _pmm_spin_lock(int *lock)
{
  while(xchg(lock,1) == 1);
}

static inline void _pmm_spin_unlock(int *lock)
{
  xchg(lock,0);
}

static inline void _pmm_bmap_init(size_t size)
{
  _pmm_bmap = (char *)_sbrk;
  _sbrk += size;
}

static inline int _pmm_bmap_getbit(int x)
{
  return _pmm_bmap[x>>3] & (1<<(x&7));
}

static inline void _pmm_bmap_setbit(int x)
{
  _pmm_bmap[x>>3] |= 1<<(x&7);
}

static inline void _pmm_bmap_clearbit(int x)
{
  _pmm_bmap[x>>3] &= ~(1<<(x&7));
}

static inline int _pmm_nbits_round(size_t size)
{
  int r=0,set=size&1;

  while(size>>=1){
    set+=(size&1);
    r++;
  }

  if(set>1) r++;
  return r;
}

static uintptr_t pmm_global_alloc(uintptr_t addr, int x, int cur, int target)
{
  uintptr_t ret = MALLOC_FAILURE;
  if(cur == target){

    if(_pmm_bmap_getbit(x) == 1)
      return MALLOC_FAILURE;
    else{
      return addr;
    }

  }else{

    ret = pmm_global_alloc(laddr(addr,cur), lch(x), cur-1, target);

    if(ret != MALLOC_FAILURE){
      _pmm_bmap_setbit(lch(x));
      return ret;
    }

    ret = pmm_global_alloc(raddr(addr,cur), rch(x), cur-1, target);

    if(ret != MALLOC_FAILURE)
      _pmm_bmap_setbit(rch(x));

  }
  return ret;
}


static void pmm_global_free(uintptr_t addr,int x, int cur)
{
  if(cur == 0 || (_pmm_bmap_getbit(lch(x)) == 0 && _pmm_bmap_getbit(rch(x)) == 0)){
    _pmm_bmap_clearbit(x);
    return;
  }
  

  if(addr & (1ul << cur))
    pmm_global_free(addr, rch(x), cur-1);
  else
    pmm_global_free(addr, lch(x), cur-1);

  if(_pmm_bmap_getbit(lch(x)) == 0 && _pmm_bmap_getbit(rch(x)) == 0 && _pmm_bmap_getbit(x) == 1)
    _pmm_bmap_clearbit(x);
}


static inline void *kalloc_slow(int nbits,int cpuid)
{ 
  assert(nbits >= PGBITS);
  UNUSED(cpuid);
  _pmm_spin_lock(&_pmm_global_lock);
  uintptr_t npages = pmm_global_alloc(0,1,BITMAPLEVELS,nbits-PGBITS);
  _pmm_spin_unlock(&_pmm_global_lock);
  return _sbrk+(npages<<PGBITS);
}

static inline void kfree_slow(void *ptr,int cpuid)
{
  assert(((uintptr_t)ptr & (PGSIZE-1)) == 0);
  UNUSED(cpuid);
  uintptr_t page_ptr = ((uintptr_t)ptr-(uintptr_t)_sbrk)>>PGBITS;
  _pmm_spin_lock(&_pmm_global_lock);
  pmm_global_free(page_ptr,1,BITMAPLEVELS);
  _pmm_spin_unlock(&_pmm_global_lock);
}

static inline void *pgalloc()
{ 
  return kalloc_slow(PGBITS,0);
}

static inline void pmm_shard_allocate_slab_space(shard_t *shard,int nbits)
{
  shard->pg = pgalloc();
  shard->unallocated = PGSIZE/(nbits);
}

static inline void pmm_shard_allocate_page(shard_t *shard)
{
  shard->slab_ptr = pgalloc();
  shard->slots = PGSIZE/sizeof(slab_t);
}

static inline void pmm_shard_insert_free_slab(shard_t *shard, slab_t *slab)
{
  _pmm_spin_lock(&shard->_sh_lock);
  slab->next = shard->free;
  shard->free = slab;
  _pmm_spin_unlock(&shard->_sh_lock);
}

static inline slab_t *pmm_shard_fetch_free_slab(shard_t *shard)
{ 
  if(shard->free == NULL)
    return NULL;

  slab_t *slab = shard->free;
  shard->free = shard->free->next;
  shard->slots++;
  return slab;
}

static inline slab_t *pmm_shard_fetch_local_free_slab(shard_t *shard)
{ 
  
  slab_t *slab = NULL;
  _pmm_spin_lock(&shard->_sh_lock);
  slab = pmm_shard_fetch_free_slab(shard);
  _pmm_spin_unlock(&shard->_sh_lock);
  return slab;
}

static inline slab_t *pmm_shard_fetch_external_free_slab(shard_t *shard)
{
  slab_t *slab = NULL;
  if(_pmm_try_spin_lock(&(shard->_sh_lock)) == 0)
    slab = pmm_shard_fetch_free_slab(shard);
  _pmm_spin_unlock(&shard->_sh_lock);
  return slab;
}

static inline slab_t *pmm_shard_allocate_new_slab(shard_t *shard, int nbits)
{ 
  if(shard->unallocated == 0)
    pmm_shard_allocate_page(shard);
  
  if(shard->slots == 0)
    pmm_shard_allocate_slab_space(shard,nbits);
  
  slab_t *slab = (slab_t *)shard->slab_ptr;
  slab->nbits = nbits;
  slab->next = NULL;
  slab->ptr = shard->pg;

  shard->slab_ptr += sizeof(slab_t);
  shard->pg += 1<<nbits;

  return slab;
}

static inline void pmm_arena_insert_allocated_slab(arena_t *arena, slab_t *slab)
{
  _pmm_spin_lock(&arena->_al_lock);
  slab->next = arena->allocated;
  arena->allocated = slab;
  _pmm_spin_unlock(&arena->_al_lock);
}

static inline slab_t *pmm_arena_fetch_free_slab(int nbits,int cpuid)
{
  slab_t *slab;
  arena_t *tl_arena = &arenas[cpuid];
  shard_t *tl_shard = &tl_arena->shards[nbits];
  shard_t *ex_shard;

  if((slab = pmm_shard_fetch_free_slab(tl_shard)) != NULL)
    goto ret;
  
  for(int i=0;i<MAXCPUS;i++){
    if(i == cpuid)
      continue;
    ex_shard = &arenas[i].shards[nbits];
    if((slab = pmm_shard_fetch_external_free_slab(ex_shard)) != NULL)
      goto ret;
  }

  slab = pmm_shard_allocate_new_slab(tl_shard,nbits);

ret:
  pmm_arena_insert_allocated_slab(tl_arena,slab);
  return slab;
}

static inline slab_t *pmm_arena_search_allocated_slab(arena_t *arena, void *ptr)
{ 
  slab_t *slab = NULL;
  _pmm_spin_lock(&arena->_al_lock);
  for(slab_t **curr=&arena->allocated;*curr;){
    slab_t *entry = *curr;
    if(entry->ptr == ptr){
      slab = entry;
      break;
    }else{
      curr = &entry->next;
    }
  }
  _pmm_spin_unlock(&arena->_al_lock);
  return slab;
}

static inline void pmm_arena_free_slab(void *ptr,int cpuid)
{
  slab_t *slab;

  if((slab = pmm_arena_search_allocated_slab(&arenas[cpuid],ptr)) != NULL)
      goto free;

  for(int i=0;i<MAXCPUS;i++){
    if(i==cpuid)
      continue;
    if((slab = pmm_arena_search_allocated_slab(&arenas[i],ptr)) != NULL)
      goto free;
  }

free:
    pmm_shard_insert_free_slab(&arenas[cpuid].shards[slab->nbits],slab);
}

static inline void shard_init(shard_t *shard,int nbits)
{
  pmm_shard_allocate_page(shard);
  pmm_shard_allocate_slab_space(shard,nbits);
  shard->_sh_lock = 0;
}

static inline void arena_init(arena_t *arena)
{
  for(int i=0;i<PGBITS;i++)
    shard_init(&arena->shards[i],i);
  arena->_al_lock = 0;
}

static inline void *kalloc_fast(int nbits,int cpuid)
{
  return pmm_arena_fetch_free_slab(nbits,cpuid)->ptr;
}

static inline void kfree_fast(void *ptr,int cpuid)
{ 
  pmm_arena_free_slab(ptr,cpuid);
}


static void *kalloc(size_t size)
{
  if(size>MALLOCMAX || size == 0)
    return (void *)MALLOC_FAILURE;

  int nbits = _pmm_nbits_round(size);
  int cpuid = cpu_current();

  if(nbits < PGBITS)
    return kalloc_fast(nbits,cpuid);
  else
    return kalloc_slow(nbits,cpuid); 
  return NULL;
}

static void kfree(void *ptr)
{
  assert(IN_RANGE(ptr,heap));
  int cpuid = cpu_current();
  
  if(((uintptr_t) ptr & (PGSIZE-1)) == 0)
    kfree_slow(ptr,cpuid);
  else
    kfree_fast(ptr,cpuid);
}

static void pmm_init()
{
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  _sbrk = heap.start;
  _pmm_bmap_init(BITMAPMAX);
  for(int i=0;i<MAXCPUS;i++)
    arena_init(&arenas[i]);
  
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
