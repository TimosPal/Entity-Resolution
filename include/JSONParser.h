#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <stdio.h>

#include "LinkedList.h"

typedef struct ValuePair {
    char* leftVal;
    char* rightVal;
}ValuePair;

void ValuePair_Free(void* value);
char* GetStringBetweenQuotes(FILE* fp);
List GetJsonPairs(char* filePath);

#endif