

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <common.h>
#include <klib.h>
#include <malloc.h>
#include <list.h>

LIST_TYPE_DEFINE(void *);
LIST_DEFINE(li);
HEAD_DEFINE(hd);

static inline void insert_list(void *ptr)
{
    node_t *node = malloc(sizeof(node_t));
    NODE_INIT(node,ptr);
    LIST_INSERT(li,node);
}

static inline void *delete_list(int pos)
{
    node_t *node;
    LIST_DELETE_POS(li,pos,node);
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
    if(li->len == 0)
        return (void *)-1;
    int pos = rand()%(li->len);
    void *ptr = delete_list(pos);
    pmm->free(ptr);
    return ptr;
}

static inline int generate_size(int mode)
{
    switch(mode){
        case 0:
            return rand()%(64)+1; // Small shard memory allocation
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
    hd = malloc(sizeof(node_t));
    li = malloc(sizeof(list_t));
    NODE_INIT(hd,NULL);
    LIST_INIT(li,hd);      
    srand(time(NULL));
    os->init();
    single_thread_stress_test(ntimes,0);
    return 0;
}