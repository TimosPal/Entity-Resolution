#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#define BUFFER_SIZE 256
#define EXIT_ERROR_CODE 1
#define DEFAULT_HASH_SIZE 1024

#define EPOCHS 10
#define LEARNING_RATE 0.01
#define VOCAB_SIZE 1000
#define MAX_ACCURACY_DIFF 0.5
#define BATCH_SIZE 512
#define THRESHOLD 0.02
#define WORKERS 1
#define EARLY_STOP_EUCLIDEAN_DISTANCE 0.000001
#define RETRAINING_TESTING_PAIRS 10000
#define TRAINING_STEPS 1
#define STEP_VALUE 0.02
#define CLIQUE_SIZE_LIMIT 3000
#define EQUAL_PAIRS true;
#define JOBS_PER_UPDATE 10;

#define IF_ERROR_MSG(x,msg) { if(x) {fprintf(stderr,"ERROR : %s.\n",msg); exit(EXIT_ERROR_CODE); } }

#endif