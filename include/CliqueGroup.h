#ifndef CLIQUEGROUP_H
#define CLIQUEGROUP_H

#include "Hash.h"
#include "LinkedList.h"
#include <stdbool.h>

typedef struct CliqueGroup{
    Hash hash;
    List cliques;
} CliqueGroup;


void CliqueGroup_Init(CliqueGroup* cg, int bucketSize,unsigned int (*hashFunction)(const void*, unsigned int), bool (*cmpFunction)(void*, void*));
void CliqueGroup_Destroy(CliqueGroup cg);
void CliqueGroup_FreeValues(CliqueGroup cg, void (*subFree)(void*));
bool CliqueGroup_Add(CliqueGroup* cg, void* key, int keysize, void* value);


#endif
