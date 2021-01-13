#include "acutest.h"

#include "JobScheduler.h"

void* FindRoot(void** args){
    //int* numbers = *(int**)(args);

    //printf("%d\n", numbers[0]);

    printf("FWFFFE\n");

    return NULL;
}

void JobScheduler_Test_General(){
    JobScheduler jobScheduler;
    JobScheduler_Init(&jobScheduler, 1);

    int* numbers = malloc(10*sizeof(int));
    for(int i = 0; i < 10; i++){
        numbers[i] = i;
    }
    
    Job* firstJob = malloc(sizeof(Job));
    void** firstArgs = malloc(2 * sizeof(void*));
    int* index1 = malloc(sizeof(int));
    *index1 = 4;
    firstArgs[0] = &numbers;
    firstArgs[1] = &index1;
    Job_Init(firstJob, FindRoot, free, firstArgs);
    JobScheduler_AddJob(&jobScheduler, firstJob);
    
    JobScheduler_Destroy(&jobScheduler);

    free(numbers);
}

TEST_LIST = {
        { "JobScheduler_Test_General", JobScheduler_Test_General },
        { NULL, NULL }
};