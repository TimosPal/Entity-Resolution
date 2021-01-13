#include "JobScheduler.h"

#include <pthread.h>

#include "Util.h"

typedef struct ThreadArgs{
    pthread_mutex_t* queueResults_Lock;

    pthread_mutex_t* queueJobs_Lock;
    pthread_cond_t* queueJobs_Cond;

    Queue* jobs;
    Queue* results;

    bool* shouldEnd;
} ThreadArgs;

void* JobHandler(void* args){
    ThreadArgs* threadArgs = args;

    while(true) {
        // Wait until the queue has a Job or the thread must exit.
        pthread_mutex_lock(threadArgs->queueJobs_Lock);
        while (threadArgs->jobs->head == NULL && *(threadArgs->shouldEnd) == false) {
            printf("1\n");
            pthread_cond_wait(threadArgs->queueJobs_Cond, threadArgs->queueJobs_Lock);
        }

        Job* currJob = NULL;
        if(!threadArgs->shouldEnd)
            currJob = Queue_Pop(threadArgs->jobs);

        pthread_mutex_unlock(threadArgs->queueJobs_Lock);

        if(threadArgs->shouldEnd){
            break;
        }

        /* currJob can only be null if the thread woke up from the shouldEnd "signal"
         * but that would mean the while loop would break , so we are free to use the
         * value */

        // Ready to execute job.
        currJob->result = currJob->task(currJob->taskArgs);

        // Add to Results queue for processing from main thread.
        pthread_mutex_lock(threadArgs->queueResults_Lock);

        Queue_Push(threadArgs->results, currJob);

        pthread_mutex_unlock(threadArgs->queueResults_Lock);
    }

    // Thread exit.
    free(threadArgs);

    int* status = malloc(sizeof(int));
    *status = 0;
    pthread_exit(status);
}

void JobScheduler_Init(JobScheduler* jobScheduler, int numberOfThreads){
    Queue_Init(&jobScheduler->jobs);
    Queue_Init(&jobScheduler->results);
    
    jobScheduler->shouldEnd = false;
    jobScheduler->numberOfThreads = numberOfThreads;
    jobScheduler->threadsIDs = malloc(numberOfThreads * sizeof(pthread_t));

    pthread_mutex_init(&jobScheduler->queueResults_Lock, NULL);

    pthread_mutex_init(&jobScheduler->queueJobs_Lock, NULL);
    pthread_cond_init(&jobScheduler->queueJobs_Cond, NULL);

    // Start threads.
    for (int i = 0; i < numberOfThreads; ++i) {
        ThreadArgs* args = malloc(sizeof(ThreadArgs));

        args->shouldEnd = &jobScheduler->shouldEnd;
        args->jobs = &jobScheduler->jobs;
        args->results = &jobScheduler->results;
        args->queueJobs_Cond = &jobScheduler->queueJobs_Cond;
        args->queueJobs_Lock = &jobScheduler->queueJobs_Lock;
        args->queueResults_Lock = &jobScheduler->queueResults_Lock;

        IF_ERROR_MSG(pthread_create(jobScheduler->threadsIDs + i, NULL, JobHandler, args) != 0, "Thread creation error")
    }
}

void Job_Init(Job* job, void* (*task)(void** args), void (*freeMethod)(void* val), void** taskArgs){
    job->result = NULL;
    job->freeMethod = freeMethod;
    job->task = task;
    job->taskArgs = taskArgs;
}

void Job_Free(void* value){
    Job* job = value;
    job->freeMethod(job->result);
    free(job);
}

void JobScheduler_Destroy(JobScheduler* jobScheduler){
    // Destroy threads.
    jobScheduler->shouldEnd = true;
    pthread_cond_broadcast(&jobScheduler->queueJobs_Cond);

    for (int i = 0; i < jobScheduler->numberOfThreads; ++i) {
        int *status;
        pthread_join(jobScheduler->threadsIDs[i], (void**) &status);
        free(status);
    }

    free(jobScheduler->threadsIDs);

    Queue_FreeValues(&jobScheduler->jobs, Job_Free);
    Queue_Destroy(&jobScheduler->jobs);

    Queue_FreeValues(&jobScheduler->results, Job_Free);
    Queue_Destroy(&jobScheduler->results);

    // Free cond / mutexes.
    pthread_mutex_destroy(&jobScheduler->queueResults_Lock);
    pthread_mutex_destroy(&jobScheduler->queueJobs_Lock);

    pthread_cond_destroy(&jobScheduler->queueJobs_Cond);
}

void JobScheduler_AddJob(JobScheduler* jobScheduler, Job* newJob){
    Queue_Push(&jobScheduler->jobs, newJob);
    pthread_cond_signal(&jobScheduler->queueJobs_Cond);
}