#ifndef SEM_H
#define SEM_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

typedef struct entry {
    pthread_t thread;
    SIMPLEQ_ENTRY(entry) next;
} entry;

typedef struct {
    struct entry *sqh_first;
    struct entry **sqh_last;
} queuehead;

typedef struct {
    int count;
    queuehead head;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

} semaphore_t;

semaphore_t* createSem(int initCount);
void destroySem(semaphore_t* target);

void down(semaphore_t* sem);
void up(semaphore_t* sem);

#endif
