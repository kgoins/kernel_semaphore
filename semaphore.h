#ifndef SEM_H
#define SEM_H

#include <sys/param.h>
#include <sys/acct.h>
#include <sys/systm.h>
#include <sys/ucred.h>
#include <sys/proc.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/malloc.h>
#include <sys/filedesc.h>
#include <sys/pool.h>

#include <sys/mount.h>
#include <sys/syscallargs.h>


#include <sys/errno.h>
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
