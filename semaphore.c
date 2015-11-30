#include "semaphore.h"

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

semaphore_t* createSem ( const char* name, int initCount ) {
    /* create new semaphore */
    semaphore_t* newSem = (semaphore_t*) malloc(sizeof(semaphore_t));
    newSem->count = initCount;

    /* sanatize string and set semaphore name */
    strncpy(newSem->name,name,31);
    newSem->name[31] = '\0';

    /* init synch vars */
    pthread_mutex_init(&newSem->mutex, NULL);
    pthread_cond_init(&newSem->cond, NULL);

    /* init queue head */
    /* (expansion of the SIMPLEQ_HEAD_INITIALIZER macro in queue.h) */
    newSem->head.sqh_first = NULL;
    newSem->head.sqh_last = &(newSem->head).sqh_first;

    return newSem;
}

void destroySem (semaphore_t* targSem) {
    pthread_mutex_destroy(&targSem->mutex);
    pthread_cond_destroy(&targSem->cond);

    free(targSem);
}

void down (semaphore_t* sem) {
    printf("Count: %d\n", sem->count);
    pthread_mutex_lock(&sem->mutex);

    /* threads must wait if no resources are available, */
    /* or if they are already waiting */
    while (sem->count == 0 || thread_in_queue(pthread_self(), sem)) {
        printf("Thread is waiting\n");

        if ( !thread_in_queue(pthread_self(), sem) ) {
            /* create new queue entry from current thread */
            entry* np = (entry*) malloc(sizeof(entry));
            np->thread = pthread_self();

            /* enqueue */
            SIMPLEQ_INSERT_TAIL(&(sem->head), np, next);
        }
        
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }

    sem->count--;
    pthread_mutex_unlock(&sem->mutex);
}

void up (semaphore_t* sem) {
    entry* queue_head;
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
}
