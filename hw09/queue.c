/*#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>*/

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "queue.h"

// TODO: Make this an interprocess queue.

queue*
make_queue()
{
    int pages = 1 + sizeof(queue) / 4096;
    //queue* qq = malloc(pages * 4096); // FIXME: Queue should be shared.
    queue* qq = mmap(0, pages * 4096, PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    return qq;
}

void
free_queue(queue* qq)
{
    //assert(qq->qii == qq->qjj);
    int pages = 1 + sizeof(queue) / 4096;
    munmap(qq, pages * 4096);
    //printf("freeing complete\n");
    //free(qq); // FIXME: MAKE THIS WORK
}

void
queue_put(queue* qq, job msg)
{
    //printf("ADDING TO QUEUE\n");
    int rv;
    rv = sem_wait(&(qq->isem));
    assert(rv == 0);

    unsigned int ii = atomic_fetch_add(&(qq->qii), 1);
    qq->jobs[ii % QUEUE_SIZE] = msg;

    rv = sem_post(&(qq->osem));
    assert(rv == 0);
}

job
queue_get(queue* qq)
{
    //printf("GETTING FROM QUEUE\n");
    int rv;
    rv = sem_wait(&(qq->osem));
    assert(rv == 0);

    unsigned int jj = atomic_fetch_add(&(qq->qjj), 1);
    job yy = qq->jobs[jj % QUEUE_SIZE];

    rv = sem_post(&(qq->isem));
    assert(rv == 0);

    return yy;
}
