#ifndef __QUEUE_H
#define __QUEUE_H

#include "list.h"

#define _QUEUE(type) _##type##_queue

#define QUEUE_T(type) type##_queue_t

#define QUEUE_TYPEDEF(type)\
typedef struct _QUEUE(type){\
    int length;\
    NODE_T(type) **front,**rear;\
    NODE_T(type) *lnode;\
} QUEUE_T(type)

#define QUEUE_INIT(queue)\
do{\
    (queue)->lnode = NULL;\
    (queue)->front = (queue)->rear = &((queue)->lnode);\
    (queue)->length = 0;\
}while(0)

#define QUEUE_PUSH(queue,node)\
do{\
    if((queue)->lnode == NULL){\
        ((queue)->lnode) = node;\
    }else{\
        (*((queue)->rear))->next = (node);\
        (node)->prev = *((queue)->rear);\
        (queue)->rear = &(*((queue)->rear))->next;\
        (queue)->length++;\
    }\
}while(0)

#define QUEUE_POP(queue,node)\
do{\
    if((queue)->lnode != NULL){\
        (node) = *((queue)->front);\
        *((queue)->front) = (*((queue)->front))->next;\
        (queue)->length--;\
    }\
}while(0)

#define QUEUE_DELETE_BY_ELM_FIELD(type,queue,el,field,node)\
do{\
    NODE_T(type) **curr;\
    for(curr = (queue)->front; *curr ;){\
        if((*curr)->elm->field == el){\
            if(*curr == *((queue)->rear)){\
                node = *curr;\
                (queue)->rear = &(*((queue)->rear))->prev;\
            }else{\
                node = *curr;\
                (queue)->front = &(*((queue)->front))->next;\
            }\
            (queue)->length--;\
        }else{\
            curr = &(*curr)->next;\
        }\
    }\
}while(0)

#endif