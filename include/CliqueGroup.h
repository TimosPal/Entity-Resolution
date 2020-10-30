#ifndef CLIQUEGROUP_H
#define CLIQUEGROUP_H

#include "Hash.h"
#include "LinkedList.h"

typedef struct CliqueGroup{
    Hash hash;
    List cliques;
} CliqueGroup;


void CliqueGroup_Init(CliqueGroup* cg, int bucketSize,unsigned int (*hashFunction)(void*, unsigned int), bool (*cmpFunction)(void*, void*));
void CliqueGroup_Add(CliqueGroup* cg, void* value);

#endif
