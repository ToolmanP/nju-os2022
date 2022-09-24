#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <execinfo.h>

#include <sys/queue.h>
#include <time.h>
#include <common.h>
#include <klib.h>
#include <malloc.h>

typedef struct _node {
    void *ptr;
    SLIST_ENTRY(_node) field;
} node_t;

typedef SLIST_HEAD(_head,_node) head_t;

head_t *alloc_hd;
int len;

static inline void *op_alloc(int sz)
{
    void *ptr = pmm->alloc(sz);
    node_t *elm = malloc(sizeof(node_t));
    elm->ptr = ptr;
    len++;
    SLIST_INSERT_HEAD(alloc_hd,elm,field);
    return ptr;
}

static inline void *op_free()
{   

    printf("op_free\n");
    if(len == 0)
        return (void *)-1;

    int pos = rand()%len;
    printf("pos: %d len: %d\n",pos,len);
    node_t *elm = NULL;
    SLIST_FOREACH(elm,alloc_hd,field){
        if(pos==0)
            break;
        else
            pos--;
    }
    SLIST_REMOVE(alloc_hd,elm,_node,field);
    len--;
    pmm->free(elm->ptr);
    printf("end of op_free\n");
    return elm->ptr;
}

static void single_thread_stress_test(int ntimes)
{   
    void *ptr;
    while(ntimes--){
        if((rand() % 2) == 0){
            printf("A,%p\n",op_alloc((rand() % MALLOCMAX)+1));
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
    alloc_hd = malloc(sizeof(head_t));
    SLIST_INIT(alloc_hd);
    srand(time(NULL));
    os->init();
    single_thread_stress_test(ntimes);
    return 0;
}