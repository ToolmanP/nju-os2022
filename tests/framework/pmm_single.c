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
int len;

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
    node_t **curr = &list;
    void *ptr;
    int i;
    for(i=0;i<pos;i++){
        printf("%p\n",(*curr)->ptr);
        curr = &((*curr)->next);
    }
        

    ptr = (*curr)->ptr;
    *curr = (*curr)->next;
    len--;
    return ptr;
}

static inline void *op_alloc(int sz)
{
    void *ptr = pmm->alloc(sz);
    node_t *elm = malloc(sizeof(node_t));
    elm->ptr = ptr;
    len++;
    return ptr;
}

static inline void *op_free()
{   

    if(len == 0)
        return (void *)-1;
    printf("begin op_free\n");
    int pos = rand()%len;
    printf("pos: %d len: %d\n",pos,len);
    void *ptr = delete_list(pos);
   
    printf("ptr: %p\n",ptr);
    pmm->free(ptr);
    printf("end of op_free\n");
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