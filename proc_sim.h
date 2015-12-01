#ifndef _PROC_SIM_H
#define _PROC_SIM_H 

#include "queue.h"
#include "semaphore.h"

/* 0 indicates null ptr */

struct node {
    struct semaphore_t* sem;
    LIST_ENTRY(node) next;
};

struct listhead {
    struct node* lh_first;
    struct node** lh_last;
};

struct proc_sim_t {
    int pid;
    struct proc_sim_t* parent;

    struct listhead list_head;
};

struct proc_sim_t* create_proc_sim (int pid);
int free_proc_sim (struct proc_sim_t* targ_proc);
    
#endif
