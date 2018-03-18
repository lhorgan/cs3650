
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>

#include "job.h"
#include "factor.h"

int
main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage:\n");
        printf("  ./main procs start count\n");
        return 1;
    }

    int procs = atoi(argv[1]);
    //assert(procs == 1); // FIXME: Support multiple procs

    int64_t start = atol(argv[2]);
    int64_t count = atol(argv[3]);

    int res = factor_init(procs, count);
    /*if(res > 0) { // parent process
        pid_t printer = fork();
        if(printer == 0) {
            print_results(count);
        }
        else {
            printf("you piece of...");
        }
        return 0;
    }*/
    if(res == 2) {
        for (int64_t ii = 0; ii < count; ++ii) {
            job jj = make_job(start + ii);
            //printf("SUBMITTING JOB\n");
            submit_job(jj);
        }
        job jj = make_job(-1);
        submit_job(jj);
        factor_wait_done();
    }
    if(res == 0) {
        //printf("I'm a child process\n");
        //return 0;
        // I'm a child process

        // FIXME: These next two lines shouldn't be here.
        //job jj = make_job(-1);
        //submit_job(jj);

        // FIXME: This should happen in worker processes.
        //printf("wtaf");
        work_off_jobs();

        // FIXME: This should happen in dedicated printer process.
        //print_results(count);

        //factor_wait_done();
        //printf("well, this is something we are ready to cleanup\n");
        factor_cleanup();
        exit(0);

        return 0;
    }
}
