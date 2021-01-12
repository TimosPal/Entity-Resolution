#ifndef JOBSCHEDULER_H
#define JOBSCHEDULER_H

#include <pthread.h>

#include "Queue.h"

typedef struct JobScheduler{
    int numberOfThreads;
    pthread_t* threadsIDs;

    pthread_mutex_t queue_Lock;
    pthread_cond_t queue_Cond;

    Queue jobs;
} JobScheduler;

typedef struct Job{
    // array of args , should be handled appropriately inside the task function.
    void** taskArgs;
    int numberOfArgs;

    void* (*task)(void** args);
} Job;

void JobScheduler_Init(JobScheduler* jobScheduler, int numberOfThreads);
void JobScheduler_Destroy(JobScheduler* jobScheduler);

#endif