#ifndef SEM_H
#define SEM_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "queue.h"
#include "proc_sim.h"

#define SYS_SEM_MAX 64
#define SYS_SEM_NAME_MAX 31

extern int sys_sem_count;

typedef struct entry {
    pthread_t thread;
    SIMPLEQ_ENTRY(entry) next;
} entry;

typedef struct {
    struct entry *sqh_first;
    struct entry **sqh_last;
} queuehead;

typedef struct semaphore_t {
    int count;
    char name[SYS_SEM_NAME_MAX];

    queuehead head;
    proc_sim_t* my_proc;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

} semaphore_t;

int allocate_semaphore(const char* name, int initCount);
int free_semaphore(semaphore_t* target);

int down(semaphore_t* sem);
int up(semaphore_t* sem);

semaphore_t* find_semaphore (const char* semName, proc_sim_t* targ_proc);

#endif
