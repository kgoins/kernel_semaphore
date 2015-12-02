#include "semaphore.h"

int sys_sem_count = 0;

static int thread_in_queue (pid_t pid, struct semaphore_t* sem) {
    /* if the thread is in sem's wait queue, return 1
     * else, return 0
     */

    struct entry* np; /* loop variable */

    SIMPLEQ_FOREACH(np, &(sem->head), next) {
        if(pthread_equal(np->thread, thread))
            return 1;
    }
    return 0;
}

struct semaphore_t* find_semaphore (const char* semName) {
    struct node* np;

    printf("find: targ_proc pid is %d\n", targ_proc->pid);

    if(targ_proc == NULL) {
        printf("find: sem doesn't exist\n");
        return NULL;
    }

    /* search targ_proc's list of semaphores */
    LIST_FOREACH(np, &(targ_proc->list_head), next)
        if(strcmp(np->sem->name, semName)) {
            printf("Found sem: %s\n", np->sem->name);
            return np->sem;
        }

    /* search proc's parent */
    printf("Find: Starting recursion\n");
    find_semaphore(semName, targ_proc->parent);

    return NULL;
}

int allocate_semaphore( const char* name, int initCount , struct proc_sim_t* this_proc) {
    struct semaphore_t* newSem; /* the newly created semaphore */
    struct node* np; /* will hold the new sem in its proc's sem list */

    printf("alloc: starting\n");
    printf("alloc: this proc's pid is %d\n", this_proc->pid);

    if( sys_sem_count >= SYS_SEM_MAX )
        return ENOMEM;

    if( strlen(name) > SYS_SEM_NAME_MAX )
        return ENAMETOOLONG;

    if( find_semaphore(name, this_proc) != NULL )
        return EEXIST;

    /* create new semaphore */
    newSem = (struct semaphore_t*) malloc(sizeof(struct semaphore_t));

    /* set name and count*/
    newSem->count = initCount;
    strcpy(newSem->name, name);

    /* init lock vars */

    /* init queue head */
    /* (expansion of the SIMPLEQ_HEAD_INITIALIZER macro in queue.h) */
    newSem->head.sqh_first = NULL;
    newSem->head.sqh_last = &(newSem->head).sqh_first;

    /* set this sem's proc */
    newSem->my_proc = this_proc;

    /* wrap sem in node and add to its proc's sem list */
    np = (struct node*)malloc(sizeof(struct node));
    np->sem = newSem;

    LIST_INSERT_HEAD( &(this_proc->list_head), np, next);

    /* lock */
    ++sys_sem_count;
    /* unlock */
    printf("finished creating sem\n");

    return 0;
}

int free_semaphore (const char* name) {
    struct node* np; /*loop var*/
    struct semaphore_t* sem = find_semaphore(name, this_proc);

    if (sem == NULL) {
        printf("Sem wasn't found!\n");
        return ENOENT;
    }

    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->cond);

    LIST_FOREACH(np, &(sem->my_proc->list_head), next)
        if (strcmp(sem->name, np->sem->name))
            break;

    LIST_REMOVE(np,next);
    free(np);

    free(sem);

    /* lock */
    sys_sem_count--;
    /* unlock */

    return 0;
}

int down_semaphore(const char* name) {
    struct entry* np; /* will hold node for process wait queue if needed  */
    struct semaphore_t* sem = find_semaphore(name, this_proc);

    if (sem == NULL)
        return ENOENT;

    printf("Count: %d\n", sem->count);

    /* threads must wait if no resources are available, */
    /* or if they are already waiting */
    while (sem->count == 0 || thread_in_queue(pthread_self(), sem)) {
        printf("Thread is waiting\n");

        if ( !thread_in_queue(pthread_self(), sem) ) {
            /* create new queue entry from current thread */
            np = (struct entry*) malloc(sizeof(struct entry));
            np->thread = pthread_self();

            /* enqueue */
            SIMPLEQ_INSERT_TAIL(&(sem->head), np, next);
        }
        
        if (find_semaphore(sem->name, sem->my_proc) == NULL)
            return ECONNABORTED;
    }

    sem->count--;

    return 0;
}

int up_semaphore(const char* name) {
    struct entry* queue_head;
    struct semaphore_t* sem = find_semaphore(name, this_proc);

    if (sem == NULL)
        return ENOENT;

    if (sem->count == 0) {
        sem->count++;

        /* dequeue and free head of queue */
        queue_head = SIMPLEQ_FIRST(&(sem->head));
        SIMPLEQ_REMOVE_HEAD(&(sem->head), queue_head, next);
        free(queue_head);

    } 
    
    else sem->count++;

    return 0;
}
