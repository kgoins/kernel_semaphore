#include "semaphore.h"

static struct semaphore_t* find_semaphore (const char* semName, struct proc* targ_proc) {
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

	struct sys_allocate_semaphore_args* uap = v;
    int err;

    struct semaphore_t newSem;

    /* vars from usr space */
    char* name;

    /* pull name from userspace and check name length */
    err = copyinstr( SCARG(uap,name), &name, SYS_SEM_NAME_MAX, NULL);

    /* error checking */
    if(err == ENAMETOOLONG) {
        *retval = ENAMETOOLONG;
        return (0);
    }
    if( sys_sem_count >= SYS_SEM_MAX ) {
        *retval = ENOMEM;
        return (0);
    }
    if(find_semaphore(name, p) != NULL) {
        *retval = EEXIST;
        return (0);
    }

    /* create new semaphore */
    newSem = (struct semaphore_t*) malloc(sizeof(semaphore_t), M_SUBPROC, M_WAITOK);

    copystr(name,newSem->name,SYS_SEM_NAME_MAX+1,NULL); /* set name */
    newSem->count = SCARG(uap, initCount);
    newSem->lock = (struct lock*) malloc(sizeof(lock), M_SUBPROC, M_WAITOK);
    newSem->s_lock = (struct simplelock*) malloc(sizeof(simplelock), M_SUBPROC, M_WAITOK);

    lockinit(newSem->lock, 0, "waiting on semaphore", 0, M_WAITOK);
    simple_lock_init(newSem->s_lock);
    SIMPLEQ_INIT(newSem->head);

    LIST_INSERT_HEAD(p->sem_list, newSem, next_sem);

    return (0);
}

int sys_free_semaphore(struct proc* p, void* v, register_t* retval) {

     struct sys_free_semaphore_args* uap = v;
     char* name;

     struct semaphore_t* sem; /* sem_list loop var  */
     struct entry* np; /* sem simpleq loop var  */
     
     copystr(SCARG(uap,name), name, SYS_SEM_NAME_MAX+1, NULL); /* set name */
     sem = find_semaphore(name, p);

     if (sem == NULL) {
         retval* ENOENT;
         return (0);
     }

     LIST_FOREACH(sem, &(targ_proc->sem_list), next_sem)
         if (strcmp(sem->name, name))
             break;

     LIST_REMOVE(sem, next);

     wakeup(sem);

     SIMPLEQ_FOREACH(np, sem->head, next_proc)
         free(np, M_SUBPROC);

     free(sem->lock, M_SUBPROC);
     free(sem->s_lock, M_SUBPROC);

     free(sem, M_SUBPROC);

     /* lock */
     sys_sem_count--;
     /* unlock */

    return (0)
}

static int proc_in_queue (struct proc* p, struct semaphore_t* sem) {
    /* if the thread is in sem's wait queue, return 1
     * else, return 0
     */

    struct entry* np; /* loop variable */
    SIMPLEQ_FOREACH(np, &(sem->head), next) {
        if(np->p->pid == p->pid)
            return 1;
    }

    return 0;
}

int sys_down_semaphore(struct proc* p, void* v, register_t* retval) {

    struct sys_down_semaphore_args* uap = v;
	char* name;

    int err;
    struct entry* np; /* will hold node for process wait queue if needed  */
    struct semaphore_t* sem; 

    copystr(SCARG(uap,name), name, SYS_SEM_NAME_MAX+1, NULL); /* set name */
    sem = find_semaphore(name, p);

    if (sem == NULL) {
        retval* = ENOENT;
        return (0);
    }

    err = lockmgr(sem->lock, LK_EXCLUSIVE, sem->s_lock, p);
    if (err != 0)
        return (err);

    /* procs must wait if no resources are available, */
    /* or if they are already waiting */
    while (sem->count == 0 ) {

        if ( !proc_in_queue(p, sem) ) {
            /* create new queue entry from current thread */
            np = (struct entry*) malloc(sizeof(entry), M_SUBPROC, M_WAITOK);
            np->p = p;

            /* enqueue */
            SIMPLEQ_INSERT_TAIL(&(sem->head), np, next);
        }

        err = lockmgr(sem->lock, LK_RELEASE, sem->s_lock, p);
        if (err != 0)
            return (err);

        tsleep(*sem, 0, "proc waiting on current sem's count", 0);

        if (find_semaphore(sem->name, p) == NULL) {
            retval* ECONNABORTED;
            return (0);
        }

        while(proc_in_queue(p, sem))
            tsleep(*sem, 0, "proc waiting on current sem", 0);

        err = lockmgr(sem->lock, LK_EXCLUSIVE, sem->s_lock, p);
        if (err != 0)
            return (err);
    }

    sem->count--;

    err = lockmgr(sem->lock, LK_RELEASE, sem->s_lock, p);
    if (err != 0)
        return (err);

    return (0)
}

int sys_up_semaphore(struct proc* p, void* v, register_t* retval) {

	struct sys_up_semaphore_args* uap = v;
    char* name;

    int err;
    struct entry* np; /* will hold node for process wait queue if needed  */
    struct semaphore_t* sem; 

    copystr(SCARG(uap,name), name, SYS_SEM_NAME_MAX+1, NULL); /* set name */
    sem = find_semaphore(name, p);

    if (sem == NULL) {
        retval* = ENOENT;
        return (0);
    }

    err = lockmgr(sem->lock, LK_EXCLUSIVE, sem->s_lock, p);
    if (err != 0)
        return (err);

	if (sem->count == 0) {
		sem->count++;

		/* dequeue and free head of queue */
		queue_head = SIMPLEQ_FIRST(&(sem->head));
		SIMPLEQ_REMOVE_HEAD(&(sem->head), queue_head, next);
		free(queue_head, M_SUBPROC);

        wakeup(*sem);
	} 

	else sem->count++;

    err = lockmgr(sem->lock, LK_RELEASE, sem->s_lock, p);
    if (err != 0)
        return (err);

    return (0)
}
