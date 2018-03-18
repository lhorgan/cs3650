
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

#include "job.h"
#include "queue.h"
#include "factor.h"

static queue* iqueue;
static queue* oqueue;

static int worker_count = 0;
static pid_t workers[64];
static pid_t printer;

void
work_off_jobs()
{
    //printf("welp....");
    while (1) {
        //printf("Neato");
        job jj = queue_get(iqueue);
        if (jj.number < 0) {
            //job done = make_job(-1);
            //submit_job(done);
            //printf("terminating process");
            break;
        }

        factor(jj.number, &(jj.count), &(jj.factors[0]));

        queue_put(oqueue, jj);
    }
}

void
print_results(int64_t count)
{
    int64_t oks = 0;

    for (int64_t ii = 0; ii < count; ++ii) {
        job res = get_result();

        printf("%ld: ", res.number);
        int64_t prod = 1;
        for (int64_t jj = 0; jj < res.count; ++jj) {
            int64_t xx = res.factors[jj];
            prod *= xx;
            printf("%ld ", xx);
        }
        printf("\n");

        if (prod == res.number) {
            ++oks;
        }
        else {
            fprintf(stderr, "Warning: bad factorization");
        }
    }

    printf("Factored %ld / %ld numbers.\n", oks, count);
}

void
factor_wait_done()
{
    for (int ii = 0; ii < worker_count; ++ii) {
        waitpid(workers[ii], 0, 0);
    }
}

int
factor_init(int num_procs, int64_t count)
{
    if (iqueue == 0) iqueue = make_queue();
    if (oqueue == 0) oqueue = make_queue();

    sem_init(&(iqueue->isem), 1, QUEUE_SIZE);
    sem_init(&(iqueue->osem), 1, 0);
    sem_init(&(oqueue->isem), 1, QUEUE_SIZE);
    sem_init(&(oqueue->osem), 1, 0);

    //printf("NUM PROCS: %i\n", num_procs);

    // FIXME: Spawn N worker procs and a printing proc.
    worker_count = num_procs;
    for(int ii = 0; ii < num_procs; ii++) {
        //printf("forking...");
        workers[ii] = fork();
        if(workers[ii] == 0) {
            return 0;
        }
    }
    printer = fork();
    if(printer == 0) {
        print_results(count);
        //exit(0);
        return 1;
    }
    /*else {
        for (int64_t ii = 0; ii < count; ++ii) {
            job jj = make_job(start + ii);
            //printf("SUBMITTING JOB\n");
            submit_job(jj);
        }
        factor_wait_done();
    }*/
    return 2;
}

void
factor_cleanup()
{
    job done = make_job(-1);

    for (int ii = 0; ii < worker_count; ++ii) {
        //printf("submitting complete job\n");
        submit_job(done);
    }

    // FIXME: Make sure all the workers are done.
    //factor_wait_done();

    //printf("HEYYY %i %i %i %i\n", iqueue->qii, iqueue->qjj, oqueue->qii, oqueue->qjj);
    free_queue(iqueue);
    iqueue = 0;
    free_queue(oqueue);
    oqueue = 0;
}

void
submit_job(job jj)
{
    queue_put(iqueue, jj);
}

job
get_result()
{
    return queue_get(oqueue);
}

static
int64_t
isqrt(int64_t xx)
{
    double yy = ceil(sqrt((double)xx));
    return (int64_t) yy;
}

void
factor(int64_t xx, int64_t* size, int64_t* ys)
{
    int jj = 0;

    while (xx % 2 == 0) {
        ys[jj++] = 2;
        xx /= 2;
    }

    for (int64_t ii = 3; ii <= isqrt(xx); ii += 2) {
        int64_t x1 = xx / ii;
        if (x1 * ii == xx) {
            ys[jj++] = ii;
            xx = x1;
            ii = 1;
        }
    }

    ys[jj++] = xx;
    *size = jj;
}
