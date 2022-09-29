

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <common.h>
#include <klib.h>
#include <malloc.h>

#include <common/list.h>
#include <modules/pmm.h>

NODE_TYPEDEF(void *,addr);
LIST_TYPEDEF(addr);
LIST_T(addr) *li;

static inline void insert_list(void *ptr)
{
    NODE_T(addr) *node = malloc(sizeof(NODE_T(addr)));
    NODE_INIT(node,ptr);
    LIST_INSERT(li,node);
}

static inline void *delete_list(int pos)
{
    NODE_T(addr) *node;
    LIST_DELETE_BY_POS(addr,li,pos,node);
    void *ret = node->elm;
    free(node);
    return ret;
}

static inline void *op_alloc(int sz)
{
    void *ptr = pmm->alloc(sz);
    insert_list(ptr);
    return ptr;
}

static inline void *op_free()
{   
    if(li->length == 0)
        return (void *)-1;
    int pos = rand()%(li->length);
    void *ptr = delete_list(pos);
    pmm->free(ptr);
    return ptr;
}

static inline int generate_size(int mode)
{
    switch(mode){
        case 0:
            return rand()%(PGSIZE-1)+1; // Small shard memory allocation
        case 1:
            return rand()%(MALLOCMAX-PGSIZE)+PGSIZE; // Large shard memory allocation
        case 2:
            return rand()%(MALLOCMAX-1)+1; // Mixed intensive testing
        default:
            assert(0);
    }
    return -1;
}
static void single_thread_stress_test(int ntimes,int mode)
{   
    void *ptr;
    int sz;
    UNUSED(sz);
    while(ntimes--){
        if((rand() % 2) == 0){
            sz = generate_size(mode);
            printf("A,%p,%d\n",op_alloc(sz),sz);
        }else{
            ptr = op_free();
            if(ptr == (void *)-1)
                continue;
            else
                printf("F,%p\n",ptr);
        } 
    }
    
}

int main(const char *args)
{   
    int ntimes = atoi(args);
    li = malloc(sizeof(NODE_T(addr)));
    LIST_INIT(li);      
    srand(time(NULL));
    os->init();
    single_thread_stress_test(ntimes,2);
    return 0;
}