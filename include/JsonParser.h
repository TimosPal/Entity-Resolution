#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <stdio.h>

#include "LinkedList.h"

typedef struct ValuePair {
    char* leftVal;
    char* rightVal;
}ValuePair;

char* GetStringBetweenQuotes(FILE* fp);
List GetJsonPairs(char* filePath);

#endif