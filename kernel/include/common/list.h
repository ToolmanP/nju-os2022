#ifndef __LIST_H
#define __LIST_H


#define _NODE(type) _##type##_node
#define _LIST(type) _##type##_list

#define NODE_T(type) type##_node_t
#define LIST_T(type) type##_list_t

#define NODE_TYPEDEF(elmtype,type)\
typedef struct _NODE(type){\
    elmtype elm;\
    struct _NODE(type) *next,*prev;\
} NODE_T(type)

#define LIST_TYPEDEF(type)\
typedef struct _LIST(type){\
    int length;\
    NODE_T(type) *lnode;\
} LIST_T(type)

#define NODE_INIT(node,el)\
do{\
    (node)->elm = el;\
    (node)->next = (node)->prev = NULL;\
}while(0)

#define NODE_GET_ELM(node,el)\
do{\
    el = (node)->elm;\
}while(0)

#define NODE_SET_ELM(node,el)\
do{\
    (node)->elm = el;\
}while(0)
#define LIST_INIT(li)\
do{\
    (li)->lnode = NULL;\
    (li)->length = 0;\
}while(0)

#define LIST_INSERT(li,node)\
do{\
    (li)->length++;\
    (node)->next = (li)->lnode;\
    (li)->lnode->prev = node;\
    (li)->lnode = node;\
}while(0)

#define LIST_DELETE_BY_ELM(type,li,el,node)\
do{\
    NODE_T(type) **curr;\
    for(curr = &(li)->lnode; *curr; ){\
        if(*curr == node){\
            node = *curr;\
            *curr = (*curr)->next;\
            break;\
        }else{\
            *curr = &(*curr)->next;\
        }\
    }\
    (li)->length--;\
}while(0)

#define LIST_DELETE_BY_ELM_FIELD(type,li,el,field,node)\
do{\
    NODE_T(type) **curr;\
    for(curr = &(li)->lnode; *curr; ){\
        if((*curr)->elm->field == el){\
            node = *curr;\
            *curr = (*curr)->next;\
            (li)->length--;\
            break;\
        }else{\
            curr = &(*curr)->next;\
        }\
    }\
    (li)->length--;\
}while(0)

#define LIST_DELETE_BY_POS(type,li,pos,node)\
do{\
    NODE_T(type) **curr = &(li)->lnode;\
    for(int i=0;i<pos;i++){\
        curr = &((*curr)->next);\
    }\
    node = *curr;\
    *curr = (*curr)->next;\
    (li)->length--;\
}while(0)

#define LIST_FOREACH(li,node)\
for(node = li->lnode; node ; node=node->next)
#endif