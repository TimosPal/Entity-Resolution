#ifndef TIME_H
#define TIME_H

#include <time.h>
#include <stdio.h>

clock_t startClock();
void endClock(clock_t start, char* process, FILE* fp);

#endif
