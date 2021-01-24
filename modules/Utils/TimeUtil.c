#include "TimeUtil.h"

void startClock(struct timespec* start){
    clock_gettime(CLOCK_MONOTONIC, start);
}

void endClock(struct timespec* start, char* process, FILE* fp){
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double cpu_time_used = (end.tv_sec - start->tv_sec);
    cpu_time_used += (end.tv_nsec - start->tv_sec) / 1000000000.0;
    fprintf(fp, "%s, %f\n", process, cpu_time_used);
}