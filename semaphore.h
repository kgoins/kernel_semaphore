#ifndef SEM_H
#define SEM_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    SIMPLEQ_HEAD(head,entry) head;
    pthread_t headTID;

} semaphore_t;

typedef struct {
    pthread_t thread;
    SIMPLEQ_ENTRY(entry) next;
} entry;

semaphore_t* createSem(int initCount);
void destroySem(semaphore_t* target);

void down(semaphore_t* sem);
void up(semaphore_t* sem);

#endif
