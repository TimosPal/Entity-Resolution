#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <stdio.h>

#include "LinkedList.h"
#include "Hash.h"

typedef struct ValuePair {
    char* leftVal;
    List rightVals; // Can have multiple values if it's a json array.
}ValuePair;

void ValuePair_Free(void* value);
char* GetStringBetweenQuotes(FILE* fp);
List GetJsonPairs(char* filePath);

#endif