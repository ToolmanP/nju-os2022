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
    struct _node *next;
} node_t;


node_t *list;
int len = 0;

static inline void insert_list(void *ptr)
{
    node_t *node = malloc(sizeof(node_t));
    node->ptr = ptr;
    node->next = list;
    list = node;
    len++;
}

static inline void *delete_list(int pos)
{
    node_t *entry = NULL;
    node_t **curr = &list;
    for(int i=0;i<=pos;i++)
        curr = &((*curr)->next);
    assert(*curr != NULL);
    entry = *curr;
    assert(entry != NULL);
    void *ptr = entry->ptr;
    *curr = entry->next;

    return ptr; 
}

static inline void *op_alloc(int sz)
{
    void *ptr = pmm->alloc(sz);
    insert_list(ptr);
    return ptr;
}

static inline void *op_free()
{   

    if(len == 0)
        return (void *)-1;
    int pos = rand()%len;
    void *ptr = delete_list(pos);
    pmm->free(ptr);
    return ptr;
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
        printf("end\n");
    }
    
}

int main(const char *args)
{   
    int ntimes = atoi(args);
    list = malloc(sizeof(node_t));
    list -> ptr = NULL;
    list -> next = NULL;
    srand(time(NULL));
    os->init();
    single_thread_stress_test(ntimes);
    return 0;
}