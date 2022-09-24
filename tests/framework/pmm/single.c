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

static void single_thread_stress_test(int ntimes)
{   
    void *ptr;
    while(ntimes--){
        if((rand() % 2) == 0){
            printf("A,%p\n",op_alloc((rand() % 256)+1));
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
    single_thread_stress_test(ntimes);
    return 0;
}