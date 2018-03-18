#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <semaphore.h>

#include "job.h"

#define QUEUE_SIZE 16
#define atomic _Atomic

typedef struct queue {
    job jobs[QUEUE_SIZE];
    unsigned int qii; // Input index.
    unsigned int qjj; // Output index.
    sem_t isem; // Space
    sem_t osem; // Items
} queue;

queue* make_queue();
void   free_queue(queue* qq);

void queue_put(queue* qq, job jj);
job  queue_get(queue* qq);

#endif
