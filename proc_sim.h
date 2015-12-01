#ifndef _PROC_SIM_H
#define _PROC_SIM_H 

#include "queue.h"
#include "semaphore.h"

/* 0 indicates null ptr */

typedef struct node {
    struct semaphore_t* sem;
    LIST_ENTRY(node) next;
} node;

typedef struct listhead{
    struct node* lh_first;
    struct node** lh_last;
} listhead;

typedef struct proc_sim_t {
    int pid;
    struct proc_sim_t* parent;

    struct listhead list_head;
} proc_sim_t;


#endif
