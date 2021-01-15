#include "acutest.h"

#include "JobScheduler.h"

//TODO : who frees the mem?

void* sum(void** args){
    int n = *(int*)args[0];

    int* sum = malloc(sizeof(int));
    *sum = 0;
    for (int i = 0; i < n; ++i) {
        *sum += i;
    }

    return sum;
}

void JobScheduler_Test_General(){
    JobScheduler jobScheduler;
    JobScheduler_Init(&jobScheduler, 10);

    for (int i = 0; i < 100; ++i) {
        void** args = malloc(sizeof(void*));
        int* n = malloc(sizeof(int));
        *n = i;
        args[0] = n;

        Job* newJob = malloc(sizeof(Job));
        Job_Init(newJob, sum, free, args);

        JobScheduler_AddJob(&jobScheduler, newJob);
    }
    JobScheduler_WaitForJobs(&jobScheduler, 100);

    int sum = 0;
    Node* temp = jobScheduler.results.head;
    while(temp != NULL){
        Job* currJob = temp->value;
        int val = *(int*)currJob->result;
        sum += val;

        free(((void **)currJob->taskArgs)[0]);
        free(currJob->taskArgs);

        temp = temp->next;
    }

    JobScheduler_Destroy(&jobScheduler);

    int outerSum = 0;
    for (int j = 0; j < 100; ++j) {
        int innerSum = 0;
        for (int i = 0; i < j; ++i) {
            innerSum += i;
        }
        outerSum += innerSum;
    }

    TEST_ASSERT(sum == outerSum);
}

TEST_LIST = {
        { "JobScheduler_Test_General", JobScheduler_Test_General },
        { NULL, NULL }
};