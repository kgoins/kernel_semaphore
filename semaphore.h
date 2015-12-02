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

struct entry {
    pthread_t thread;
    SIMPLEQ_ENTRY(entry) next;
};

struct queuehead {
    struct entry *sqh_first;
    struct entry **sqh_last;
} ;

struct semaphore_t {
    int count;
    char name[SYS_SEM_NAME_MAX];

    struct queuehead head;
    struct proc_sim_t* my_proc;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

};

int allocate_semaphore(const char* name, int initCount,
        struct proc_sim_t* this_proc);
int free_semaphore(const char* name, struct proc_sim_t* this_proc);

int down_semaphore(const char* name, struct proc_sim_t* this_proc);
int up_semaphore(const char* name, struct proc_sim_t* this_proc);

struct semaphore_t* find_semaphore (const char* semName, 
        struct proc_sim_t* targ_proc);

#endif
