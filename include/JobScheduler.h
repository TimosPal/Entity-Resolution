#ifndef JOBSCHEDULER_H
#define JOBSCHEDULER_H

#include <pthread.h>

#include "Queue.h"

typedef struct JobScheduler{
    int numberOfThreads;
    pthread_t* threadsIDs;

    pthread_mutex_t queueResults_Lock;

    pthread_mutex_t queueJobs_Lock;
    pthread_cond_t queueJobs_Cond;

    Queue jobs;
    Queue results;

    bool shouldEnd;
} JobScheduler;

typedef struct Job{
    // array of args , should be handled appropriately inside the task function.
    void** taskArgs;
    void* (*task)(void** args);
    void (*freeMethod)(void* val);

    void* result;
} Job;

void Job_Init(Job* job, void* (*task)(void** args), void (*freeMethod)(void* val), void** taskArgs);

void JobScheduler_Init(JobScheduler* jobScheduler, int numberOfThreads);
void JobScheduler_Destroy(JobScheduler* jobScheduler);
void JobScheduler_AddJob(JobScheduler* jobScheduler, Job* newJob);

#endif