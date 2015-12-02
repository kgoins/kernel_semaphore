#include "semaphore.h"

static struct semaphore_t* find_semaphore (const char* semName, struct proc targ_proc) {
    struct node* np;

    if(targ_proc == NULL)
        return NULL;

    /* search targ_proc's list of semaphores */
    LIST_FOREACH(np, &(targ_proc->list_head), next)
        if(strcmp(np->sem->name, semName)) {
            return np->sem;
        }

    /* search proc's parent */
    find_semaphore(semName, targ_proc->p_pptr);

    return NULL;
}

int sys_allocate_semaphore(struct proc* p, void* v, register_t* retval) {
    /* 
     * The arguments are passed in a structure defined as:
     * struct sys_my_syscall_args {
     *  syscallarg(char *) str;
     *  syscallarg(int) val;
     * }
     */

	struct sys_allocate_semaphore_args* uap = v;
    int err;
    char* name;

    err = copyinstr( SCARG(uap,name), &name, SYS_SEM_NAME_MAX, NULL);
   
    if( sys_sem_count >= SYS_SEM_MAX ) {
        *retval = ENOMEM;
        return (0);
    }

    if( err == ENAMETOOLONG ) {
        *retval = ENAMETOOLONG;
        return (0);
    }

    if( find_semaphore(name, p) != NULL )
        return EEXIST;
 

    return (0)
}
