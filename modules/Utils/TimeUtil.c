#include "TimeUtil.h"

clock_t startClock(){
    return clock();
}

void endClock(clock_t start, char* process, FILE* fp){
    clock_t end = clock();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    fprintf(fp, "%s, %f\n", process, cpu_time_used);
}