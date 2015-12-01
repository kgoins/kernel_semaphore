#include "proc_sim.h"

struct proc_sim_t* create_proc_sim (int pid) {
    struct proc_sim_t* newProc = (struct proc_sim_t*) malloc(sizeof(struct proc_sim_t));
    newProc->pid = pid;
    newProc->parent = NULL;
	
	newProc->list_head.lh_first = NULL;
	newProc->list_head.lh_last = &(newProc->list_head.lh_first);

    return newProc;
}

int free_proc_sim (struct proc_sim_t* targ_proc) {
    struct node* np;

    LIST_FOREACH(np, &(targ_proc->list_head), next) {
        free_semaphore(np->sem);
        free(np);
    }

    free(targ_proc);

    return 0;
}
