#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct _node{
    void *ptr;
    struct _node *next;
} node_t;

typedef struct _list{
    node_t *hd;
    int len;
} list_t;

static inline void insert_list(void *ptr)
{
    
}