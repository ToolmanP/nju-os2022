#ifndef __HEAP_H
#define __HEAP_H

#include <common.h>

#define HEAP_TYPE_DEFINE(type,cap)\
typedef struct _heap{\
    type elms[cap];\
    int nelms;\
    int (*compare)(type *a,type *b);\
    void (*swap)(type *a,type *b);\
}heap_t

#define HEAP_DEFINE(name) heap_t *name

#define HEAP_INIT(heap,compare)\
do{\
    heap->nelms = 0;\
    heap->op = compare;\
    heap->swap = swap;\
}while(0)

#define HEAP_PROCRATE_DOWN(heap,pos)\
do{\
    int nxt = pos;\
    while(nxt < heap->nelms){\
        nxt = 2*pos;\
        if(nxt+1 < nelms && heap->compare(&heap->elms[nxt],&heap->elms[nxt+1]) == 1){\
            nxt++;\
        }\
        if(heap->compare(&heap->compare(&heap->elms[pos],&heap->elms[nxt])) == 1){\
            heap->swap(&heap->elms[pos],&heap->elms[nxt]);\
            pos = nxt;\
        }else{\
            break;\
        }\
    }\
}while(0)

#define HEAP_INSERT(heap,elm)\
do{\
    heap->elms[++nelms] = elm;\
    for(int i=nelms>>1;i>=1;i--){\
        HEAP_PROCRATE_DOWN(heap,i);\
    }\
}while(0)

#endif