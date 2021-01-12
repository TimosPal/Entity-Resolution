#include "JobScheduler.h"

#include <pthread.h>

#include "Util.h"

void* JobHandler(void* args){

    // Thread exit.
    int* status = malloc(sizeof(int));
    *status = 0;
    pthread_exit(status);
}

void JobScheduler_Init(JobScheduler* jobScheduler, int numberOfThreads){
    jobScheduler->numberOfThreads = numberOfThreads;
    jobScheduler->threadsIDs = malloc(numberOfThreads * sizeof(pthread_t));

    jobScheduler->start_Lock = PTHREAD_MUTEX_INITIALIZER;
    jobScheduler->start_Cond = PTHREAD_COND_INITIALIZER;

    // Start threads.
    for (int i = 0; i < numberOfThreads; ++i) {
        IF_ERROR_MSG(pthread_create(jobScheduler->threadsIDs + i, NULL, JobHandler, NULL) != 0, "Thread creation error")
    }
}

void JobScheduler_Destroy(JobScheduler* jobScheduler){
    // Destroy threads.
    for (int i = 0; i < jobScheduler->numberOfThreads; ++i) {
        int *status;
        pthread_join(jobScheduler->threadsIDs[i], (void**) &status);
        free(status);
    }

    free(jobScheduler->threadsIDs);
}
