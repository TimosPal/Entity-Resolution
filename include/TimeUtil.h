#ifndef TIME_H
#define TIME_H

#include <time.h>
#include <stdio.h>

void startClock(struct timespec* start);
void endClock(struct timespec* start, char* process, FILE* fp);

#endif
