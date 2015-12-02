#include "semaphore.h"
#include "proc_sim.h"

void* run_p1(void* proc) {
    /* int err; */
    struct node* np;
    struct proc_sim_t* p1 = proc;
    
    allocate_semaphore("sem1", 3, p1);

    LIST_FOREACH(np, &(p1->list_head), next) {
        printf("Sem name: %s\n", np->sem->name);
    }

    printf("starting removal of sem1\n");
    free_semaphore("sem1", p1);

    LIST_FOREACH(np, &(p1->list_head), next) {
        printf("Sem name after removal: %s\n", np->sem->name);
    }

    pthread_exit(NULL);
}


int main (int argc, char const* argv[])
{
    struct proc_sim_t* p1;
    p1 = create_proc_sim(1);

    pthread_t thd1;
    pthread_create(&thd1, NULL, run_p1, (void*) p1);

    pthread_join(thd1, NULL);

    printf("cleaning up proc_sims\n");
    free_proc_sim(p1);
    
    return 0;
}
