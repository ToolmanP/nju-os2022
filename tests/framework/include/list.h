#ifndef __LIST_H
#define __LIST_H

#define NODE_TYPE_DEFINE(type)\
typedef struct _node{\
    type elm;\
    struct _node *next;\
} node_t

#define LIST_TYPE_DEFINE(type)\
NODE_TYPE_DEFINE(type);\
typedef struct _list{\
    int len;\
    node_t *head;\
} list_t

#define LIST_DEFINE(name) list_t *name
#define HEAD_DEFINE(name) node_t *name

#define LIST_INIT(li,hd)\
do{\
    li->head = hd;\
    li->len = 0;\
}while(0)

#define NODE_INIT(node,el)\
do{\
    node->elm = el;\
    node->next = NULL;\
}while(0)

#define LIST_INSERT(li,node)\
do{\
    li->len++;\
    node->next = li->head;\
    li->head = node;\
}while(0)

#define LIST_DELETE_ELM(li,elm,node)\
do{\
    node_t **curr;\
    node_t *entry;\
    node = NULL;\
    for(curr = &li->head;*curr;){\
        entry = *curr;\
        if(entry->elm == elm){\
            node = entry;\
            *curr = entry->next;\
            li->len--;\
            break;\
        }else{\
            curr = &entry->next;\
        }\
    }\
}while(0)

#define LIST_DELETE_POS(li,pos,node)\
do{\
    node_t **curr = &li->head;\
    for(int i=0;i<pos;i++){\
        curr = &((*curr)->next);\
    }\
    node = *curr;\
    *curr = node->next;\
    li->len--;\
}while(0)

#endif