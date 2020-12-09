#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUFFER_SIZE 256
#define EXIT_ERROR_CODE 1
#define STOPWORDS_FILE "../../stopwords.txt"
#define DEFAULT_HASH_SIZE 1024

#define IF_ERROR_MSG(x,msg) { if(x) {fprintf(stderr,"ERROR : %s.\n",msg); exit(EXIT_ERROR_CODE); } }

typedef struct Tuple{
    void* value1;
    void* value2;
}Tuple;

#endif