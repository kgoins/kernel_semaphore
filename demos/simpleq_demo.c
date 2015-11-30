#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
typedef struct entry {
        int data;
        SIMPLEQ_ENTRY(entry) next;
} entry;

int main (int argc, char const* argv[])
{
    entry* nodePtr; /*bitch var*/

    /* create entry */ 
    entry* item1 = (entry*) malloc(sizeof(entry));
    item1->data = 12;

    /* init queue */
    SIMPLEQ_HEAD(head, entry) head = SIMPLEQ_HEAD_INITIALIZER(head);

    SIMPLEQ_INSERT_TAIL(&head, item1, next);

    printf("Traversing the Queue\n");
    SIMPLEQ_FOREACH(nodePtr, &head, next)
        printf("entry %d\n", nodePtr->data);

    SIMPLEQ_REMOVE_HEAD(&head, (nodePtr = SIMPLEQ_FIRST(&head)), next );
    printf("removed entry %d\n", nodePtr->data);

    /* cleanup pointers */
    free(item1);

    return 0;
}
