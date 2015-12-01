#include "semaphore.h"

int sys_sem_count = 0;

static int thread_in_queue (pthread_t thread, semaphore_t* sem) {
    /* if the thread is in sem's wait queue, return 1
     * else, return 0
     */

    entry* np; /* loop variable */

    SIMPLEQ_FOREACH(np, &(sem->head), next) {
        if(pthread_equal(np->thread, thread))
            return 1;
    }
    return 0;
}

semaphore_t* find_semaphore (const char* semName, proc_sim_t* targ_proc) {
    node* np;
    if(targ_proc->parent == NULL) 
        return NULL;

    /* search targ_proc's list of semaphores */
    LIST_FOREACH(np, &(targ_proc->list_head), next)
        if(strcmp(np->sem->name, semName))
            return np->sem;

    /* search proc's parent */
    find_semaphore(semName, targ_proc->parent);

    return NULL;
}

int createSem ( const char* name, int initCount , proc_sim_t* this_proc) {
    semaphore_t* newSem; /* the newly created semaphore */
    node* np; /* will hold the new sem in its proc's sem list */

    if ( sys_sem_count >= SYS_SEM_MAX )
        return ENOMEM;

    if ( strlen(name) > SYS_SEM_NAME_MAX )
        return ENAMETOOLONG;

    if ( find_semaphore(name, this_proc) != NULL )
        return EEXIST;

    /* create new semaphore */
    newSem = (semaphore_t*) malloc(sizeof(semaphore_t));


    /* set name and count*/
    newSem->count = initCount;
    strcpy(newSem->name, name);

    /* init lock vars */
    pthread_mutex_init(&newSem->mutex, NULL);
    pthread_cond_init(&newSem->cond, NULL);

    /* init queue head */
    /* (expansion of the SIMPLEQ_HEAD_INITIALIZER macro in queue.h) */
    newSem->head.sqh_first = NULL;
    newSem->head.sqh_last = &(newSem->head).sqh_first;

    /* set this sem's proc */
    newSem->my_proc = this_proc;

    /* wrap sem in node and add to its proc's sem list */
    np = (node*) malloc(sizeof(node));
    np->sem = newSem;

    LIST_INSERT_HEAD( &(newSem->my_proc->list_head), np, next);

    /* lock */
    ++sys_sem_count;
    /* unlock */

    return 0;
}

int free_semaphore (semaphore_t* targSem) {
    node* np; /*loop var*/

    if (find_semaphore(targSem->name, targSem->my_proc) == NULL)
        return ENOENT;

    pthread_mutex_destroy(&targSem->mutex);
    pthread_cond_destroy(&targSem->cond);

    /* find semaphore and return its containing node */
    LIST_FOREACH(np, &(targSem->my_proc->list_head), next)
        if (strcmp(targSem->name, np->sem->name))
            break;

    /* remove node from list and destroy node */
    LIST_REMOVE(np,next);
    free(np);

    free(targSem);

    /* lock */
    sys_sem_count--;
    /* unlock */

    return 0;
}

int down (semaphore_t* sem) {
    entry* np; /* will hold node for process wait queue if needed  */

    if (find_semaphore(sem->name, sem->my_proc) == NULL)
        return ENOENT;

    printf("Count: %d\n", sem->count);
    pthread_mutex_lock(&sem->mutex);

    /* threads must wait if no resources are available, */
    /* or if they are already waiting */
    while (sem->count == 0 || thread_in_queue(pthread_self(), sem)) {
        printf("Thread is waiting\n");

        if ( !thread_in_queue(pthread_self(), sem) ) {
            /* create new queue entry from current thread */
            np = (entry*) malloc(sizeof(entry));
            np->thread = pthread_self();

            /* enqueue */
            SIMPLEQ_INSERT_TAIL(&(sem->head), np, next);
        }
        
        pthread_cond_wait(&sem->cond, &sem->mutex);
        if (find_semaphore(sem->name, sem->my_proc) == NULL)
            return ECONNABORTED;
    }

    sem->count--;
    pthread_mutex_unlock(&sem->mutex);

    return 0;
}

int up (semaphore_t* sem) {
    entry* queue_head;

    if (find_semaphore(sem->name, sem->my_proc) == NULL)
        return ENOENT;

    pthread_mutex_lock(&sem->mutex);

    if (sem->count == 0) {
        sem->count++;

        /* dequeue and free head of queue */
        queue_head = SIMPLEQ_FIRST(&(sem->head));
        SIMPLEQ_REMOVE_HEAD(&(sem->head), queue_head, next);
        free(queue_head);

        pthread_cond_broadcast(&sem->cond);
    } 
    
    else sem->count++;

    pthread_mutex_unlock(&sem->mutex);

    return 0;
}
