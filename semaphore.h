#ifndef SEM_H
#define SEM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/proc.h>

#include "queue.h"

#define SYS_SEM_MAX 64
#define SYS_SEM_NAME_MAX 31

extern int sys_sem_count;

struct entry {
    int pid;
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
    int my_proc;
};

int allocate_semaphore(const char* name, int initCount, int this_proc);
int free_semaphore(const char* name, int this_proc);

int down_semaphore(const char* name, int this_proc);
int up_semaphore(const char* name, int this_proc);

struct semaphore_t* find_semaphore (const char* semName, int targ_proc);

#endif
